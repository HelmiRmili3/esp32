
#include <esp_now.h>
#include <WiFi.h>
#include <Wire.h>
#include <Adafruit_INA219.h>

uint8_t broadcastAddress[] = {0x24, 0x62, 0xAB, 0xFE, 0x6B, 0xC8};// REPLACE WITH RECEIVER MAC ADDRESS

// Structure example to send data
// Must match the receiver structure
typedef struct message {
   float shuntVoltage;
   float busVoltage;
   float current_mA;
} Message;

Message sendedMessage;

Adafruit_INA219 ina219;

// callback when data is sent
void OnDataSent(const uint8_t *mac_addr, esp_now_send_status_t status) {
 	Serial.print(F("\r\n Master packet sent:\t"));
 	Serial.println(status == ESP_NOW_SEND_SUCCESS ? "Delivery Success" : "Delivery Fail");
}

void setup() {
 	// Init Serial Monitor
 	Serial.begin(115200);
  while (!Serial);

  if (!ina219.begin()) {
    Serial.println("Failed to find INA219 chip");
    while (1) { delay(10); }
  }

  Serial.println("Measuring voltage and current with INA219 ...");
 	// Set device as a Wi-Fi Station
  
 	WiFi.mode(WIFI_STA);
 	// Init ESP-NOW
 	if (esp_now_init() != ESP_OK) {
 			Serial.println(F("Error initializing ESP-NOW"));
 			return;
 	}
 	Serial.println(WiFi.macAddress());
 	
 	// Define Send function
 	esp_now_register_send_cb(OnDataSent);


 	// Register peer
  esp_now_peer_info_t peerInfo;
  memset(&peerInfo, 0, sizeof(peerInfo));
 	memcpy(peerInfo.peer_addr, broadcastAddress, 6);
 	peerInfo.channel = 0;
 	peerInfo.encrypt = false;

 	// Add peer
 	if (esp_now_add_peer(&peerInfo) != ESP_OK) {
 			Serial.println(F("Failed to add peer"));
 			return;
 	}
}

void loop() {
  
  // Read sensor data
  sendedMessage.shuntVoltage = ina219.getShuntVoltage_mV();
  sendedMessage.busVoltage = ina219.getBusVoltage_V();
  sendedMessage.current_mA = ina219.getCurrent_mA();

  // Print sensor readings
  Serial.print("Bus Voltage:   "); Serial.print(sendedMessage.busVoltage); Serial.println(" V");
  Serial.print("Shunt Voltage: "); Serial.print(sendedMessage.shuntVoltage); Serial.println(" mV");
  Serial.print("Current:       "); Serial.print(sendedMessage.current_mA); Serial.println(" mA");
  Serial.println();


 	// Send message via ESP-NOW
 	esp_err_t result = esp_now_send(broadcastAddress, (uint8_t *) &sendedMessage, sizeof(sendedMessage));

 	if (result == ESP_OK) {
 			Serial.println(F("Sent with success"));
 	}
 	else {
 			Serial.println(F("Error sending the data"));
 	}
 	delay(1000);
}


