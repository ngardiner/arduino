# Garage Opener

## Introduction

This sketch was created for the esp8266 WeMos D1 Mini wifi board, and provides the following:

   * Exposes a HTTP web form for controlling and viewing garage door state
   * Exposes an API endpoint compatible with Home Assistant to control the door via a RESTful switch

### OTA Functionality

This Arduino sketch is written with OTA functionality to allow OTA updates. Once the sketch is uploaded over USB for the first time, subsequent updates can be sent using the Arduino SDK.

## EEPROM Counter

The Garage OTA sketch provides a toggle switch which allows enabling or disabling the EEPROM counter function enable or disable the EEPROM counter function of the Garage Opener. If enabled, the Arduino will record the number of times that the door is opened or closed. This does contribute to read/write 

The EEPROM on an ESP8266 is actually emulated using flash memory, and has approximately 100K write cycles of lifetime before it fails. 

## HomeAssistant

The ```/api``` endpoint exposed by the device is intended to allow HomeAssistant to send requests and get the Garage Door state.

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
