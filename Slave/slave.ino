#include <WiFi.h>
#include <WebSocketsClient.h>
#include <Arduino.h>
#include "PinDefinitionsAndMore.h" // Set IR_RECEIVE_PIN for different CPU's
#include <IRremote.hpp>            // include the library
#include "slave.h"

int millisReconnectNet = 0;
int millisPrevReconnectNet = 0;
WebSocketsClient webSocket;

void CheckNetworkReconnect()
{
  millisReconnectNet = millis();
  if ((WiFi.status() != WL_CONNECTED) && (millisReconnectNet - millisPrevReconnectNet >= RECONNECT_INTERVAL_MS))
  {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    millisPrevReconnectNet = millisReconnectNet;
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
    Serial.println("unhandled event");
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
    Serial.println("Sent: " + message);
  }
  else
  {
    Serial.println("ERROR: WebSocket not connected!");
  }
}