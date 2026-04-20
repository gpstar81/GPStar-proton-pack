# Web-Based API Reference

All web-enabled devices expose behavior which can be managed via RESTful API endpoints. These are available via the Attenuator (either as a kit or WiFi adapter) and all GPStar II devices. This provides an API-first design which can either use the built-in web pages with the device, or you may create your own UI.

## OpenAPI

As of the v6.1.1 release, all devices will now directly self-report their available API's in a standard format called [OpenAPI](https://swagger.io/specification/) which is widely understood by many services across the internet, meaning the data can be integrated easily with a large variety of services and tools, if desired. The specification for the APIs is produced using the OpenAPI 3.x specification and can be obtained directly via `GET /openapi.json`

A new HTML page has also been added to every device which can display the API's, along with a description of their purpose, and a means to directly run the API from your web browser. To access this special page you will need to navigate to this location for your device: `GET /swaggerui`

**Note:** This page uses a CDN to download the necessary SwaggerUI stylesheets and JavaScript libraries from the internet, meaning it may be necessary to switch between the private WiFi of the device and a network interface that allows access to the internet at least to download the contents. Ideally this is best done using a computer which may have both a wired and WiFi interface, so that you can connect wirelessly to your device while handling internet downloads via the wired interface.

## Push Events

By convention, all of the API endpoints work on a "pull" basis--you only get data when you request it, not as it happens. So in order to get data in a timely manner we have 2 options: either create a "polling" mechanism to grab data on a regular cadence, or push the data direct to clients. In the case of the former this can be extremely wasteful in terms of resource utilization, especially for such small devices as the microcontrollers in use. However, since we use a special web server which is specifically designed for asynchronous communication we can utilize 2 standard web-based features: Web Sockets and Server-Sent Events.

### Web Sockets

Our built-in web server allows us to offer a special URI endpoint `/ws` to support [WebSockets](https://developer.mozilla.org/en-US/docs/Web/API/WebSockets_API). This is much more efficient as it 1) only sends data as the system changes states, and 2) is meant to report only the information necessary for interactions. Basically, instead of calling `/status` repeatedly and hoping you pick up a recent change (for example, due to user input like firing the wand) we can notify all connected clients in real-time when updates occur.

All devices support the WebSocket output, which primarily reports the current device status and is updated after specific system actions occur. Note that while this data will always be in the form of a JSON object, some events may send a plain string, so always check the contents of the text data carefully before usage to ensure you are working with the expected data format. The main benefit of using WebSockets is that this data stream is bi-directional! So while a device can immediately notify clients when something happens, clients could potentially communicate over the same socket they're already connected to. Currently this is limited to only keeping a heartbeat from clients to know they're still attached, but could be expanded in the future.

### Server-Sent Events

The other type of push communications is [Server-Sent Events](https://developer.mozilla.org/en-US/docs/Web/API/Server-sent_events) which is similar to WebSockets but is strictly one-way traffic from the server to the client. This is meant for high-speed updates and doesn't care if the data was received or not (because the client can't really respond to say otherwise).

For the GPStar II Neutrona Wand and GPStar II Single-shot Blaster the web server makes use of server-sent events for a responsive, read-only transmission of data. This is primarily used for sending telemetry data for the 3D spatial display or providing information related to calibration operations.