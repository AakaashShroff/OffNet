#include <ESP8266WiFi.h>
#include <SPI.h>
#include <LoRa.h>

const char* password = "OffNet@2024";

WiFiServer wifiServer(80);

#define LORA_NSS  15
#define LORA_RST  16
#define LORA_DIO0  5

const String deviceID = "ESP1";

void setup() {
  Serial.begin(9600);
  Serial.println("Starting ESP8266...");

  String ssid = deviceID + "OffNet";

  WiFi.softAP(ssid.c_str(), password);
  Serial.println("Access Point Started");

  IPAddress IP = WiFi.softAPIP();
  Serial.print("AP IP address: ");
  Serial.println(IP);

  wifiServer.begin();
  Serial.println("Server started");

  LoRa.setPins(LORA_NSS, LORA_RST, LORA_DIO0);
  if (!LoRa.begin(433E6)) {
    Serial.println("Starting LoRa failed!");
    while (1);
  }
  Serial.println("LoRa Initialized");
}

void loop() {
  receiveLoRaMessage();

  WiFiClient client = wifiServer.available();
  if (client) {
    handleClient(client);
  }
}

void receiveLoRaMessage() {
  int packetSize = LoRa.parsePacket();
  if (packetSize) {
    Serial.print("Received packet: ");

    String receivedData = "";
    while (LoRa.available()) {
      receivedData += (char)LoRa.read();
    }
    Serial.print(receivedData);

    int firstColon = receivedData.indexOf(':');
    int secondColon = receivedData.indexOf(':', firstColon + 1);
    int thirdColon = receivedData.indexOf(':', secondColon + 1);
    int fourthColon = receivedData.indexOf(':', thirdColon + 1);

    String senderUser = receivedData.substring(0, firstColon);
    String senderESP = receivedData.substring(firstColon + 1, secondColon);
    String receiverUser = receivedData.substring(secondColon + 1, thirdColon);
    String receiverESP = receivedData.substring(thirdColon + 1, fourthColon);
    String message = receivedData.substring(fourthColon + 1);

    if (receiverESP == deviceID) {
      Serial.print(" from ");
      Serial.print(senderUser);
      Serial.print(" (");
      Serial.print(senderESP);
      Serial.print("): ");
      Serial.println(message);

      forwardMessageToPython(receiverUser, receivedData);
    } else {
      Serial.println(" - Packet not for this device.");
    }

    Serial.print(" with RSSI ");
    Serial.println(LoRa.packetRssi());
  }
}

void forwardMessageToPython(String receiverUser, String message) {
  String serverUrl = "http://192.168.4.2:8080/transmit?msg=" + message;

  WiFiClient client;
  if (!client.connect("192.168.4.2", 8080)) {
    Serial.println("Connection to Python server failed!");
    return;
  }

  client.print(String("GET ") + "/transmit?msg=" + message + " HTTP/1.1\r\n" +
               "Host: 192.168.4.2\r\n" +
               "Connection: close\r\n\r\n");

  while (client.connected() || client.available()) {
    if (client.available()) {
      String line = client.readStringUntil('\n');
      Serial.println(line);
    }
  }

  client.stop();
}

void handleClient(WiFiClient client) {
  Serial.println("New Client Connected.");

  String request = client.readStringUntil('\r');
  Serial.println("Full Request: " + request);

  int msgIndex = request.indexOf("/transmit?msg=");
  if (msgIndex != -1) {
    int start = msgIndex + String("/transmit?msg=").length();
    int end = request.indexOf(' ', start);
    if (end == -1) {
      end = request.length(); 
    }
    String message = request.substring(start, end);
    Serial.println("Message to transmit: " + message);

    int firstColon = message.indexOf(':');
    int secondColon = message.indexOf(':', firstColon + 1);
    int thirdColon = message.indexOf(':', secondColon + 1);
    int fourthColon = message.indexOf(':', thirdColon + 1);

    String senderUser = message.substring(0, firstColon);
    String senderESP = message.substring(firstColon + 1, secondColon);
    String receiverUser = message.substring(secondColon + 1, thirdColon);
    String receiverESP = message.substring(thirdColon + 1, fourthColon);
    String actualMessage = message.substring(fourthColon + 1);

    bool result = transmitLoRaMessage(senderUser + ":" + senderESP + ":" + receiverUser + ":" + receiverESP + ":" + actualMessage);

    String responseMessage;
    if (result) {
      responseMessage = "Transmitted";
      Serial.println("Message transmitted successfully via LoRa.");
    } else {
      responseMessage = "Error: Transmission failed";
      Serial.println("Failed to transmit message via LoRa.");
    }

    client.println("HTTP/1.1 200 OK");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println("Content-Length: " + String(responseMessage.length()));
    client.println();
    client.println(responseMessage);
  } else {
    String errorMessage = "Invalid request";
    client.println("HTTP/1.1 404 Not Found");
    client.println("Content-Type: text/plain");
    client.println("Connection: close");
    client.println("Content-Length: " + String(errorMessage.length()));
    client.println();
    client.println(errorMessage);
    Serial.println("Invalid request received.");
  }

  client.stop();
  Serial.println("Client Disconnected.");
}

bool transmitLoRaMessage(String message) {
  LoRa.beginPacket();
  LoRa.print(message);
  if (LoRa.endPacket()) {
    return true;
  } else {
    return false;
  }
}
