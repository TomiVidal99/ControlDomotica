#include <WiFi.h>
#include <WebSocketsClient.h>
#include <Arduino.h>
#include "../PinDefinitionsAndMore.h"
#include <IRremote.hpp>

// these definitions have to be defined before slave.h
#define DEVICE_NAME "IR_BLASTER"
#define DEVICE_LOCATION "Habitacion 1"
#define DEVICE_DESCRIPTION "Lámpara LED"

#include "../slave.h"
#include "../slave.ino"

void onText(WStype_t type, uint8_t *payload, size_t length);

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  while (!Serial)
    ;
#endif

  connectToWiFi();

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
    char newSSID[32] = "";
    char newPasswd[64] = "";
    ssid.toCharArray(newSSID, sizeof(newSSID));
    password.toCharArray(newPasswd, sizeof(newPasswd));

    handleNewWiFiCredentials(newSSID, newPasswd);
  }
}