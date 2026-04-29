/**
 *   Communications - Serial communication packet definitions for GPStar devices.
 *   Provides common objects and enums for serial data exchange.
 *   Copyright (C) 2023-2026 Michael Rajotte, Dustin Grau, Nomake Wan
 *
 *   This program is free software; you can redistribute it and/or modify
 *   it under the terms of the GNU General Public License as published by
 *   the Free Software Foundation; either version 3 of the License, or
 *   (at your option) any later version.
 *
 *   This program is distributed in the hope that it will be useful,
 *   but WITHOUT ANY WARRANTY; without even the implied warranty of
 *   MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *   GNU General Public License for more details.
 *
 *   You should have received a copy of the GNU General Public License
 *   along with this program; if not, see <https://www.gnu.org/licenses/>.
 *
 */

#pragma once
#include <stdint.h>
#include <set>

// Web Server Includes
#include <AsyncJson.h>
#include <ESPAsyncWebServer.h>

// Store MDNS name and IP addresses for OpenAPI spec generation
String deviceMdnsName = "";
String deviceIpAddress = "";
String externalIpAddress = "";

// Function pointer to get current external IP dynamically
String (*getExternalIpCallback)() = nullptr;

/**
 * Set the device MDNS name for OpenAPI specification
 * Call this after device initialization and before setupRouting()
 */
void setDeviceMdnsName(const String& mdnsName) {
  deviceMdnsName = mdnsName;
}

/**
 * Set the device IP address for OpenAPI specification (local AP IP)
 * Call this after network connection and before setupRouting()
 */
void setDeviceIpAddress(const String& ipAddress) {
  deviceIpAddress = ipAddress;
}

/**
 * Set callback function to dynamically retrieve external IP address
 * This allows the OpenAPI spec to reflect current WiFi state
 */
void setExternalIpCallback(String (*callback)()) {
  getExternalIpCallback = callback;
}

/**
 * Helper to compute the size (in bytes) of an embedded binary asset using
 * the linker-provided start/end markers generated for each asset (must be
 * defined in the calling program for each file which is an embedded asset).
 * Inputs:
 *   - start: pointer to the first byte (e.g. _binary_assets_<file>_start)
 *   - end:   pointer to the one-past-last byte (e.g. _binary_assets_<file>_end)
 * Outputs:
 *   - size_t: number of bytes in the embedded asset (0 on invalid pointers or if end <= start)
 */
inline size_t embeddedFileSize(const uint8_t* start, const uint8_t* end) {
  if (start == nullptr || end == nullptr) return 0;
  if (end <= start) return 0;
  return (size_t)(end - start);
}

/**
 * Helper to return a small JSON object with a "status" property: {"status":"<value>"}
 * This returns the provided status string verbatim (no escaping or modification).
 */
String returnJsonStatus(const String &status = String("success")) {
  String s_out;
  s_out.reserve(status.length() + 16); // Reserve space to avoid multiple allocations.
  s_out = "{\"status\":\"";
  s_out += status; // Append status value.
  s_out += "\"}";
  return s_out;
}

// Helper function to safely extract boolean values from JSON and update bool field directly
void updateJsonBool(bool& targetField, const JsonDocument& jsonBody, const String& propertyName) {
  // Updates the target field if property exists and is of the expected type, otherwise the target is unchanged.
  if(jsonBody[propertyName].is<bool>()) {
    targetField = jsonBody[propertyName].as<bool>(); // Directly assign bool value (true or false)
  }
}

/*
 * HTTP and MIME Constants - Values for consistent web responses
 */

// HTTP Status Codes
const uint16_t HTTP_STATUS_200 = 200; // OK
const uint16_t HTTP_STATUS_201 = 201; // Created
const uint16_t HTTP_STATUS_204 = 204; // No Content
const uint16_t HTTP_STATUS_400 = 400; // Bad Request
const uint16_t HTTP_STATUS_404 = 404; // Not Found
const uint16_t HTTP_STATUS_409 = 409; // Conflict
const uint16_t HTTP_STATUS_417 = 417; // Expectation Failed
const uint16_t HTTP_STATUS_500 = 500; // Internal Server Error
const uint16_t HTTP_STATUS_503 = 503; // Service Unavailable

// HTTP Headers
const char* HEADER_CACHE_CONTROL = "Cache-Control";
const char* HEADER_CONTENT_ENCODING = "Content-Encoding";

// HTTP Header Values
const char* CACHE_NO_CACHE = "no-cache, must-revalidate";
const char* ENCODING_GZIP = "gzip";

// MIME Types
const char* MIME_PLAIN = "text/plain";
const char* MIME_HTML = "text/html";
const char* MIME_CSS = "text/css";
const char* MIME_JAVASCRIPT = "application/javascript; charset=UTF-8";
const char* MIME_JSON = "application/json";
const char* MIME_STL = "model/stl";
const char* MIME_SVG = "image/svg+xml";
const char* MIME_ICON = "image/x-icon";

/*
 * OpenAPI Tag Definitions - Organizes endpoints into logical categories
 */
const char* TAG_PAGES = "Pages";
const char* TAG_ASSETS = "Assets";
const char* TAG_CONFIGURATION = "Configuration";
const char* TAG_EEPROM = "EEPROM";
const char* TAG_SYSTEM = "System";
const char* TAG_DEVICE_CONTROL = "Device Control";
const char* TAG_SENSOR_CONTROL = "Sensor Control";
const char* TAG_STREAM_MODES = "Stream Modes";
const char* TAG_THEMES = "Themes";
const char* TAG_VOLUME_CONTROL = "Volume Control";
const char* TAG_MUSIC_CONTROL = "Music Control";
const char* TAG_WIFI = "WiFi";
const char* TAG_DOCUMENTATION = "Documentation";
const char* TAG_PORTAL = "Captive Portal";

/*
 * Common Response Descriptions - Reusable strings for API responses
 */
const char* RESP_SUCCESS_STATUS = "Success status";
const char* RESP_HTML_PAGE = "HTML page content";
const char* RESP_JAVASCRIPT_FILE = "JavaScript file";
const char* RESP_CSS_FILE = "CSS file";
const char* RESP_STL_FILE = "STL file";
const char* RESP_SVG_FILE = "SVG file";
const char* RESP_ICON_FILE = "Icon file";
const char* RESP_SETTINGS_SAVED = "Settings saved successfully";
const char* RESP_SETTINGS_UPDATED = "Settings updated successfully";
const char* RESP_JSON_OBJECT = "JSON object";
const char* RESP_SYSTEM_STATUS = "System status object";
const char* RESP_CONFIG_OBJECT = "Configuration object";
const char* RESP_WIFI_SETTINGS = "WiFi settings object";
const char* RESP_NETWORK_ARRAY = "Array of network SSIDs";
const char* RESP_OPENAPI_SPEC = "OpenAPI 3.x JSON specification";
const char* RESP_NO_CONTENT_RESTART = "No content (device restarting)";
const char* RESP_CONNECTIVITY_CHECK = "Device connectivity check";

/*
 * JSON Property Names - Common property keys used in OpenAPI specification
 */
const char* JSON_PROPERTY_DESCRIPTION = "description";
const char* JSON_PROPERTY_NAME = "name";
const char* JSON_PROPERTY_URL = "url";
const char* JSON_PROPERTY_SUMMARY = "summary";
const char* JSON_PROPERTY_TAGS = "tags";
const char* JSON_PROPERTY_TYPE = "type";
const char* JSON_PROPERTY_CONTENT = "content";
const char* JSON_PROPERTY_SCHEMA = "schema";
const char* JSON_PROPERTY_RESPONSES = "responses";
const char* JSON_PROPERTY_PROPERTIES = "properties";
const char* JSON_PROPERTY_STATUS = "status";
const char* JSON_PROPERTY_REQUIRED = "required";

/*
 * JSON Data Types - Common type values used in OpenAPI schema definitions
 */
const char* JSON_TYPE_OBJECT = "object";
const char* JSON_TYPE_STRING = "string";

/*
 * Structures for Route Definitions
 */

 // Structure to define a route which does not require a body
struct RouteDefinition {
  const char* path;
  WebRequestMethodComposite method; // HTTP Verb (eg. HTTP_GET, HTTP_POST)
  void (*handler)(AsyncWebServerRequest *request);
  const char* summary;      // Short description for OpenAPI
  const char* description;  // Detailed description for OpenAPI
  const char* tag;          // Category/tag for OpenAPI grouping
  const char* responseDesc; // Description of successful response
};

// Structure to define a body handler route (POST with JSON body)
struct BodyHandlerDefinition {
  const char* path;
  AsyncCallbackJsonWebHandler* handler;
  const char* summary;         // Short description for OpenAPI
  const char* description;     // Detailed description for OpenAPI
  const char* tag;             // Category/tag for OpenAPI grouping
  const char* requestBodyDesc; // Description of expected request body
  const char* responseDesc;    // Description of successful response
};

/*
 * Route Registration System
 */

// Maximum number of routes that can be registered
const size_t MAX_API_ROUTES = 100;
const size_t MAX_BODY_ROUTES = 50;

// Pre-allocated route arrays with sentinel values
RouteDefinition apiRoutes[MAX_API_ROUTES] = {
  {nullptr, HTTP_ANY, nullptr, nullptr, nullptr, nullptr, nullptr} // Sentinel entry
};

BodyHandlerDefinition bodyHandlerRoutes[MAX_BODY_ROUTES] = {
  {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr} // Sentinel entry
};

// Track current insertion points
size_t apiRouteCount = 0;
size_t bodyRouteCount = 0;

/**
 * Add a simple route (GET, POST, PUT, DELETE) to the routing table
 *   - path: The URI endpoint (e.g., "/status", "/pack/on", "/music/next")
 *   - method: HTTP method enum (HTTP_GET, HTTP_POST, HTTP_PUT, HTTP_DELETE)
 *   - handler: Function pointer to the request handler (e.g., handleGetStatus, handlePackOn)
 *   - summary: Short description of what this endpoint does (used in OpenAPI spec)
 *   - description: Detailed explanation of the endpoint's functionality (used in OpenAPI spec)
 *   - tag: Category/group for organizing endpoints in API documentation (e.g., "System", "Pack Control", "Music Control")
 *   - responseDesc: Description of what a successful response contains (used in OpenAPI spec)
 */
bool addSimpleRoute(const char* path,
                    WebRequestMethodComposite method,
                    void (*handler)(AsyncWebServerRequest *request),
                    const char* summary,
                    const char* description,
                    const char* tag,
                    const char* responseDesc = RESP_SUCCESS_STATUS) {
  if(apiRouteCount >= MAX_API_ROUTES - 1) {
    return false; // Array full (need space for sentinel)
  }

  // Add the new route at the current position
  apiRoutes[apiRouteCount] = {path, method, handler, summary, description, tag, responseDesc};
  apiRouteCount++;

  // Ensure sentinel remains at the end
  apiRoutes[apiRouteCount] = {nullptr, HTTP_ANY, nullptr, nullptr, nullptr, nullptr, nullptr};

  return true;
}

/**
 * Add a body handler route (POST with JSON body) to the routing table
 *   - path: The URI endpoint (e.g., "/config/device/save")
 *   - handler: Pointer to the AsyncCallbackJsonWebHandler object that processes the JSON body
 *   - summary: Short description of what this endpoint does (used in OpenAPI spec)
 *   - description: Detailed explanation of the endpoint's functionality (used in OpenAPI spec)
 *   - tag: Category/group for organizing endpoints in API documentation (e.g., "Configuration", "WiFi")
 *   - requestBodyDesc: Description of the expected JSON structure in the request body (used in OpenAPI spec)
 *   - responseDesc: Description of what a successful response contains (used in OpenAPI spec)
 */
bool addBodyRoute(const char* path,
                  AsyncCallbackJsonWebHandler* handler,
                  const char* summary,
                  const char* description,
                  const char* tag,
                  const char* requestBodyDesc,
                  const char* responseDesc = RESP_SETTINGS_UPDATED) {
  if(bodyRouteCount >= MAX_BODY_ROUTES - 1) {
    return false; // Array full (need space for sentinel)
  }

  // Add the new route at the current position
  bodyHandlerRoutes[bodyRouteCount] = {path, handler, summary, description, tag, requestBodyDesc, responseDesc};
  bodyRouteCount++;

  // Ensure sentinel remains at the end
  bodyHandlerRoutes[bodyRouteCount] = {nullptr, nullptr, nullptr, nullptr, nullptr, nullptr, nullptr};

  return true;
}

/*
 * Helper function to convert HTTP method enum to string for OpenAPI specification
 */
const __FlashStringHelper* httpMethodToString(WebRequestMethodComposite method) {
  if(method == HTTP_GET) return F("get");
  if(method == HTTP_POST) return F("post");
  if(method == HTTP_PUT) return F("put");
  if(method == HTTP_DELETE) return F("delete");
  return F("get");  // Default fallback
}

/*
 * Web API Routing Setup - Use the registered routes to configure the web server
 */

void handleNotFound(AsyncWebServerRequest *request) {
  // Returned for any invalid URL requested.
  request->send(HTTP_STATUS_404, MIME_PLAIN, F("Not Found"));
}

// Define all known URI endpoints for the web server.
// Declare this last as it uses all of the above functions.
// Note: Arrays must be terminated with a sentinel entry (handler = nullptr)
void setupRouting(AsyncWebServer& server) {
  // Dynamically register all routes from the apiRoutes array
  // Loop until we find a sentinel entry (handler == nullptr)
  for(size_t i = 0; apiRoutes[i].handler != nullptr; i++) {
    const RouteDefinition& route = apiRoutes[i];

    // Register the route with the web server
    server.on(route.path, route.method, route.handler);
  }

  // Dynamically register all body handler routes (PUT with JSON body)
  // Loop until we find a sentinel entry (handler == nullptr)
  for(size_t i = 0; bodyHandlerRoutes[i].handler != nullptr; i++) {
    server.addHandler(bodyHandlerRoutes[i].handler);
  }

  // Register the not found handler
  server.onNotFound(handleNotFound);
}

/*
 * OpenAPI 3.x Specification Generator
 */

// Generate OpenAPI 3.x specification as JSON
String generateOpenAPISpec() {
  String spec;
  JsonDocument jsonBody;

  // OpenAPI version and info
  jsonBody["openapi"] = F("3.0.3");

  JsonObject info = jsonBody["info"].to<JsonObject>();
  info["title"] = F("GPStar Device API");
  info[JSON_PROPERTY_DESCRIPTION] = F("RESTful API for controlling this GPStar device");
  info["version"] = F("6.2.x");

  JsonObject contact = info["contact"].to<JsonObject>();
  contact[JSON_PROPERTY_NAME] = F("GPStar Technologies");
  contact[JSON_PROPERTY_URL] = F("https://www.gpstartechnologies.com");

  // External documentation
  JsonObject externalDocs = jsonBody["externalDocs"].to<JsonObject>();
  externalDocs[JSON_PROPERTY_DESCRIPTION] = F("GPStar Documentation Home");
  externalDocs[JSON_PROPERTY_URL] = F("https://gpstar81.github.io/GPStar-proton-pack/");

  // Servers
  JsonArray servers = jsonBody["servers"].to<JsonArray>();
  
  // Add MDNS server entry, if available
  if(deviceMdnsName.length() > 0) {
    JsonObject mdnsServer = servers.add<JsonObject>();
    mdnsServer[JSON_PROPERTY_URL] = String(F("http://")) + deviceMdnsName;
    mdnsServer[JSON_PROPERTY_DESCRIPTION] = F("Local device server (mDNS)");
  }
  
  // Add local IP address server entry, if available
  if(deviceIpAddress.length() > 0) {
    JsonObject ipServer = servers.add<JsonObject>();
    ipServer[JSON_PROPERTY_URL] = String(F("http://")) + deviceIpAddress;
    ipServer[JSON_PROPERTY_DESCRIPTION] = F("Local device server (Local IP)");
  }

  // Add external IP address server entry using callback (skip if 0.0.0.0 or not set)
  if(getExternalIpCallback != nullptr) {
    String currentExternalIp = getExternalIpCallback();
    if(currentExternalIp.length() > 0 && !currentExternalIp.equals("0.0.0.0")) {
      JsonObject ipServer = servers.add<JsonObject>();
      ipServer[JSON_PROPERTY_URL] = String(F("http://")) + currentExternalIp;
      ipServer[JSON_PROPERTY_DESCRIPTION] = F("Local device server (WiFi IP)");
    }
  }

  // Paths object
  JsonObject paths = jsonBody["paths"].to<JsonObject>();

  // Add regular routes (loop until sentinel entry with nullptr handler)
  for(size_t i = 0; apiRoutes[i].handler != nullptr; i++) {
    const RouteDefinition& route = apiRoutes[i];

    // Skip static asset routes and captive portal endpoints for cleaner API documentation
    if(route.tag == TAG_ASSETS || route.tag == TAG_PAGES || route.tag == TAG_PORTAL) {
      continue;
    }

    // Obtain the given path as expected for ESPAsyncWebServer
    String openApiPath = String(route.path);

    // Transform the path for OpenAPI documentation, if needed
    if(openApiPath.equals("/wifi/network/*")) {
      openApiPath = "/wifi/network/{index}";
    }
    if(openApiPath.equals("/power/set/*")) {
      openApiPath = "/power/set/{powerLevel}";
    }
    if(openApiPath.equals("/volume/master/set/*")) {
      openApiPath = "/volume/master/set/{percentage}";
    }

    // Create path object
    JsonObject pathItem = paths[openApiPath].to<JsonObject>();
    JsonObject operation = pathItem[httpMethodToString(route.method)].to<JsonObject>();

    operation[JSON_PROPERTY_SUMMARY] = route.summary;
    operation[JSON_PROPERTY_DESCRIPTION] = route.description;

    JsonArray tags = operation[JSON_PROPERTY_TAGS].to<JsonArray>();
    tags.add(route.tag);

    // Add query parameters for specific, consistent routes
    if(openApiPath.equals("/music/select")) {
      JsonArray parameters = operation["parameters"].to<JsonArray>();
      JsonObject trackParam = parameters.add<JsonObject>();
      trackParam[JSON_PROPERTY_NAME] = F("track");
      trackParam["in"] = F("query");
      trackParam[JSON_PROPERTY_DESCRIPTION] = F("Track number to select (1-based index)");
      trackParam[JSON_PROPERTY_REQUIRED] = true;
      JsonObject paramSchema = trackParam[JSON_PROPERTY_SCHEMA].to<JsonObject>();
      paramSchema[JSON_PROPERTY_TYPE] = F("integer");
      paramSchema["minimum"] = 500;
    }
    if(openApiPath.equals("/infrared/signal")) {
      JsonArray parameters = operation["parameters"].to<JsonArray>();
      JsonObject trackParam = parameters.add<JsonObject>();
      trackParam[JSON_PROPERTY_NAME] = F("type");
      trackParam["in"] = F("query");
      trackParam[JSON_PROPERTY_DESCRIPTION] = F("IR signal type to send");
      trackParam[JSON_PROPERTY_REQUIRED] = true;
      JsonObject paramSchema = trackParam[JSON_PROPERTY_SCHEMA].to<JsonObject>();
      paramSchema[JSON_PROPERTY_TYPE] = F("string");
      JsonArray enumValues = paramSchema["enum"].to<JsonArray>();
      enumValues.add(F("ghostintrap"));
      enumValues.add(F("firing"));
      enumValues.add(F("gpstartest"));
      paramSchema["default"] = F("ghostintrap");
    }

    // Add path parameters for specific, consistent routes
    if(openApiPath.equals("/wifi/network/{index}")) {
      JsonArray parameters = operation["parameters"].to<JsonArray>();
      JsonObject indexParam = parameters.add<JsonObject>();
      indexParam[JSON_PROPERTY_NAME] = F("index");
      indexParam["in"] = F("path");
      indexParam[JSON_PROPERTY_DESCRIPTION] = F("Index of the saved network to delete (0-based)");
      indexParam[JSON_PROPERTY_REQUIRED] = true;
      JsonObject paramSchema = indexParam[JSON_PROPERTY_SCHEMA].to<JsonObject>();
      paramSchema[JSON_PROPERTY_TYPE] = F("integer");
      paramSchema["minimum"] = 0;
    }
    if(openApiPath.equals("/power/set/{powerLevel}")) {
      JsonArray parameters = operation["parameters"].to<JsonArray>();
      JsonObject percentageParam = parameters.add<JsonObject>();
      percentageParam[JSON_PROPERTY_NAME] = F("powerLevel");
      percentageParam["in"] = F("path");
      percentageParam[JSON_PROPERTY_DESCRIPTION] = F("Power Level to set (1-5)");
      percentageParam[JSON_PROPERTY_REQUIRED] = true;
      JsonObject paramSchema = percentageParam[JSON_PROPERTY_SCHEMA].to<JsonObject>();
      paramSchema[JSON_PROPERTY_TYPE] = F("integer");
      paramSchema["minimum"] = 1;
      paramSchema["maximum"] = 5;
    }
    if(openApiPath.equals("/volume/master/set/{percentage}")) {
      JsonArray parameters = operation["parameters"].to<JsonArray>();
      JsonObject percentageParam = parameters.add<JsonObject>();
      percentageParam[JSON_PROPERTY_NAME] = F("percentage");
      percentageParam["in"] = F("path");
      percentageParam[JSON_PROPERTY_DESCRIPTION] = F("Percentage of the master volume to set (0-100)");
      percentageParam[JSON_PROPERTY_REQUIRED] = true;
      JsonObject paramSchema = percentageParam[JSON_PROPERTY_SCHEMA].to<JsonObject>();
      paramSchema[JSON_PROPERTY_TYPE] = F("integer");
      paramSchema["minimum"] = 0;
      paramSchema["maximum"] = 100;
    }

    JsonObject responses = operation[JSON_PROPERTY_RESPONSES].to<JsonObject>();
    JsonObject response200 = responses["200"].to<JsonObject>();
    response200[JSON_PROPERTY_DESCRIPTION] = route.responseDesc;

    JsonObject content = response200[JSON_PROPERTY_CONTENT].to<JsonObject>();
    JsonObject appJson = content[MIME_JSON].to<JsonObject>();
    JsonObject schema = appJson[JSON_PROPERTY_SCHEMA].to<JsonObject>();
    schema[JSON_PROPERTY_TYPE] = JSON_TYPE_OBJECT;
  }

  // Add body handler routes (POST with JSON body, loop until sentinel with nullptr handler)
  for(size_t i = 0; bodyHandlerRoutes[i].handler != nullptr; i++) {
    const BodyHandlerDefinition& route = bodyHandlerRoutes[i];

    // Create path object
    JsonObject pathItem = paths[route.path].to<JsonObject>();
    JsonObject operation = pathItem["post"].to<JsonObject>();

    operation[JSON_PROPERTY_SUMMARY] = route.summary;
    operation[JSON_PROPERTY_DESCRIPTION] = route.description;

    JsonArray tags = operation[JSON_PROPERTY_TAGS].to<JsonArray>();
    tags.add(route.tag);

    // Request body
    JsonObject requestBody = operation["requestBody"].to<JsonObject>();
    requestBody[JSON_PROPERTY_DESCRIPTION] = route.requestBodyDesc;
    requestBody[JSON_PROPERTY_REQUIRED] = true;

    JsonObject reqContent = requestBody[JSON_PROPERTY_CONTENT].to<JsonObject>();
    JsonObject reqAppJson = reqContent[MIME_JSON].to<JsonObject>();
    JsonObject reqSchema = reqAppJson[JSON_PROPERTY_SCHEMA].to<JsonObject>();
    reqSchema[JSON_PROPERTY_TYPE] = JSON_TYPE_OBJECT;

    // Response
    JsonObject responses = operation[JSON_PROPERTY_RESPONSES].to<JsonObject>();
    JsonObject response200 = responses["200"].to<JsonObject>();
    response200[JSON_PROPERTY_DESCRIPTION] = route.responseDesc;

    JsonObject respContent = response200[JSON_PROPERTY_CONTENT].to<JsonObject>();
    JsonObject respAppJson = respContent[MIME_JSON].to<JsonObject>();
    JsonObject respSchema = respAppJson[JSON_PROPERTY_SCHEMA].to<JsonObject>();
    respSchema[JSON_PROPERTY_TYPE] = JSON_TYPE_OBJECT;

    JsonObject properties = respSchema[JSON_PROPERTY_PROPERTIES].to<JsonObject>();
    JsonObject statusProp = properties[JSON_PROPERTY_STATUS].to<JsonObject>();
    statusProp[JSON_PROPERTY_TYPE] = JSON_TYPE_STRING;
  }

  // Collect used tags from all routes
  std::set<String> usedTags;

  // Scan regular routes while skipping HTML pages, assets (images, CSS, JS), and captive portal endpoints
  for(size_t i = 0; apiRoutes[i].handler != nullptr; i++) {
    if(apiRoutes[i].tag != TAG_ASSETS && apiRoutes[i].tag != TAG_PAGES && apiRoutes[i].tag != TAG_PORTAL) {
      usedTags.insert(String(apiRoutes[i].tag));
    }
  }

  // Scan body handler routes which default to POST and expect a JSON body
  for(size_t i = 0; bodyHandlerRoutes[i].handler != nullptr; i++) {
    usedTags.insert(String(bodyHandlerRoutes[i].tag));
  }

  // Tags for grouping - only add tags that are actually used
  JsonArray tags = jsonBody[JSON_PROPERTY_TAGS].to<JsonArray>();

  // Helper function to add tags conditionally (when used by any route)
  auto addTagIfUsed = [&](const char* tagName, const __FlashStringHelper* description) {
    if(usedTags.find(String(tagName)) != usedTags.end()) {
      JsonObject tag = tags.add<JsonObject>();
      tag[JSON_PROPERTY_NAME] = tagName;
      tag[JSON_PROPERTY_DESCRIPTION] = description;
    }
  };

  addTagIfUsed(TAG_CONFIGURATION, F("Device Configuration/Preferences"));
  addTagIfUsed(TAG_EEPROM, F("EEPROM Save Operations"));
  addTagIfUsed(TAG_SYSTEM, F("System Status and Control"));
  addTagIfUsed(TAG_DEVICE_CONTROL, F("Device Control/Operations"));
  addTagIfUsed(TAG_SENSOR_CONTROL, F("Sensor Control/Operations"));
  addTagIfUsed(TAG_THEMES, F("System Theme Selection"));
  addTagIfUsed(TAG_STREAM_MODES, F("Stream Mode Selection"));
  addTagIfUsed(TAG_VOLUME_CONTROL, F("Audio Volume Control"));
  addTagIfUsed(TAG_MUSIC_CONTROL, F("Music Playback Control"));
  addTagIfUsed(TAG_WIFI, F("WiFi Network Management"));
  addTagIfUsed(TAG_DOCUMENTATION, F("API Documentation"));

  // Serialize JSON object to string
  serializeJson(jsonBody, spec);
  return spec;
}

// Handler for OpenAPI specification endpoint at http://<device>/openapi.json
void handleOpenAPISpec(AsyncWebServerRequest *request) {
  request->send(HTTP_STATUS_200, MIME_JSON, generateOpenAPISpec());
}
