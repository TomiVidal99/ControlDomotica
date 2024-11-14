#include <WiFi.h>

#define AP_SSID "ESP32_AP_TEST"
#define AP_PSSWD "AP123456"

WiFiServer server(80);

void setup() {
  Serial.begin(115200);

  // Configure the ESP32 as an Access Point
  WiFi.softAP(AP_SSID, AP_PSSWD);

  // Optional: Set the IP address of the AP
  IPAddress IP(192, 168, 4, 1);
  IPAddress NMask(255, 255, 255, 0);
  WiFi.softAPConfig(IP, IP, NMask);

  Serial.print("Access Point \"");
  Serial.print(AP_SSID);
  Serial.println("\" started");

  // Display the IP address
  Serial.print("IP address:\t");
  Serial.println(WiFi.softAPIP());
}

void loop() {
}
