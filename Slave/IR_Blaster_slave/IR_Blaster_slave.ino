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

void setup()
{
    Serial.begin(115200);
    while (!Serial)
        ;

    Serial.printf("Send IR signals at pin %d\n", IR_SEND_PIN);
    IrSender.begin();
    disableLEDFeedback();

    WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
    while (WiFi.status() != WL_CONNECTED)
    {
        delay(1000);
        Serial.println("Connecting to WiFi...");
    }
    Serial.print("Connected to WiFi, ");
    Serial.println(WiFi.localIP());
    delay(500);

    webSocket.begin("192.168.100.233", 81, "/");
    webSocket.onEvent(handleWebSocketEvent);
    webSocket.setReconnectInterval(1000);
}

void loop()
{
    webSocket.loop();

    CheckNetworkReconnect();
}

void onText(WStype_t type, uint8_t *payload, size_t length)
{
    Serial.printf("Received message: %s\n", payload);
    String data = String((char *)payload);
    if (data.indexOf(APPLY_CMD_CODE) >= 0)
    {
        Serial.println("Applying new command! ");

        int firstComma = data.indexOf(',');
        String code = data.substring(0, firstComma);
        String cmd_number = data.substring(firstComma + 1);

        int parsed_cmd_number = cmd_number.toInt();
        Serial.printf("SLAVE GOT CMD NUMBER: %d\n", parsed_cmd_number);

        switch (parsed_cmd_number)
        {
        case 0: // prender LEDs
            Serial.println("Prendiendo LEDs");
            IrSender.sendNEC(0xEF00, 0x3, 1);
            delay(100);
            IrSender.sendPulseDistanceWidth(38, 9050, 4450, 650, 1650, 650, 550, 0x25060050C, 35, PROTOCOL_IS_LSB_FIRST, 0, 0);
            break;
        case 1: // apagar LEDs
            Serial.println("Apagando LEDs");
            IrSender.sendNEC(0xEF00, 0x2, 1);
            delay(100);
            IrSender.sendPulseDistanceWidth(38, 9000, 4450, 650, 1650, 650, 550, 0x25060050C, 35, PROTOCOL_IS_LSB_FIRST, 0, 0);
            break;
        default:
            Serial.println("ERROR: no se pudo encontrar el comando enviado! " + cmd_number);
            break;
        }
    }
}