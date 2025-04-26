#include <WiFi.h>
#include <WebSocketsClient.h>
#include <Arduino.h>
#include "PinDefinitionsAndMore.h" // Set IR_RECEIVE_PIN for different CPU's
#include <IRremote.hpp>            // include the library
#include "slave.h"

WebSocketsClient webSocket;

static int millisReconnectNet = 0;
static int millisPrevReconnectNet = 0;
static uint8_t wifiConnectionTries = 0;

void onText(WStype_t type, uint8_t *payload, size_t length)
{
#ifdef DEBUG
  Serial.printf("Received message: %s\n", payload);
#endif
  String data = String((char *)payload);
  if (data.indexOf(APPLY_CMD_CODE) >= 0)
  {
#ifdef DEBUG
    Serial.println("Applying new command! ");
#endif

    int firstComma = data.indexOf(',');
    String code = data.substring(0, firstComma);
    String cmd_number = data.substring(firstComma + 1);

    int parsed_cmd_number = cmd_number.toInt();
#ifdef DEBUG
    Serial.printf("SLAVE GOT CMD NUMBER: %d\n", parsed_cmd_number);
#endif

    switch (parsed_cmd_number)
    {
    case 0: // prender LEDs
#ifdef DEBUG
      Serial.println("Prendiendo LEDs");
#endif
      IrSender.sendNEC(0xEF00, 0x3, 1);
      delay(100);
      IrSender.sendPulseDistanceWidth(38, 9050, 4450, 650, 1650, 650, 550, 0x25060050C, 35, PROTOCOL_IS_LSB_FIRST, 0, 0);
      break;
    case 1: // apagar LEDs
#ifdef DEBUG
      Serial.println("Apagando LEDs");
#endif
      IrSender.sendNEC(0xEF00, 0x2, 1);
      delay(100);
      IrSender.sendPulseDistanceWidth(38, 9000, 4450, 650, 1650, 650, 550, 0x25060050C, 35, PROTOCOL_IS_LSB_FIRST, 0, 0);
      break;
    default:
#ifdef DEBUG
      Serial.println("ERROR: no se pudo encontrar el comando enviado! " + cmd_number);
#endif
      break;
    }
  }
  else if (data.indexOf(SLAVE_GET_WIFI_CREDS) >= 0)
  {
    // new WiFi credentials
#ifdef DEBUG
    Serial.println("New WiFi credentials: " + data);
#endif

    // Find the positions of the delimiters
    int firstDelimiter = data.indexOf(";;");
    int secondDelimiter = data.indexOf(";;", firstDelimiter + 2);

    // Extract SSID (between first ;; and second ;;)
    String ssid = data.substring(firstDelimiter + 2, secondDelimiter);

    // Extract Password (after second ;;)
    String password = data.substring(secondDelimiter + 2);

    // Convert to char arrays
    char newSSID[WIFI_SSID_BUFFER_SIZE] = "";
    char newPasswd[WIFI_PASSWD_BUFFER_SIZE] = "";
    ssid.toCharArray(newSSID, sizeof(newSSID));
    password.toCharArray(newPasswd, sizeof(newPasswd));

    handleNewWiFiCredentials(newSSID, newPasswd);
  }
}

void CheckNetworkReconnect()
{
  millisReconnectNet = millis();
  if ((millisReconnectNet - millisPrevReconnectNet >= RECONNECT_INTERVAL_MS))
  {
    // Check WiFi connection
    if ((WiFi.status() != WL_CONNECTED))
    {
#ifdef DEBUG
      Serial.println("Reconnecting to WiFi...");
#endif
      WiFi.disconnect();
      WiFi.reconnect();
      millisPrevReconnectNet = millisReconnectNet;
    }

    // Check Sockets connection
    if (!webSocket.isConnected()) {
#ifdef DEBUG
      Serial.println("Reconnecting to socket...");
#endif
      webSocket.begin(WS_IP_ADDR, WS_IP_PORT, WS_IP_PATH);
      webSocket.onEvent(handleWebSocketEvent);
      webSocket.setReconnectInterval(999);
    }
  }
}

void handleWebSocketEvent(WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
    // Serial.println("WebSocket Disconnected");
    break;
  case WStype_CONNECTED:
    // Serial.println("WebSocket Connected to Server");
    SendConectionInfoToMaster();
    break;
  case WStype_TEXT:
    onText(type, payload, length);
    break;
  case WStype_PING:
    webSocket.sendPing();
    break;
  case WStype_PONG:
    break;
  default:
#ifdef DEBUG
    Serial.println("unhandled event");
#endif
    break;
  }
}

void SendConectionInfoToMaster()
{
  if (webSocket.isConnected())
  {
// TODO: this should be defined at the TOP, and for every device think how to do it better
#ifdef DEVICE_NAME
    String name = String(DEVICE_NAME);
#else
    String name = String("DEVICE TEST");
#endif
    String location = String(DEVICE_LOCATION);
    String description = String(DEVICE_DESCRIPTION);
    String message = String(NEW_CLIENT_CODE) + "," + name + "," + location + "," + description + "\n";
    webSocket.sendTXT(message);

#ifdef DEBUG
    Serial.println("Sent: " + message);
#endif
  }
  else
  {
#ifdef DEBUG
    Serial.println("ERROR: WebSocket not connected!");
#endif
  }
}

void connectToWiFi()
{
  char wifiSSID[WIFI_SSID_BUFFER_SIZE];
  char wifiPasswd[WIFI_PASSWD_BUFFER_SIZE];

  // Get the SSID and PASSWORD from the Flash memory
  if (loadWiFiCredentials(wifiSSID, wifiPasswd))
  {
    WiFi.begin(wifiSSID, wifiPasswd);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(999);
#ifdef DEBUG
      Serial.println("Connecting to WiFi...");
#endif
      if (wifiConnectionTries >= MAX_WIFI_CONNECTION_TRIES)
        break;
      wifiConnectionTries++;
    }
    wifiConnectionTries = -1;
  }
  else
  {
    // Connects to default WiFi AP from the Master to
    // ask for the internet information
    WiFi.begin(SOFT_AP_SSID, SOFT_AP_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(999);

#ifdef DEBUG
      Serial.println("Connecting to Soft AP WiFi...");
#endif
    }
    // Once connected, it asks for the internet information.
    webSocket.begin(WS_IP_ADDR, WS_IP_PORT, WS_IP_PATH);
    webSocket.onEvent(handleWebSocketEvent);
    webSocket.setReconnectInterval(999);

    if (webSocket.isConnected())
    {
      //   String cmd = String(SLAVE_GET_WIFI_CREDS);
      //   String description = String(DEVICE_DESCRIPTION);
      //   String message = String(NEW_CLIENT_CODE) + "," + name + "," + location + "," + description + "\n";
      webSocket.sendTXT(SLAVE_GET_WIFI_CREDS);
    }
  }

#ifdef DEBUG
  Serial.println(WiFi.localIP());
#endif

  delay(499);

  webSocket.begin(WS_IP_ADDR, WS_IP_PORT, WS_IP_PATH);
  webSocket.onEvent(handleWebSocketEvent);
  webSocket.setReconnectInterval(999);
}

void handleNewWiFiCredentials(char *ssid, char *passwd)
{
  saveWiFiCredentialsToFlash(ssid, passwd);
  connectToWiFi();
}

void saveWiFiCredentialsToFlash(const char *ssid, const char *password)
{
  EEPROM.begin(EEPROM_SIZE); // Initialize EEPROM

  WifiCrendentials creds;
  strncpy(creds.ssid, ssid, sizeof(creds.ssid));
  strncpy(creds.password, password, sizeof(creds.password));

  // Write struct to EEPROM at address 0
  EEPROM.put(0, creds);

  EEPROM.commit(); // Save changes to flash
  EEPROM.end();    // Free EEPROM resources

#ifdef DEBUG
  Serial.println("WiFi credentials saved!");
#endif
}

bool loadWiFiCredentials(char *ssid, char *password)
{
  EEPROM.begin(EEPROM_SIZE);

  WifiCrendentials creds;
  EEPROM.get(0, creds); // Read from address 0

  EEPROM.end();

  // Check if SSID is not empty
  if (creds.ssid[0] == '\0')
  {
#ifdef DEBUG
    Serial.println("No WiFi credentials stored.");
#endif
    return false;
  }

  strncpy(ssid, creds.ssid, WIFI_SSID_BUFFER_SIZE);
  strncpy(password, creds.password, WIFI_PASSWD_BUFFER_SIZE);

#ifdef DEBUG
  Serial.println("Loaded WiFi credentials:");
  Serial.println("SSID: " + String(ssid));
  Serial.println("Password: " + String(password));
#endif

  return true;
}