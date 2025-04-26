#include <WiFi.h>
#include <WebSocketsClient.h>
#include <Arduino.h>
#if !defined(ARDUINO_ESP32C3_DEV) // This is due to a bug in RISC-V compiler, which requires unused function sections :-(.
#define DISABLE_CODE_FOR_RECEIVER // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#endif
#include "PinDefinitionsAndMore.h" // Set IR_RECEIVE_PIN for different CPU's
#include <IRremote.hpp>            // include the library
#include <EEPROM.h>

#ifndef __SLAVE_
#define __SLAVE_

#define WIFI_SSID_BUFFER_SIZE 32
#define WIFI_PASSWD_BUFFER_SIZE 64

// If you want to have Serial output enable DEBUG
// #define DEBUG
typedef struct WifiCrendentials
{
    char ssid[32];
    char password[64];
};

#define EEPROM_SIZE sizeof(WifiCrendentials)
#define EEPROM_ADDR 0

#define SOFT_AP_SSID "DOMOTICA"
#define SOFT_AP_PASSWORD "123456789"

// #define WIFI_SSID "ESP32_AP_TEST"
// #define WIFI_PASSWORD "AP123456"
// #define WS_IP_ADDR "192.168.4.2"
#define WS_IP_ADDR "domotica.local" // this has to be the same as MDNS_DEVICE_ALIAS in the master. TODO: they should be the same macro
#define WS_IP_PORT ((uint16_t)81)
#define WS_IP_PATH "/"
// #define WIFI_SSID "Casa Amarilla"
// #define WIFI_PASSWORD "mariposa15"
#define RECONNECT_INTERVAL_MS 30000
#define RECV_PIN 15
#define IR_SEND_PIN 2

#define MAX_WIFI_CONNECTION_TRIES 20

// COMMUNICATION CODES
#define NEW_CLIENT_CODE "new::client"
#define REQUEST_INFO_CODE "req::info"
#define REQUEST_DEVICES_CODE "get::devices"
#define REMOVED_DEVICE_CODE "removed::client"
#define APPLY_CMD_CODE "use::cmd"
#define UPDATE_CREDENTIALS_CMD "set::lan"
#define CREDENTIALS_UPDATED_ACK "cred::success"
#define SLAVE_GET_WIFI_CREDS "req::creds"

// DEVICE INFORMATION
// TODO: this are the things that have to be modify
// in every slave!!!
// #define DEVICE_NAME "IR_BLASTER_TEST"
// #define DEVICE_LOCATION "habitacion 1"
// #define DEVICE_DESCRIPTION "Controlador del aire acondicionado"

void CheckNetworkReconnect(void);                                          // checks if the LAN it's disconnected and attempts to reconnect                                                     // sends an HTTP request to the master website (NO REQUIERED NOW). TODO: remove this
void SendConectionInfoToMaster(void);                                      // sends the information of this device to the host
void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length); // parses the information recieved over the web socket with the master
void CheckReconnectWS(void);
void connectToWiFi(void);
void handleNewWiFiCredentials(char *ssid, char *passwd);                 // callback for when the master sends the new credentials
void saveWiFiCredentialsToFlash(const char *ssid, const char *password); // Save to non volatile memory (Flash) the SSID and PASSWORD
bool loadWiFiCredentials(char *ssid, char *password);                    // Get SSID and PASSWORD from Flash, if they exist returns true

// This is the function that handles the processing of the information once a device sends it
// this is basically what it changes between the different types of slaves
void onText(WStype_t type, uint8_t *payload, size_t length);

#endif