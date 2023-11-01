#include <WiFi.h>
#include <esp_now.h>
#include <Wire.h>

#define AO_PIN 3 // Digital gas sensor input pin
const int solarPanelOutput = 100; // Solar panel output in watts
const int electrolyzerMinPower = 50; // Min power for the electrolyzer
const int electrolyzerMaxPower = 200; // Max power for the electrolyzer
const int fuelCellMinPower = 50; // Min power for the fuel cell
const int fuelCellMaxPower = 200; // Max power for the fuel cell
const int maxHydrogenPressure = 30; // Max hydrogen pressure in the storage tank

int housePowerDemand = 100; // Initial house power demand in watts
int electrolyzerPower = 0; // Initial electrolyzer power consumption in watts
int fuelCellPower = 0; // Initial fuel cell power production in watts
int hydrogenStorage = 0; // Initial hydrogen storage in cubic meters
int hydrogenPressure = 20; // Initial hydrogen pressure in bars

typedef struct message {
   float shuntVoltage;
   float busVoltage;
   float current_mA;
} Message;

Message receivedMessagefromEsp32_0; // c8:c9:a3:d0:cf:88
Message receivedMessagefromEsp32_1; // 24:0a:c4:61:0a:fc

void OnDataRecv(const uint8_t * mac, const uint8_t *incomingData, int len) {
  if (mac[0] == 0xc8 && mac[1] == 0xc9 && mac[2] == 0xa3 && mac[3] == 0xd0 && mac[4] == 0xcf && mac[5] == 0x88) {
    memcpy(&receivedMessagefromEsp32_0, incomingData, sizeof(receivedMessagefromEsp32_0));
    Serial.println("Received data from ESP32 with MAC address c8:c9:a3:d0:cf:88:");
    Serial.print("ShuntVoltage: ");
    Serial.println(receivedMessagefromEsp32_0.shuntVoltage);
    Serial.print("BusVoltage: ");
    Serial.println(receivedMessagefromEsp32_0.busVoltage);
    Serial.print("Current_mA: ");
    Serial.println(receivedMessagefromEsp32_0.current_mA);
  }
  if (mac[0] == 0x24 && mac[1] == 0x0a && mac[2] == 0xc4 && mac[3] == 0x61 && mac[4] == 0x0a && mac[5] == 0xfc) {
    memcpy(&receivedMessagefromEsp32_1, incomingData, sizeof(receivedMessagefromEsp32_1));
    Serial.println("Received data from ESP32 with MAC address 24:0a:c4:61:0a:fc:");
    Serial.print("ShuntVoltage: ");
    Serial.println(receivedMessagefromEsp32_1.shuntVoltage);
    Serial.print("BusVoltage: ");
    Serial.println(receivedMessagefromEsp32_1.busVoltage);
    Serial.print("Current_mA: ");
    Serial.println(receivedMessagefromEsp32_1.current_mA);
  }
}

void setup() {
  Serial.begin(115200);
  pinMode(AO_PIN, INPUT);

  WiFi.mode(WIFI_STA);

  if (esp_now_init() != ESP_OK) {
    Serial.println("Error initializing ESP-NOW");
    return;
  }
  Serial.print(F("Receiver initialized: "));
  Serial.println(WiFi.macAddress());

  esp_now_register_recv_cb(OnDataRecv);
}

void loop() {
  int gasValue = analogRead(AO_PIN);

  Serial.print("MQ2 sensor AO value: ");
  Serial.println(gasValue);

  int solarPowerGenerated = solarPanelOutput;

  if (solarPowerGenerated >= housePowerDemand) {
    housePowerDemand = 0;
    electrolyzerPower = solarPowerGenerated - housePowerDemand;
  } else {
    housePowerDemand -= solarPowerGenerated;
    electrolyzerPower = 0;
  }

  if (electrolyzerPower >= electrolyzerMinPower && electrolyzerPower <= electrolyzerMaxPower) {
    int hydrogenGenerated = electrolyzerPower / 100; // Example conversion factor
    hydrogenStorage += hydrogenGenerated;
    hydrogenPressure = maxHydrogenPressure;
  }

  if (housePowerDemand > 0) {
    if (hydrogenStorage > 0) {
      int powerSupplied = min(fuelCellMaxPower, hydrogenStorage);
      fuelCellPower = min(powerSupplied, housePowerDemand);
      hydrogenStorage -= fuelCellPower;
      hydrogenPressure = max(hydrogenPressure - 2, 0);
    }
  } else {
    fuelCellPower = 0;
  }

  //Serial.print("House Power Demand: ");
  //Serial.println(housePowerDemand);
  //Serial.print("Electrolyzer Power: ");
  //Serial.println(electrolyzerPower);
  //Serial.print("Fuel Cell Power: ");
  //Serial.println(fuelCellPower);
  //Serial.print("Hydrogen Storage: ");
  //Serial.println(hydrogenStorage);
  //Serial.print("Hydrogen Pressure: ");
  //Serial.println(hydrogenPressure);

  delay(2000); // 10-minute interval (10 minutes * 60 seconds * 1000 milliseconds)
}
