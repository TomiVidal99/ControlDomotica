#include <WiFi.h>
#include <WebSocketsClient.h>
#include <Arduino.h>
#if !defined(ARDUINO_ESP32C3_DEV) // This is due to a bug in RISC-V compiler, which requires unused function sections :-(.
#define DISABLE_CODE_FOR_RECEIVER // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#endif
#include "PinDefinitionsAndMore.h" // Set IR_RECEIVE_PIN for different CPU's
#include <IRremote.hpp>            // include the library

// #define WIFI_SSID "ESP32_AP_TEST"
// #define WIFI_PASSWORD "AP123456"
// #define WS_IP_ADDR "192.168.4.2"
#define WS_IP_ADDR "esp32-device.local"
#define WS_IP_PORT ((uint16_t)81)
#define WS_IP_PATH "/"
#define WIFI_SSID "Casa Amarilla"
#define WIFI_PASSWORD "mariposa15"
#define RECONNECT_INTERVAL_MS 30000
#define RECV_PIN 15
#define IR_SEND_PIN 2

// COMMUNICATION CODES
#define NEW_CLIENT_CODE "new::client"
#define APPLY_CMD_CODE "use::cmd"

// DEVICE INFORMATION
// TODO: this are the things that have to be modify
// in every slave!!!
// #define DEVICE_NAME "IR_BLASTER_TEST"
// #define DEVICE_LOCATION "habitacion 1"
// #define DEVICE_DESCRIPTION "Controlador del aire acondicionado"

void CheckNetworkReconnect();                                              // checks if the LAN it's disconnected and attempts to reconnect                                                     // sends an HTTP request to the master website (NO REQUIERED NOW). TODO: remove this
void SendConectionInfoToMaster();                                          // sends the information of this device to the host
void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length); // parses the information recieved over the web socket with the master
void CheckReconnectWS();

// This is the function that handles the processing of the information once a device sends it
// this is basically what it changes between the different types of slaves
void onText(WStype_t type, uint8_t *payload, size_t length);