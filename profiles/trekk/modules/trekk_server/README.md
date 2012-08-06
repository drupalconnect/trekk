
Trekk Server permits exposing content via Services 3 and Views. To use it, make sure you have correctly installed both Views 3 and Services 3, along with one of Services 3's server modules (we recommend REST server).

To expose content for consumption by Trekk Client,

1. Create a view that selects the content you want to expose. Your view may utilize arguments, sorts, and filters, as normal. You may create any number of displays. All of them will be accessible via your Services 3 resource, where the name of the resource corresponds to the name of your view.
2. Create a Services 3 endpoint and enable the resource that corresponds to your view. Only the retrieve state is available).
3. Test your endpoint by doing a GET request, as described below.

Example:

Assuming a view with the name "content_export", your Services 3 resource also would be called "content_export". After enabling this resource's "retrieve" state you may access your view at the URI <endpoint>/<view_name>/<preview|data>. If you have enabled the trekk_server_example feature, then you can see this in action by visiting

GET /api/v1/content_export/preview
GET /api/v1/content_export/data

The "preview" and "data" components of the resource URI indicate how you wish the view to be output. "Preview" will output rendered HTML, while "data" will directly output the views result. On the client side, you may use one or the other URI depending on your needs. HTML output will be useful when you do not wish the client to create nodes from your exported data. "Data" will be usefull if you do wish to create nodes, or if you want to do other processing before rendering the content on a page.
