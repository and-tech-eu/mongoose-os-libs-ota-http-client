# Implementation of Mongoose OS OTA HTTP client

This library adds a device configuration section called `update`, where
a device could be configured to poll a specified HTTP URL for a new
app firmware.

Also, this library adds a C API to fetch a new firmware from the given
URL and update programmatically.

## Configuration section

The library adds the following object to the device configuration:


```javascript
  "update": {
    "commit_timeout": 0,        // OTA commit timeout
    "url": "",                  // HTTP URL to poll
    "interval": 0,              // Polling interval
    "extra_http_headers": "",   // Extra HTTP request headers
    "ssl_ca_file": "ca.pem",    // TLS CA cert file
    "ssl_client_cert_file": "", // TLS cert file
    "ssl_server_name": "",      // TLS server name
    "enable_post": true
  }
```

## Request Headers

The library includes the following additional headers with the request:

  * `X-MGOS-Device-ID: ID MAC`
    * `ID` is the value of the `device.id` configuration variable, if set, otherwise `-`.
    * `MAC` is device's primary MAC address (as returned by `device_get_mac_address()`.
  * `User-Agent: APP/APP_VERSION (MongooseOS/MGOS_VERSION; PLATFORM)`
    * `APP` is the name of the application.
    * `APP_VERSION` is the version of the application (from mos.yml)
    * `MGOS_VERSION` is the version of Mongoose OS
    * `PLATFORM` is the target platform
  * `X-MGOS-FW-Version: PLATFORM APP_VERSION APP_BUILD_ID`
    * `PLATFORM` is the target platform
    * `APP_VERSION` is the version of the application (from mos.yml)
    * `APP_BUILD_ID` is the build identification string (generated at build time)

## Response Codes

Client code supports redirects (301 or 302 response code + `Location` header).

Client interprets `304 Not Modified` as "no update at this time".

## Server Side Code

Example of a server that serves uodates depending on the current version of the firmware can be found [here](https://github.com/cesanta/mongoose-os/blob/master/tools/updateredirector/updateredirector.go).
