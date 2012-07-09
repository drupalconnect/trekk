<?php
class MigrateRemoteFileFileFieldHandler extends MigrateFileFieldHandler {
  public function __construct() {
    parent::__construct();
  }

  
  /**
   * Parses file information to create an appropriate data array.
   *
   * @param $entity
   * @param array $field_info
   * @param array $instance
   * @param $migration
   * @param $arguments
   *   Supported arguments are
   *   - 'file_function': One of
   *     - 'file_fid' if the incoming $value is a fid that we can use as is.
   *     - 'file_copy' to use the file_copy() function
   *     - 'file_move' to use file_move()
   *     - 'file_fast' to use the existing file
   *     - 'file_link'
   *     - 'file_blob'
   *   - 'file_replace' See file_copy(). FILE_EXISTS_REPLACE, FILE_EXISTS_RENAME,
   *      or FILE_EXISTS_ERROR.
   *   - 'source_path': A path to be prepended to the path of the source item.
   * @param $value
   *   If file_function is set to file_fid, this will be a fid. Otherwise, it may
   *   be a path to the source file, or a json-encoded array of properties:
   *   - path
   *   - alt
   *   - title
   *   - description
   *   - display
   * @return array
   */
  protected function buildFileArray($entity, array $field_info, array $instance, $migration, $arguments, $value) {
    static $fids;

    // If we've been passed a fid, all the hard work's been done
    if ($arguments['file_function'] == 'file_fid') {
      $file = file_load($value);
      if (empty($file)) {
        $migration->saveMessage(t('The fid provided to create a file field (%fid) does not exist in the managed_file table.', array('%fid' => $value)), Migration::MESSAGE_ERROR);
        return;
      }
    }
    else {
      // Is the value a straight path, or JSON containing a set of properties?
      if ($value{0} == '{') {
        $properties = drupal_json_decode($value);
        $path = $properties['path'];
        // Properties passed in with the image override any set via arguments
        if (!empty($properties['alt'])) {
          $arguments['alt'] = $properties['alt'];
        }
        if (!empty($properties['title'])) {
          $arguments['title'] = $properties['title'];
        }
        if (!empty($properties['description'])) {
          $arguments['description'] = $properties['description'];
        }
        if (!empty($properties['display'])) {
          $arguments['display'] = $properties['display'];
        }
      }
      else {
        $path = $value;
      }

      // One can override a file_function via CLI or drushrc.php
      if ($migration->getOption('file_function')) {
        $file_function = $migration->getOption('file_function');
      }
      else {
        $file_function = $arguments['file_function'];
      }

      $destination_dir = $this->destinationDir($field_info, $instance);
      file_prepare_directory($destination_dir, FILE_CREATE_DIRECTORY);
      if (!empty($arguments['source_path'])) {
        // $full_path needs adjusting for file_blob
        if ($file_function == 'file_blob') {
          $full_path = file_stream_wrapper_uri_normalize($destination_dir . "/" . str_replace('/', '-', $arguments['source_path']));
        }
        else {
          $full_path = rtrim($arguments['source_path'], "/\\") .
            '/' . ltrim($path, "/\\");
        }
      }
      else {
        $full_path = $path;
      }
      $remote = FALSE;

      // TODO: belongs in prepare or validate?
      // Check that source exists. If not, mark the entity as 'needs_update' and bail.
      // Sometimes the source file arrives later, when rsync is slower than DB.
      if (!is_file($full_path)) {
        // is_file() won't handle URLs, check to see if we have a remote file
        // (only relevant in the file_copy case)
        if ($file_function == 'file_copy') {
          $d_req = drupal_http_request($full_path);
          //watchdog('filefield request', dprint_r($d_req, TRUE));
          if (isset($d_req->code) && $d_req->code == 200) {
            if (!empty($d_req->redirect_url)) {
              $full_path = $d_req->redirect_url;
              $redirected = TRUE;
            }
            else {
              $redirected = FALSE;
            }
            watchdog('filefield handler', ($redirected ? 'Redir to ' : 'Found ') . $full_path);
            
            $remote = TRUE;
          } else {
            watchdog('filefield handler', $full_path . ' is not 200');
          }
          
        }
        if (!$remote) {
          $migration->saveMessage(t('Source file does not exist: !path',
            array('!path' => $full_path)), Migration::MESSAGE_WARNING);
          //$migration->needsUpdate = MigrateMap::STATUS_NEEDS_UPDATE;
          return;
        }
      }

      if ($remote) {
        $destination_file = $destination_dir . "/" . basename($full_path);
      }
      else {
        $destination_file = file_stream_wrapper_uri_normalize($destination_dir . "/" . str_replace('/', '-', basename($full_path)));
      }

      $file_replace = empty($arguments['file_replace']) ? FILE_EXISTS_RENAME : $arguments['file_replace'];
      $destination_file = file_destination($destination_file, $file_replace);
      $real_destination_file = drupal_realpath($destination_file);
      $source = (object) array(
        'uri' => $full_path,
        'uid' => isset($entity->uid) ? $entity->uid : 0,
        'filename' => basename($full_path),
        'filemime' => file_get_mimetype($full_path),
        'timestamp' => REQUEST_TIME,
      );

      // Check that destination does not exist. If it does, reuse it and return.
      if (file_exists($real_destination_file)) {
        // Save this file to DB.
        if ($existing_files = file_load_multiple(array(), array('uri' => $destination_file))) {
          // Existing record exists. Reuse it. TODO: Perhaps we never should re-use records.
          $file = reset($existing_files);
          $file = file_save($file);
        }
        else {
          // Get this orphaned file into the file table.
          $file = clone $source;
          $file->fid = NULL;
          $file->uri = $destination_file;
          $file->status |= FILE_STATUS_PERMANENT; // Save a write in file_field_presave().
          $file = file_save($file);
        }
      }
      else {
        migrate_instrument_start('MigrateFileFieldHandler file_function');
        switch ($file_function) {
          case 'file_copy':
            if ($remote) {
              $result = @copy($source->uri, $destination_file);
              if ($result) {
                $source->uri = $destination_file;
                $file = file_save($source);
              }
              else {
                $migration->saveMessage(t('Unable to copy file from !source',
                  array('!source' => $source->uri)));
              }
            }
            else {
              $file = file_copy($source, $destination_dir, $file_replace);
            }
            break;
          case 'file_move':
            // file_move() does a copy then delete which slow. So we implement our own.
            if (file_prepare_directory($destination_dir, FILE_CREATE_DIRECTORY)) {
              if (rename($source->uri, $real_destination_file)) {
                $file = clone $source;
                $file->fid = NULL;
                $file->uri = $destination_file;
                $file->status |= FILE_STATUS_PERMANENT; // Save a write in file_field_presave().
                $file = file_save($file);

                // Inform modules that the file has been copied.
                module_invoke_all('file_copy', $file, $source);
                break;
              }
              else {
                $migration->saveMessage(t('Unable to rename !source to !uri', array('!source' => $source->uri, '!uri' => $destination_file)), Migration::MESSAGE_ERROR);
                return;
              }
            }
            else {
              $migration->saveMessage(t('Unable to prepare directory !dir', array('!dir' => $destination_dir)), Migration::MESSAGE_ERROR);
              return;
            }
            break;
          case 'file_fast':
            // Keep re-using an existing file. We still benefit from the file_exists() check above.
            if (!isset($fids[$source])) {
              $full_path = DRUPAL_ROOT . '/misc/druplicon.png';
              $source = (object) array(
                'uri' => $full_path,
                'uid' => isset($entity->uid) ? isset($entity->uid) : 0,
                'filename' => basename($full_path),
                'filemime' => file_get_mimetype($full_path),
                'timestamp' => REQUEST_TIME,
              );
              $file = file_copy($source, $destination_dir, FILE_EXISTS_RENAME);
              $fid = $file->fid;
            }
            else {
              $file = new stdClass();
              $file->fid = $fids[$source];
            }
            break;
          case 'file_link':
          case 'file_blob':
            // The file is copied by some outside process (e.g., rsync), and we
            // just need to make sure it's present and has a files table row.
            // Not present - skip
            migrate_instrument_start('file_link: file_exists');
            if (!file_exists($full_path)) {
              migrate_instrument_stop('file_link: file_exists');
              $message = t('File does not exist in Drupal files directory: !path',
                array('!path' => $full_path));
              if (!empty($arguments['throw_error'])) {
                throw new MigrateException($message, MigrationBase::MESSAGE_ERROR);
              }
              else {
                $migration->saveMessage($message, MigrationBase::MESSAGE_INFORMATIONAL);
              }
              $message_saved = TRUE;
              // TODO     $migration->needsUpdate = MigrateMap::STATUS_NEEDS_UPDATE;
              continue;
            }
            migrate_instrument_stop('file_link: file_exists');

            // Files table entry exists? Use that...
            migrate_instrument_start('file_link: select existing');
            // Note that indexing files.filepath can be very helpful.
            // TODO: Might be better way that straight query in D7?
            $file = db_select('file_managed', 'f')
                    ->fields('f')
                    ->condition('uri', $full_path)
                    ->execute()
                    ->fetchObject();
            migrate_instrument_stop('file_link: select existing');
            if (!$file) {
              migrate_instrument_start('file_link: create file record');
              $file = clone $source;
              $file->uri = $full_path;
              $file->uid = isset($entity->uid) ? $entity->uid : 0;
              $file->status |= FILE_STATUS_PERMANENT;
              $file = file_save($file);
              migrate_instrument_stop('file_link: create file record');
            }
            break;
        }
        migrate_instrument_stop('MigrateFileFieldHandler file_function');
      }
    }

    if ($file) {
      // Build up a return object.
      $object_field['fid'] = $file->fid;
      if (!empty($arguments['alt'])) {
        $object_field['alt'] = $arguments['alt'];
      }
      else {
        $object_field['alt'] = '';
      }
      if (!empty($arguments['title'])) {
        $object_field['title'] = $arguments['title'];
      }
      else {
        $object_field['title'] = '';
      }
      if (!empty($arguments['description'])) {
        $object_field['description'] = $arguments['description'];
      }
      else {
        $object_field['description'] = '';
      }
      if (!empty($arguments['display'])) {
        $object_field['display'] = $arguments['display'];
      }
      else {
        $object_field['display'] = TRUE;
      }
      return $object_field;
    }
    else {
      $migration->saveMessage(t('Unable to create file record for value=%value, arguments=%arguments', array('%value' => print_r($value, TRUE), '%arguments' => print_r($arguments, TRUE))), Migration::MESSAGE_ERROR);
    }
  }

}
