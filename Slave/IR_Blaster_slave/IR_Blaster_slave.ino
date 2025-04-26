#include <WiFi.h>
#include <WebSocketsClient.h>
#include <Arduino.h>
#include "../PinDefinitionsAndMore.h"
#include <IRremote.hpp>

// If you want to have logging messages
// #define DEBUG

// these definitions have to be defined before slave.h
#define DEVICE_NAME "IR_BLASTER"
#define DEVICE_LOCATION "Habitacion 1"
#define DEVICE_DESCRIPTION "Lámpara LED"

#include "../slave.h"
#include "../slave.ino"

extern WebSocketsClient webSocket;

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial)
    ;
#endif

  connectToWiFi();

  //  IR stuff
#ifdef DEBUG
  Serial.printf("Send IR signals at pin %d\n", IR_SEND_PIN);
#endif
  IrSender.begin();
  disableLEDFeedback();
}

void loop()
{
  webSocket.loop();
  CheckNetworkReconnect();
}