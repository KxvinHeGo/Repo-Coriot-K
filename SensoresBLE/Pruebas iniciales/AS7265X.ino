#include <Wire.h>
#include <ArduinoBLE.h>
#include "SparkFun_AS7265X.h"

#define SERVICE_UUID        "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"

#define NOMBRE_DISPOSITIVO "DV-2011"

AS7265X sensor;

BLEService sensorService(SERVICE_UUID);

BLEStringCharacteristic sensorCharacteristic(
  CHARACTERISTIC_UUID,
  BLERead | BLENotify,
  512
);

unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_MS = 1000;

void setup() {
  Serial.begin(115200);
  delay(500);

  Serial.println("Iniciando...");

  Wire.begin();

  if (!sensor.begin()) {
    Serial.println("ERROR: AS7265X no detectado");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("AS7265X detectado correctamente");

  if (!BLE.begin()) {
    Serial.println("Error iniciando BLE");
    while (1);
  }

  BLE.setLocalName(NOMBRE_DISPOSITIVO);
  BLE.setDeviceName(NOMBRE_DISPOSITIVO);

  BLE.setAdvertisedService(sensorService);

  sensorService.addCharacteristic(sensorCharacteristic);

  BLE.addService(sensorService);

  sensorCharacteristic.writeValue("{}");

  BLE.advertise();

  Serial.println("BLE iniciado correctamente");
  Serial.println("Esperando conexiones...");
}

void loop() {

  BLEDevice central = BLE.central();

  if (central) {

    Serial.print("Cliente conectado: ");
    Serial.println(central.address());

    while (central.connected()) {

      if (millis() - ultimoEnvio >= INTERVALO_MS) {

        ultimoEnvio = millis();

        sensor.takeMeasurementsWithBulb();

        float A = sensor.getCalibratedA();
        float B = sensor.getCalibratedB();
        float C = sensor.getCalibratedC();
        float D = sensor.getCalibratedD();
        float E = sensor.getCalibratedE();
        float F = sensor.getCalibratedF();
        float G = sensor.getCalibratedG();
        float H = sensor.getCalibratedH();
        float I = sensor.getCalibratedI();
        float J = sensor.getCalibratedJ();
        float K = sensor.getCalibratedK();
        float L = sensor.getCalibratedL();
        float R = sensor.getCalibratedR();
        float S = sensor.getCalibratedS();
        float T = sensor.getCalibratedT();
        float U = sensor.getCalibratedU();
        float V = sensor.getCalibratedV();
        float W = sensor.getCalibratedW();

        String json =
          "{"
          "\"id\":\"" NOMBRE_DISPOSITIVO "\","
          "\"A\":" + String(A, 1) + ","
          "\"B\":" + String(B, 1) + ","
          "\"C\":" + String(C, 1) + ","
          "\"D\":" + String(D, 1) + ","
          "\"E\":" + String(E, 1) + ","
          "\"F\":" + String(F, 1) + ","
          "\"G\":" + String(G, 1) + ","
          "\"H\":" + String(H, 1) + ","
          "\"I\":" + String(I, 1) + ","
          "\"J\":" + String(J, 1) + ","
          "\"K\":" + String(K, 1) + ","
          "\"L\":" + String(L, 1) + ","
          "\"R\":" + String(R, 1) + ","
          "\"S\":" + String(S, 1) + ","
          "\"T\":" + String(T, 1) + ","
          "\"U\":" + String(U, 1) + ","
          "\"V\":" + String(V, 1) + ","
          "\"W\":" + String(W, 1) +
          "}";

        sensorCharacteristic.writeValue(json);

        Serial.println("--------------------------------");
        Serial.print("Enviado (");
        Serial.print(json.length());
        Serial.println(" bytes):");
        Serial.println(json);
      }

      BLE.poll();
      delay(10);
    }

    Serial.println("Cliente desconectado");
    BLE.advertise();
  }
}