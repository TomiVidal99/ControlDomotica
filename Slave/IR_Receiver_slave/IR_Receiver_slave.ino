/*
 * Specify which protocol(s) should be used for decoding.
 * If no protocol is defined, all protocols (except Bang&Olufsen) are active.
 * This must be done before the #include <IRremote.hpp>
 */
//#define DECODE_DENON        // Includes Sharp
//#define DECODE_JVC
//#define DECODE_KASEIKYO
//#define DECODE_PANASONIC    // alias for DECODE_KASEIKYO
//#define DECODE_LG
// #define DECODE_NEC          // Includes Apple and Onkyo. To enable all protocols , just comment/disable this line.
//#define DECODE_SAMSUNG
//#define DECODE_SONY
//#define DECODE_RC5
//#define DECODE_RC6

//#define DECODE_BOSEWAVE
//#define DECODE_LEGO_PF
//#define DECODE_MAGIQUEST
//#define DECODE_WHYNTER
//#define DECODE_FAST

//#define DECODE_DISTANCE_WIDTH // Universal decoder for pulse distance width protocols
//#define DECODE_HASH         // special decoder for all protocols

//#define DECODE_BEO          // This protocol must always be enabled manually, i.e. it is NOT enabled if no protocol is defined. It prevents decoding of SONY!

//#define DEBUG               // Activate this for lots of lovely debug output from the decoders.

//#define RAW_BUFFER_LENGTH  750 // For air condition remotes it requires 750. Default is 200.

#include <WiFi.h>
#include <WebSocketsClient.h>
#include <Arduino.h>
#include "../PinDefinitionsAndMore.h"
#include <IRremote.hpp>

// these definitions have to be defined before slave.h
#define DEVICE_NAME "Receptor IR"
#define DEVICE_LOCATION "Habitacion 1"
#define DEVICE_DESCRIPTION "Receptor de comandos IR"

#include "../slave.h"
#include "../slave.ino"

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;

  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.print("Connected to WiFi, ");
  Serial.println(WiFi.localIP());
  delay(500);

  webSocket.begin("192.168.100.233", 81, "/");
  webSocket.onEvent(handleWebSocketEvent);
  webSocket.setReconnectInterval(1000);

  IrReceiver.begin(IR_RECEIVE_PIN, ENABLE_LED_FEEDBACK);
  Serial.printf("Ready to receive IR signals of protocols at pin %d\n", IR_RECEIVE_PIN);
  printActiveIRProtocols(&Serial);
}

void loop() {
  webSocket.loop();

  CheckNetworkReconnect();

  if (IrReceiver.decode()) {

    /*
         * Print a summary of received data
         */
    if (IrReceiver.decodedIRData.protocol == UNKNOWN) {
      Serial.println(F("Received noise or an unknown (or not yet enabled) protocol"));
      // We have an unknown protocol here, print extended info
      IrReceiver.printIRResultRawFormatted(&Serial, true);

      IrReceiver.resume();  // Do it here, to preserve raw data for printing with printIRResultRawFormatted()
    } else {
      IrReceiver.resume();  // Early enable receiving of the next IR frame

      IrReceiver.printIRResultShort(&Serial);
      IrReceiver.printIRSendUsage(&Serial);
    }
    Serial.println();

    /*
         * Finally, check the received data and perform actions according to the received command
         */
    // if (IrReceiver.decodedIRData.flags & IRDATA_FLAGS_IS_REPEAT) {
    //   Serial.println(F("Repeat received. Here you can repeat the same action as before."));
    // } else {
    //   if (IrReceiver.decodedIRData.command == 0x10) {
    //     // do something
    //   } else if (IrReceiver.decodedIRData.command == 0x11) {
    //     // do something else
    //   }
    // }
  }
}

void onText(WStype_t type, uint8_t *payload, size_t length) {
  Serial.printf("Received message: %s\n", payload);
  String data = String((char *)payload);
}