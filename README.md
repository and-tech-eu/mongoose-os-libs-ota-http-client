# Implementation of Mongoose OS OTA HTTP client

## Overview

This library adds a device configuration section called `update`, where
a device could be configured to poll a specified HTTP URL for a new
app firmware.

Also, this library adds a C API to fetch a new firmware from the given
URL and update programmatically.

## C API

See https://mongoose-os.com/docs/reference/api.html#ota-http-client 

## Configuration section


```javascript
  "update": {
    "timeout": 300,
    "commit_timeout": 0,        // OTA commit timeout
    "url": "",                  // HTTP URL to poll
    "interval": 0,              // Polling interval
    "ssl_ca_file": "ca.pem",    // TLS CA cert file
    "ssl_client_cert_file": "", // TLS cert file
    "ssl_server_name": "",      // TLS server name
    "enable_post": true
  }
```

