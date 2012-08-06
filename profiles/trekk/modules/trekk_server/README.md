
Trekk Server permits exposing content via Services 3 and Views. To use it, make sure you have correctly installed both Views 3 and Services 3, along with one of Services 3's server modules (we recommend REST server).

To expose content for consumption by Trekk Client,

1. Create a view that selects the content you want to expose. Your view may utilize arguments, sorts, and filters, as normal. You may create any number of displays. All of them will be accessible via your Services 3 resource, where the name of the resource corresponds to the name of your view.
2. Create a Services 3 endpoint and enable the resource that corresponds to your view. Only the retrieve state is available).
3. Test your endpoint by doing a GET request, as described below.

Example:

Assuming a view with the name "content_export", your Services 3 resource also would be called "content_export". After enabling this resource's "retrieve" state you may access your view at the URI <endpoint>/<view_name>/<display id>. If you have enabled the trekk_server_example feature, then you can see this in action by visiting

GET /api/v1/content_export/default

Pay special attention to the ?output URI query parameter, which indicates how you wish the view to be output. "html" will output rendered HTML, while "data" will directly output the views result. On the client side, you may use one or the other URI parameters, depending on your needs. HTML output will be useful when you do not wish the client to create nodes from your exported data. "Data" will be usefull if you do wish to create nodes, or if you want to do other processing before rendering the content on a page.

The server also accepts a number of URI query parameters:

?display_id (Sets the ID of the views display, e.g., "default")
?args (Allows passing views arguments. Be sure to pass them in the same order as defined in the view)
?offset (Page number, for paged views)
?limit (Number of items to display)
?filters (Allows passing views filters)
?output (Allows control of rendering. "data" causes the server to output the view results object directly, while "html" causes a preview to be rendered and sent back.)
