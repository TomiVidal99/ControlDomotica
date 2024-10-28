#include <WiFi.h>
#include <WebSocketsClient.h>
#include <HTTPClient.h>

// IR Sender Stuff -------------------------
#include <Arduino.h>
#if !defined(ARDUINO_ESP32C3_DEV)  // This is due to a bug in RISC-V compiler, which requires unused function sections :-(.
#define DISABLE_CODE_FOR_RECEIVER  // Disables static receiver code like receive timer ISR handler and static IRReceiver and irparams data. Saves 450 bytes program memory and 269 bytes RAM if receiving functions are not required.
#endif
//#define SEND_PWM_BY_TIMER         // Disable carrier PWM generation in software and use (restricted) hardware PWM.
//#define USE_NO_SEND_PWM           // Use no carrier PWM, just simulate an active low receiver signal. Overrides SEND_PWM_BY_TIMER definition

// IR Sender Stuff
#include "PinDefinitionsAndMore.h"  // Set IR_RECEIVE_PIN for different CPU's
#include <IRremote.hpp>             // include the library
//-------------------------------------

/*
 * Helper macro for getting a macro definition as string
 */
#if !defined(STR_HELPER)
#define STR_HELPER(x) #x
#define STR(x) STR_HELPER(x)
#endif

#define WIFI_SSID "Casa Amarilla"
#define WIFI_PASSWORD "mariposa15"
#define RECONNECT_INTERVAL_MS 30000
#define RECV_PIN 15

// COMMUNICATION CODES
#define NEW_CLIENT_CODE "new::client"
#define APPLY_CMD_CODE "use::cmd"

// DEVICE INFORMATION
#define DEVICE_NAME "IR_BLASTER_TEST"
#define DEVICE_LOCATION "habitacion 1"
#define DEVICE_DESCRIPTION "Controlador del aire acondicionado"
#define DEVICE_DEFAULT_CMD 0

int currentMillis = 0;
int previousMillis = 0;

WebSocketsClient webSocket;

void CheckNetworkReconnect();                                               // checks if the LAN it's disconnected and attempts to reconnect                                                     // sends an HTTP request to the master website (NO REQUIERED NOW). TODO: remove this
void SendConectionInfoToMaster();                                           // sends the information of this device to the host
void handleWebSocketEvent(WStype_t type, uint8_t* payload, size_t length);  // parses the information recieved over the web socket with the master

void setup() {
  Serial.begin(115200);
  while (!Serial)
    ;  // Wait for Serial to become available. Is optimized away for some cores.

  Serial.println("I'm the slave!");

  // IR Sender Stuff---------------------------------
  // Just to know which program is running on my Arduino
  Serial.println(F("START " __FILE__ " from " __DATE__ "\r\nUsing library version " VERSION_IRREMOTE));
  Serial.print(F("Send IR signals at pin "));
  Serial.println(IR_SEND_PIN);
  /*
     * The IR library setup. That's all!
     */
  IrSender.begin();      // Start with IR_SEND_PIN -which is defined in PinDefinitionsAndMore.h- as send pin and enable feedback LED at default feedback LED pin
  disableLEDFeedback();  // Disable feedback LED at default feedback LED pin
  // -----------------------------------------------

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
}

void loop() {

  webSocket.loop();

  CheckNetworkReconnect();
}

void CheckNetworkReconnect() {
  currentMillis = millis();
  if ((WiFi.status() != WL_CONNECTED) && (currentMillis - previousMillis >= RECONNECT_INTERVAL_MS)) {
    Serial.print(millis());
    Serial.println("Reconnecting to WiFi...");
    WiFi.disconnect();
    WiFi.reconnect();
    previousMillis = currentMillis;
  }
}

void handleWebSocketEvent(WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected");
      break;
    case WStype_CONNECTED:
      Serial.println("WebSocket Connected to Server");
      SendConectionInfoToMaster();
      break;
    case WStype_TEXT:
      {
        Serial.printf("Received message: %s\n", payload);
        String data = String((char*)payload);
        if (data.indexOf(APPLY_CMD_CODE) >= 0) {
          Serial.println("Applying new command! ");
          // IrSender.sendNEC(0xEF00, 0x3, 1);
          // delay(2000);
          // IrSender.sendNEC(0xEF00, 0x2, 1);

          int firstComma = data.indexOf(',');
          String code = data.substring(0, firstComma);
          String cmd_number = data.substring(firstComma + 1);

          int parsed_cmd_number = cmd_number.toInt();
          Serial.printf("SLAVE GOT CMD NUMBER: %d\n", parsed_cmd_number);

          switch (parsed_cmd_number) {
            case 0:  // prender LEDs
              Serial.println("Prendiendo LEDs");
              IrSender.sendNEC(0xEF00, 0x3, 1);
              break;
            case 1:  // apagar LEDs
              Serial.println("Apagando LEDs");
              IrSender.sendNEC(0xEF00, 0x2, 1);
              break;
            default:
              Serial.println("ERROR: no se pudo encontrar el comando enviado! " + cmd_number);
              break;
          }
        }
        break;
      }
    default:
      Serial.println("unhandled event");
      break;
  }
}

void SendConectionInfoToMaster() {
  if (webSocket.isConnected()) {
    // TODO: this should be defined at the TOP, and for every device think how to do it better
    String name = String(DEVICE_NAME);
    String location = String(DEVICE_LOCATION);
    String description = String(DEVICE_DESCRIPTION);
    String message = String(NEW_CLIENT_CODE) + "," + name + "," + location + "," + description + "\n";
    webSocket.sendTXT(message);
    Serial.println("Sent: " + message);
  } else {
    Serial.println("ERROR: WebSocket not connected!");
  }
}