/*
  🌐 ESP32 MQTT Project
  -------------------------------------
  Author: Arish S.
  Description:
  Connects ESP32 to a Wi-Fi network and a Mosquitto MQTT broker.
  Publishes dummy JSON data (distance, temperature) to "esp32/data".
  Subscribes to "esp32/command" for controlling onboard LED via MQTT messages.
*/

#include <WiFi.h>
#include <PubSubClient.h>

// 🟢 Wi-Fi credentials
const char* ssid = "xxxxxxx";       // Replace with your Wi-Fi SSID
const char* password = "xxxxxxx";           // Replace with your Wi-Fi password

// 🟢 MQTT Broker (Use your PC or Cloud broker IP/Domain)
const char* mqtt_server = "192.168.1.xxx";   // Replace with your broker IP (from 'ipconfig')

// 🧠 MQTT topics
const char* publishTopic = "esp32/data";
const char* subscribeTopic = "esp32/command";

// Create WiFi & MQTT clients
WiFiClient espClient;
PubSubClient client(espClient);

// Reconnect function for MQTT
void reconnect() {
  // Loop until reconnected
  while (!client.connected()) {
    Serial.print("Attempting MQTT connection...");
    String clientId = "ESP32Client-" + String(random(0xffff), HEX);
    
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected ✅");

      // Publish a message when connected
      client.publish("esp32/status", "ESP32 connected to MQTT broker successfully!");
      
      // Subscribe to topic for LED commands
      client.subscribe(subscribeTopic);
      Serial.println("Subscribed to topic: esp32/command");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" — retrying in 5 seconds...");
      delay(5000);
    }
  }
}

// Wi-Fi connection setup
void setup_wifi() {
  delay(10);
  Serial.println();
  Serial.print("Connecting to Wi-Fi: ");
  Serial.println(ssid);

  WiFi.begin(ssid, password);

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    delay(500);
    Serial.print(".");
  }

  Serial.println();
  Serial.println("✅ WiFi connected!");
  Serial.print("📶 IP Address: ");
  Serial.println(WiFi.localIP());
}

// Callback when a message arrives
void callback(char* topic, byte* message, unsigned int length) {
  Serial.println("📩 Message arrived!");
  Serial.print("Topic: ");
  Serial.println(topic);
  Serial.print("Message: ");

  String messageTemp;
  for (int i = 0; i < length; i++) {
    messageTemp += (char)message[i];
  }
  Serial.println(messageTemp);

  // LED Control Logic
  if (String(topic) == "esp32/command") {
    if (messageTemp == "1") {
      digitalWrite(2, HIGH);  // LED ON
      Serial.println("💡 LED turned ON");
    } else if (messageTemp == "0") {
      digitalWrite(2, LOW);   // LED OFF
      Serial.println("💤 LED turned OFF");
    }
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(2, OUTPUT); // Onboard LED (GPIO 2)

  setup_wifi(); // Connect to Wi-Fi

  client.setServer(mqtt_server, 1883);  // Connect to MQTT broker
  client.setCallback(callback);         // Set message callback
}

void loop() {
  // Ensure connection to MQTT broker
  if (!client.connected()) {
    reconnect();
  }
  client.loop();

  // Dummy JSON payload
  String payload = "{\"distance\":25.6, \"temperature\":31.4}";
  
  // Publish every 5 seconds
  client.publish(publishTopic, payload.c_str());
  Serial.println("📤 Published: " + payload);

  delay(5000);
}
