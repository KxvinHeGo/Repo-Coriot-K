#include <Wire.h>
#include <ArduinoBLE.h>
#include "Adafruit_BME680.h"

#define SERVICE_UUID        "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"

#define NOMBRE_DISPOSITIVO "DV-2012"

Adafruit_BME680 bme;

BLEService sensorService(SERVICE_UUID);

BLEStringCharacteristic sensorCharacteristic(
  CHARACTERISTIC_UUID,
  BLERead | BLENotify,
  256
);

bool deviceConnected = false;
unsigned long ultimoEnvio = 0;
const unsigned long INTERVALO_MS = 1000;

void setup() {
  Serial.begin(115200);
  delay(500);
  Serial.println("Iniciando...");

  Wire.begin();

  bool detectado = bme.begin(0x77) || bme.begin(0x76);

  if (!detectado) {
    Serial.println("ERROR: BME688 no detectado. Revisa conexión I2C/dirección.");
    while (1) {
      delay(1000);
    }
  }

  Serial.println("BME688 detectado correctamente");

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

  if (!BLE.begin()) {
    Serial.println("Error iniciando BLE");
    while (1);
  }

  BLE.setLocalName(NOMBRE_DISPOSITIVO);
  BLE.setDeviceName(NOMBRE_DISPOSITIVO);

  BLE.setAdvertisedService(sensorService);

  sensorService.addCharacteristic(sensorCharacteristic);

  BLE.addService(sensorService);

  sensorCharacteristic.writeValue("{\"status\":\"ONLINE\"}");

  BLE.advertise();

  Serial.println("================================");
  Serial.println("BLE iniciado correctamente");
  Serial.println("Esperando conexiones...");
  Serial.println("================================");
}

void loop() {

  BLEDevice central = BLE.central();

  if (central) {

    Serial.print("Cliente conectado: ");
    Serial.println(central.address());

    deviceConnected = true;

    while (central.connected()) {

      if (millis() - ultimoEnvio >= INTERVALO_MS) {

        ultimoEnvio = millis();

        if (!bme.performReading()) {
          Serial.println("Fallo al leer el BME688, se reintenta en el próximo ciclo");
          continue;
        }

        float temperatura = bme.temperature;
        float humedad     = bme.humidity;
        float presion     = bme.pressure / 100.0;
        float gas         = bme.gas_resistance / 1000.0;

        String json =
          "{"
          "\"id\":\"" NOMBRE_DISPOSITIVO "\","
          "\"temp\":" + String(temperatura, 2) + ","
          "\"humi\":" + String(humedad, 2) + ","
          "\"presion\":" + String(presion, 2) + ","
          "\"gas\":" + String(gas, 2) +
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

    deviceConnected = false;

    Serial.println("Cliente desconectado");

    BLE.advertise();
  }
}