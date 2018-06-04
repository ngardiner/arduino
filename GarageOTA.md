# Garage Opener

## Introduction

This sketch was created for the esp8266 WeMos D1 Mini wifi board, and provides the following:

   * Exposes a HTTP web form for controlling and viewing garage door state
   * Exposes an API endpoint compatible with Home Assistant to control the door via a RESTful switch
   
   
## HomeAssistant

The /api endpoint exposed by the device is intended to allow HomeAssistant to

### RESTful Switch Definiton

The following definition under the switch: configuration tree will define a RESTful switch to control the Garage Door.

```
- platform: rest
  name: Garage Door Switch
  resource: http://192.168.xxx.xxx/api
  body_on: 'toggle'
  body_off: 'toggle'
  is_on_template: '{{ value_json.closed }}'
  headers:
    Content-Type: text/plain
```
