#include <esp_now.h>// https://github.com/espressif/esp-idf/blob/master/components/esp_wifi/include/esp_now.h
#include <WiFi.h>

// Structure example to receive data
// Must match the sender structure
typedef struct message {
   float shuntVoltage;
   float busVoltage;
   float current_mA;
} Message;

Message receivedMessage;


// callback when data is received
void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
 	memcpy(&receivedMessage, incomingData, sizeof(receivedMessage));
 	 Serial.println("Received data:");
    Serial.print("ShuntVoltage: ");
    Serial.println(receivedMessage.shuntVoltage);
    Serial.print("BusVoltage: ");
    Serial.println(receivedMessage.busVoltage);
    Serial.print("Current_mA: ");
    Serial.println(receivedMessage.current_mA);

}

void setup() {
 	// Initialize Serial Monitor
 	Serial.begin(115200);

 	// Set device as a Wi-Fi Station
 	WiFi.mode(WIFI_STA);

 	// Init ESP-NOW
 	if (esp_now_init() != ESP_OK) {
 			Serial.println("Error initializing ESP-NOW");
 			return;
 	}
 	Serial.print(F("Reciever initialized : "));
 	Serial.println(WiFi.macAddress());

 	// Define receive function
 	esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
}

