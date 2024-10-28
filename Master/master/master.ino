#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>

#include "./index.h"

#define WIFI_SSID "Casa Amarilla"
#define WIFI_PASSWORD "mariposa15"
#define MAX_CLIENTS_ALLOWED 3

#define NEW_CLIENT_CODE "new::client"
#define REQUEST_INFO_CODE "req::info"
#define REQUEST_DEVICES_CODE "get::devices"
#define REMOVED_DEVICE_CODE "removed::client"
#define APPLY_CMD_CODE "use::cmd"

typedef struct ClientItem {
  int socketID;
  String name;
  String location;
  String description;
};
ClientItem connectedClients[MAX_CLIENTS_ALLOWED] = {};

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

int lastClientConnected = -1;

String stringifyClientItem(ClientItem client);
ClientItem getClientWithID(int id);
void initEmptyClients();
void pushNewClient(ClientItem client);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length);
void printClientItem(ClientItem item);

void webSocketEvent(uint8_t num, WStype_t type, uint8_t* payload, size_t length) {
  switch (type) {
    case WStype_DISCONNECTED:
      Serial.println("WebSocket Disconnected, " + String(num));
      webSocket.broadcastTXT(String(REMOVED_DEVICE_CODE) + "," + String(num));
      for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++) {
        if (connectedClients[i].socketID != -1) {  // TODO: remove this, just for a simple debugging
          printClientItem(connectedClients[i]);
        }
        if (connectedClients[i].socketID == num) {
          Serial.println("Removing client from list with index " + String(i));
          connectedClients[i] = {
            .socketID = -1,
            .name = "",
            .location = "",
            .description = "",
          };
        }
      }
      break;
    case WStype_CONNECTED:
      Serial.println("WebSocket Connected to Server, " + String(num));
      break;
    case WStype_TEXT:
      {
        // Handle incoming messages from clients
        Serial.printf("[%u] Received: %s\n", num, payload);

        String data = String((char*)payload);
        if (data.indexOf(NEW_CLIENT_CODE) >= 0) {
          Serial.println("GOT: " + String(NEW_CLIENT_CODE));
          int firstComma = data.indexOf(',');
          int secondComma = data.indexOf(',', firstComma + 1);
          int thirdComma = data.indexOf(',', secondComma + 1);
          String code = data.substring(0, firstComma);
          String name = data.substring(firstComma + 1, secondComma);
          String location = data.substring(secondComma + 1, thirdComma);
          String description = data.substring(thirdComma + 1);
          ClientItem item = {
            .socketID = num,
            .name = name,
            .location = location,
            .description = description,
          };
          pushNewClient(item);
          if (lastClientConnected != -1) {
            webSocket.broadcastTXT("new::client," + String(connectedClients[lastClientConnected].socketID) + "," + connectedClients[lastClientConnected].name + "," + connectedClients[lastClientConnected].location + "," + connectedClients[lastClientConnected].description + "\n");
          }
        } else if (data.indexOf(REQUEST_INFO_CODE) >= 0) {
          Serial.println("GOT: " + String(REQUEST_INFO_CODE));
          String str = stringifyClientItem(getClientWithID(num));
          webSocket.sendTXT(num, str);
        } else if (data.indexOf(REQUEST_DEVICES_CODE) >= 0) {
          Serial.println("GOT: " + String(REQUEST_DEVICES_CODE));
          for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++) {
            if (connectedClients[i].socketID != -1) {
              //Serial.println(stringifyClientItem(connectedClients[i]));
              webSocket.sendTXT(num, String(NEW_CLIENT_CODE) + "," + String(connectedClients[i].socketID) + "," + connectedClients[i].name + "," + connectedClients[i].location + "," + connectedClients[i].description + "\n");
            }
          }
        } else if (data.indexOf(APPLY_CMD_CODE) >= 0) {
          // Expects that the command for the device has the following format:
          // APPLY_CMD_CODE,socketID,CMD_NUMBER
          // where CMD_NUMBER it's a number from 0-1 for the time being
          Serial.println("GOT: " + String(APPLY_CMD_CODE));
          Serial.printf("FullString '%s'\n", data);
          int firstComma = data.indexOf(',');
          int secondComma = data.indexOf(',', firstComma + 1);
          String code = data.substring(0, firstComma);
          String id = data.substring(firstComma + 1, secondComma);
          String cmd_num = data.substring(secondComma + 1);
          Serial.println("MASTER RECIEVED CMD: \n\t" + id + "\n\t" + cmd_num);
          String sending_cmd = String(APPLY_CMD_CODE) + "," + cmd_num + "\n";
          Serial.println("Sending cmd: " + sending_cmd);
          webSocket.sendTXT(id.toInt(), sending_cmd);
        }
      }
      break;
    case WStype_PONG:
      break;
    default:
      Serial.print("unhandled event, ");
      Serial.println(type);
      break;
  }
  // if (type == WStype_TEXT) {

  //   // webSocket.broadcastTXT("State updated!");
  // }
}

void setup() {
  Serial.begin(115200);
  Serial.println("I'm the master!");

  initEmptyClients();

  // Connect to WiFi
  WiFi.begin(WIFI_SSID, WIFI_PASSWORD);
  while (WiFi.status() != WL_CONNECTED) {
    delay(1000);
    Serial.println("Connecting to WiFi...");
  }
  Serial.println("Connected to WiFi");
  Serial.println(WiFi.localIP());

  // Serve the static HTML page
  server.on("/", HTTP_GET, []() {
    server.send(200, "text/html", htmlPage);
  });

  // Start WebSocket server and set event handler
  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  // Start the web server
  server.begin();
}

void loop() {
  webSocket.loop();
  server.handleClient();
}

String stringifyClientItem(ClientItem item) {
  String str = String(item.socketID);
  str += ", " + item.name;
  str += ", " + item.description;
  str += ", " + item.location;
  str += "\n";
}

ClientItem getClientWithID(int id) {
  for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++) {
    if (connectedClients[i].socketID == id) {
      return connectedClients[i];
    }
  }
  return {
    .socketID = -1,
    .name = "",
    .location = "",
    .description = "",
  };
}

void initEmptyClients() {
  for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++) {
    connectedClients[i] = {
      .socketID = -1,
      .name = "",
      .location = "",
      .description = "",
    };
  }
}

// TODO: maybe return -1 if failed? aka no more space for clients
void pushNewClient(ClientItem client) {
  if (client.socketID != -1) {
    for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++) {
      if (connectedClients[i].socketID == -1) {
        connectedClients[i] = client;
        lastClientConnected = i;
        Serial.println("Pushed new client!");
        printClientItem(client);
        return;
      }
    }
  }
}

// Prints to the Serial output the stringify version of an ClientItem
void printClientItem(ClientItem item) {
  Serial.println("ClientItem:");
  Serial.println("\t socketID: " + String(item.socketID));
  Serial.println("\t name: " + String(item.name));
  Serial.println("\t location: " + String(item.location));
  Serial.println("\t description: " + String(item.description));
  Serial.println("");
}