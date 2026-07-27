#include <Wire.h>
#include <ArduinoBLE.h>
#include "Adafruit_BME680.h"
#include "SparkFun_AS7265X.h"

#define SERVICE_UUID        "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"

#define NOMBRE_DISPOSITIVO "DV-2013"

Adafruit_BME680 bme;
AS7265X spectralSensor;

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
  delay(1000);

  Serial.println("================================");
  Serial.println("INICIANDO DV-2013");
  Serial.println("================================");

  Wire.begin();

  // ===== BME688 =====
  bool bmeDetectado = bme.begin(0x77) || bme.begin(0x76);

  if (!bmeDetectado) {
    Serial.println("ERROR: BME688 no detectado");
    while (1);
  }

  bme.setTemperatureOversampling(BME680_OS_8X);
  bme.setHumidityOversampling(BME680_OS_2X);
  bme.setPressureOversampling(BME680_OS_4X);
  bme.setIIRFilterSize(BME680_FILTER_SIZE_3);
  bme.setGasHeater(320, 150);

  Serial.println("BME688 detectado");

  // ===== AS7265X =====
  if (!spectralSensor.begin()) {
    Serial.println("ERROR: AS7265X no detectado");
    while (1);
  }

  Serial.println("AS7265X detectado");

  // ===== BLE =====
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

        // ===== BME688 =====
        if (!bme.performReading()) {
          Serial.println("Error leyendo BME688");
          continue;
        }

        // ===== AS7265X =====
        spectralSensor.takeMeasurementsWithBulb();

        String json =
          "{"
          "\"id\":\"" NOMBRE_DISPOSITIVO "\","

          "\"temp\":" + String(bme.temperature, 2) + ","
          "\"humi\":" + String(bme.humidity, 2) + ","
          "\"presion\":" + String(bme.pressure / 100.0, 2) + ","
          "\"gas\":" + String(bme.gas_resistance / 1000.0, 2) + ","

          "\"A\":" + String(spectralSensor.getCalibratedA(), 1) + ","
          "\"B\":" + String(spectralSensor.getCalibratedB(), 1) + ","
          "\"C\":" + String(spectralSensor.getCalibratedC(), 1) + ","
          "\"D\":" + String(spectralSensor.getCalibratedD(), 1) + ","
          "\"E\":" + String(spectralSensor.getCalibratedE(), 1) + ","
          "\"F\":" + String(spectralSensor.getCalibratedF(), 1) + ","
          "\"G\":" + String(spectralSensor.getCalibratedG(), 1) + ","
          "\"H\":" + String(spectralSensor.getCalibratedH(), 1) + ","
          "\"I\":" + String(spectralSensor.getCalibratedI(), 1) + ","
          "\"J\":" + String(spectralSensor.getCalibratedJ(), 1) + ","
          "\"K\":" + String(spectralSensor.getCalibratedK(), 1) + ","
          "\"L\":" + String(spectralSensor.getCalibratedL(), 1) + ","
          "\"R\":" + String(spectralSensor.getCalibratedR(), 1) + ","
          "\"S\":" + String(spectralSensor.getCalibratedS(), 1) + ","
          "\"T\":" + String(spectralSensor.getCalibratedT(), 1) + ","
          "\"U\":" + String(spectralSensor.getCalibratedU(), 1) + ","
          "\"V\":" + String(spectralSensor.getCalibratedV(), 1) + ","
          "\"W\":" + String(spectralSensor.getCalibratedW(), 1) +

          "}";

        sensorCharacteristic.writeValue(json);

        Serial.println("--------------------------------");
        Serial.print("JSON enviado (");
        Serial.print(json.length());
        Serial.println(" bytes)");
        Serial.println(json);
      }

      BLE.poll();
      delay(10);
    }

    Serial.println("Cliente desconectado");

    BLE.advertise();
  }
}