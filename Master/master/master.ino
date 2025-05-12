#include <WiFi.h>
#include <WebServer.h>
#include <WebSocketsServer.h>
#include <EEPROM.h>
#include <ESPmDNS.h>

#include "./index.h"
#include "./config_page.h"

// If you want to have Serial output enable DEBUG
#define DEBUG

#define SSID_BUFFER_LENGTH 32
#define PASSWD_BUFFER_LENGTH 64

typedef struct WifiCrendentials
{
  char ssid[32];
  char password[64];
};

#define EEPROM_SIZE sizeof(WifiCrendentials)
#define EEPROM_ADDR 0

// #define WIFI_SSID "ESP32_AP_TEST"
// #define WIFI_PASSWORD "AP123456"
// #define WIFI_SSID "Casa Amarilla"
// #define WIFI_PASSWORD "mariposa15"
#define SOFT_AP_SSID "DOMOTICA"
#define SOFT_AP_PASSWORD "123456789"
#define MAX_CLIENTS_ALLOWED 20
#define HTTP_ROUTE_DEVICES "/dispositivos"

#define MAX_WIFI_CONNECTION_TRIES 20

#define MDNS_DEVICE_ALIAS "domotica"

#define NEW_CLIENT_CODE "new::client"
#define REQUEST_INFO_CODE "req::info"
#define REQUEST_DEVICES_CODE "get::devices"
#define REMOVED_DEVICE_CODE "removed::client"
#define APPLY_CMD_CODE "use::cmd"
#define UPDATE_CREDENTIALS_CMD "set::lan"
#define CREDENTIALS_UPDATED_ACK "cred::success"
#define SLAVE_GET_WIFI_CREDS "req::creds"

// the ping should be at least twice the timeout
// so at least there are two ping no?
#define CLIENT_TIMEOUT_MS 5000
#define PING_CLIENTS_EVERY_MS 2000

typedef struct ClientItem
{
  int socketID;
  String name;
  String location;
  String description;
  unsigned long lastPing;
};
ClientItem connectedClients[MAX_CLIENTS_ALLOWED];
uint8_t total_clients_connected = 0;

WebServer server(80);
WebSocketsServer webSocket = WebSocketsServer(81);

uint8_t wifiConnectionTries = 0;
int lastClientConnected = -1; // maybe don't use this?? instead change the logic??

/**
 * SOFT_AP       -> Hosts a website to configure the device
 * NORMAL_HOST   -> It's the normal mode, that connects to the LAN
 */
typedef enum AP_MODE
{
  SOFT_AP,
  NORMAL_HOST,
  NONE
};
AP_MODE shouldChangeMasterMode = SOFT_AP;
AP_MODE currentMasterMode = NONE;

String stringifyClientItem(ClientItem client);
ClientItem getClientWithID(int id);
void initEmptyClients();
void pushNewClient(ClientItem client);
void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length);
void printClientItem(ClientItem item);
void checkConnectedClients();
void handleNetworkInitialization();                                      // This handle the connection to WiFi and the host of the website and all network related.
void saveWiFiCredentialsToFlash(const char *ssid, const char *password); // Save to non volatile memory (Flash) the SSID and PASSWORD
bool loadWiFiCredentials(char *ssid, char *password);                    // Get SSID and PASSWORD from Flash, if they exist returns true

void setup()
{
#ifdef DEBUG
  Serial.begin(115200);
  Serial.println("I'm the master!");
#endif

  handleNetworkInitialization();

  printf("Setup complete!");
}

void loop()
{
  webSocket.loop();
  server.handleClient();
  checkConnectedClients();
}

String stringifyClientItem(ClientItem item)
{
  String str = String(item.socketID);
  str += ", " + item.name;
  str += ", " + item.description;
  str += ", " + item.location;
  str += "\n";
  return str;
}

ClientItem getClientWithID(int id)
{
  for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++)
  {
    if (connectedClients[i].socketID == id)
    {
      return connectedClients[i];
    }
  }
  return {
      .socketID = -1,
      .name = "",
      .location = "",
      .description = "",
      .lastPing = 0,
  };
}

void initEmptyClients()
{
  for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++)
  {
    connectedClients[i] = {
        .socketID = -1,
        .name = "",
        .location = "",
        .description = "",
        .lastPing = 0,
    };
  }
}

// TODO: maybe return -1 if failed? aka no more space for clients
void pushNewClient(ClientItem client)
{
  if (client.socketID != -1)
  {
    for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++)
    {
      if (connectedClients[i].socketID == -1)
      {
        connectedClients[i] = client;
        lastClientConnected = i;
#ifdef DEBUG
        Serial.println("Pushed new client!");
#endif
        printClientItem(client);
        total_clients_connected++;
        return;
      }
    }
  }
}

// Prints to the Serial output the stringify version of an ClientItem
void printClientItem(ClientItem item)
{
#ifdef DEBUG
  Serial.println("ClientItem:");
  Serial.println("\t socketID: " + String(item.socketID));
  Serial.println("\t name: " + String(item.name));
  Serial.println("\t location: " + String(item.location));
  Serial.println("\t description: " + String(item.description));
  Serial.println("");
#endif
}

void webSocketEvent(uint8_t num, WStype_t type, uint8_t *payload, size_t length)
{
  switch (type)
  {
  case WStype_DISCONNECTED:
#ifdef DEBUG
    Serial.println("WebSocket Disconnected, " + String(num));
#endif
    webSocket.broadcastTXT(String(REMOVED_DEVICE_CODE) + "," + String(num));
    for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++)
    {
      if (connectedClients[i].socketID == num)
      {
#ifdef DEBUG
        Serial.println("Removing client from list with index " + String(i));
#endif
        connectedClients[i] = {
            .socketID = -1,
            .name = "",
            .location = "",
            .description = "",
            .lastPing = 0,
        };
      }
    }
    break;
  case WStype_CONNECTED:
#ifdef DEBUG
    Serial.println("WebSocket Connected to Server, " + String(num));
#endif
    break;
  case WStype_TEXT:
  {
    // Handle incoming messages from clients
#ifdef DEBUG
    Serial.printf("[%u] Received: %s\n", num, payload);
#endif

    String data = String((char *)payload);
    if (data.indexOf(NEW_CLIENT_CODE) >= 0)
    {
#ifdef DEBUG
      Serial.println("GOT: " + String(NEW_CLIENT_CODE));
#endif

      for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++)
      {
        if (connectedClients[i].socketID == num)
        {
          printf("The client was already added!");
          return;
        }
      }

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
          .lastPing = millis(),
      };
      pushNewClient(item);
#ifdef DEBUG
      Serial.println("Saved new client with ID " + num);
#endif
      if (lastClientConnected != -1)
      {
        webSocket.broadcastTXT("new::client," + String(connectedClients[lastClientConnected].socketID) + "," + connectedClients[lastClientConnected].name + "," + connectedClients[lastClientConnected].location + "," + connectedClients[lastClientConnected].description + "\n");
      }
    }
    else if (data.indexOf(REQUEST_INFO_CODE) >= 0)
    {
#ifdef DEBUG
      Serial.println("GOT: " + String(REQUEST_INFO_CODE));
#endif
      String str = stringifyClientItem(getClientWithID(num));
      webSocket.sendTXT(num, str);
    }
    else if (data.indexOf(REQUEST_DEVICES_CODE) >= 0)
    {
#ifdef DEBUG
      Serial.println("GOT: " + String(REQUEST_DEVICES_CODE));
#endif
      for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++)
      {
        if (connectedClients[i].socketID != -1)
        {
          // Serial.println(stringifyClientItem(connectedClients[i]));
          webSocket.sendTXT(num, String(NEW_CLIENT_CODE) + "," + String(connectedClients[i].socketID) + "," + connectedClients[i].name + "," + connectedClients[i].location + "," + connectedClients[i].description + "\n");
        }
      }
    }
    else if (data.indexOf(APPLY_CMD_CODE) >= 0)
    {
      // Expects that the command for the device has the following format:
      // APPLY_CMD_CODE,socketID,CMD_NUMBER
      // where CMD_NUMBER it's a number from 0-1 for the time being
#ifdef DEBUG
      Serial.println("GOT: " + String(APPLY_CMD_CODE));
      Serial.printf("FullString '%s'\n", data);
#endif
      int firstComma = data.indexOf(',');
      int secondComma = data.indexOf(',', firstComma + 1);
      String code = data.substring(0, firstComma);
      String id = data.substring(firstComma + 1, secondComma);
      String cmd_num = data.substring(secondComma + 1);
#ifdef DEBUG
      Serial.println("MASTER RECIEVED CMD: \n\t" + id + "\n\t" + cmd_num);
#endif
      String sending_cmd = String(APPLY_CMD_CODE) + "," + cmd_num + "\n";
#ifdef DEBUG
      Serial.println("Sending cmd: " + sending_cmd);
#endif
      webSocket.sendTXT(id.toInt(), sending_cmd);
    }
    else if (data.indexOf(UPDATE_CREDENTIALS_CMD) >= 0)
    {
      // This is when LAN information has to be set to
      // connect to a different network
      // new: SSID and PASSWORD
      // it's expected to receive the information in the following way:
      // "set::lan;;SSID;;PASSWORD"
#ifdef DEBUG
      Serial.println("GOT: " + data);
#endif

      // Find the positions of the delimiters
      int firstDelimiter = data.indexOf(";;");
      int secondDelimiter = data.indexOf(";;", firstDelimiter + 2);

      // Extract SSID (between first ;; and second ;;)
      String ssid = data.substring(firstDelimiter + 2, secondDelimiter);

      // Extract Password (after second ;;)
      String password = data.substring(secondDelimiter + 2);

      // Convert to char arrays
      char newSSID[SSID_BUFFER_LENGTH] = "";
      char newPasswd[PASSWD_BUFFER_LENGTH] = "";
      ssid.toCharArray(newSSID, sizeof(newSSID));
      password.toCharArray(newPasswd, sizeof(newPasswd));

#ifdef DEBUG
      Serial.println("SSID: " + String(newSSID));
      Serial.println("Password: " + String(newPasswd));
#endif
      saveWiFiCredentialsToFlash(newSSID, newPasswd);
      handleNetworkInitialization();

      webSocket.sendTXT(num, CREDENTIALS_UPDATED_ACK);
    }
    else if (data.indexOf(SLAVE_GET_WIFI_CREDS) >= 0)
    {
      // This handles the "handshaking" when there's a new
      // SSID and PASSWD for the LAN, so that the slaves
      // get the new information for the LAN
      char ssid[SSID_BUFFER_LENGTH] = "";
      char passwd[PASSWD_BUFFER_LENGTH] = "";
      if (loadWiFiCredentials(ssid, passwd))
      {
#ifdef DEBUG
        Serial.printf("Sending new credentials to slave (%d)\n", num);
#endif
        webSocket.sendTXT(num, String(SLAVE_GET_WIFI_CREDS) + ";;" + String(ssid) + ";;" + String(passwd));
      }
    }
  }
  break;
  case WStype_PING:
  {
#ifdef DEBUG
    Serial.println("---> Pinged back: " + num);
#endif
    // ClientItem client = getClientWithID(num);
    // Serial.printf("Loading: %ld\n", millis());
    // client.lastPing = millis();
    for (int i = 0; i < MAX_CLIENTS_ALLOWED; i++)
    {
      if (connectedClients[i].socketID == num)
      {
#ifdef DEBUG
        Serial.println("Updated lastPing");
#endif
        connectedClients[i].lastPing = millis();
        break;
      }
    }
    break;
  }
  case WStype_PONG:
    break;
  default:
#ifdef DEBUG
    Serial.print("unhandled event, ");
    Serial.println(type);
#endif
    break;
  }
}

volatile unsigned long currentTime;
volatile unsigned long lastPingTime = 0;
void checkConnectedClients()
{
  int i;
  currentTime = millis();
  if (currentTime - lastPingTime >= PING_CLIENTS_EVERY_MS)
  {
    for (i = 0; i < MAX_CLIENTS_ALLOWED; i++)
    {
      if (connectedClients[i].socketID != -1)
      {
        webSocket.sendPing(connectedClients[i].socketID);
#ifdef DEBUG
        Serial.printf("Pinged client with ID %d \n", connectedClients[i].socketID);
#endif
      }
    }
    lastPingTime = currentTime;
  }
  for (i = 0; i < MAX_CLIENTS_ALLOWED; i++)
  {
    if (connectedClients[i].socketID != -1 && currentTime - connectedClients[i].lastPing > CLIENT_TIMEOUT_MS)
    {
#ifdef DEBUG
      Serial.printf("diff: %ld\n", currentTime - connectedClients[i].lastPing);
#endif
      // Client has timed out, so remove it
#ifdef DEBUG
      Serial.println("Client timed out: " + String(connectedClients[i].socketID));
#endif
      webSocket.broadcastTXT(String(REMOVED_DEVICE_CODE) + "," + String(connectedClients[i].socketID));
      connectedClients[i] = {.socketID = -1}; // Clear client info
      total_clients_connected--;
    }
  }
}

void handleNetworkInitialization()
{
  char wifiSSID[32];
  char wifiPasswd[64];

  // Get the SSID and PASSWORD from the Flash memory
  if (loadWiFiCredentials(wifiSSID, wifiPasswd))
  {
    WiFi.begin(wifiSSID, wifiPasswd);
    while (WiFi.status() != WL_CONNECTED)
    {
      delay(1000);
#ifdef DEBUG
      Serial.println("Connecting to WiFi...");
#endif
      if (wifiConnectionTries >= MAX_WIFI_CONNECTION_TRIES)
        break;
      wifiConnectionTries++;
    }
    wifiConnectionTries = 0;
  }

  initEmptyClients();

  WiFi.mode(WIFI_MODE_APSTA);

  // Configure AP subnet (optional)
  WiFi.softAPConfig(IPAddress(192, 168, 5, 1), IPAddress(192, 168, 5, 1), IPAddress(255, 255, 255, 0));
  WiFi.softAP(SOFT_AP_SSID, SOFT_AP_PASSWORD);

  webSocket.begin();
  webSocket.onEvent(webSocketEvent);

  server.on("/", HTTP_GET, []()
            { server.send(200, "text/html", configPage); });

  server.on(HTTP_ROUTE_DEVICES, HTTP_GET, []()
            { server.send(200, "text/html", htmlPage); });

  server.begin();

  // allow the device to be discoverable in the LAN
  // TODO: make the MDNS ALIAS something unique. Maybe have a macro for it??
  if (MDNS.begin(MDNS_DEVICE_ALIAS))
  { // Set a unique hostname
#ifdef DEBUG
    Serial.println("mDNS responder started");
#endif
    MDNS.addService("http", "tcp", 80); // Advertise HTTP service on port 80
  }
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

  strncpy(ssid, creds.ssid, 32);
  strncpy(password, creds.password, 64);

#ifdef DEBUG
  Serial.println("Loaded WiFi credentials:");
  Serial.println("SSID: " + String(ssid));
  Serial.println("Password: " + String(password));
#endif

  return true;
}