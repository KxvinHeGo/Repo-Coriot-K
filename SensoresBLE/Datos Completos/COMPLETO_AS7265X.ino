#include <Wire.h>
#include <ArduinoBLE.h>
#include "SparkFun_AS7265X.h"

#define SERVICE_UUID        "19B10000-E8F2-537E-4F6C-D104768A1214"
#define CHARACTERISTIC_UUID "19B10001-E8F2-537E-4F6C-D104768A1214"

#define NOMBRE_DISPOSITIVO "DV-2015"

AS7265X sensor;

BLEService sensorService(SERVICE_UUID);

// Característica BLE con capacidad de 512 bytes
BLEStringCharacteristic sensorCharacteristic(
  CHARACTERISTIC_UUID,
  BLERead | BLENotify | BLEWrite,
  512
);

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
  Serial.println("Esperando conexiones y comandos...");
}

void loop() {
  BLEDevice central = BLE.central();

  if (central) {
    Serial.print("Cliente conectado: ");
    Serial.println(central.address());

    while (central.connected()) {
      
      // Revisa si la aplicación en Kotlin escribió una orden en la característica BLE
      if (sensorCharacteristic.written()) {
        String comandoRecibido = sensorCharacteristic.value();
        comandoRecibido.trim();

        Serial.print("Comando recibido desde App: ");
        Serial.println(comandoRecibido);

        if (comandoRecibido.startsWith("MEDIR")) {
          procesarYEjecutarMedicion(comandoRecibido);
        }
      }

      BLE.poll();
      delay(10);
    }

    Serial.println("Cliente desconectado");
    BLE.advertise();
  }
}

void procesarYEjecutarMedicion(String comando) {
  // Extrae posiciones del delimitador ';'
  int p1 = comando.indexOf(';');
  int p2 = comando.indexOf(';', p1 + 1);
  int p3 = comando.indexOf(';', p2 + 1);

  if (p1 == -1 || p2 == -1 || p3 == -1) {
    Serial.println("ERROR: Formato de comando invalido");
    return;
  }

  String modo = comando.substring(p1 + 1, p2);
  int gananciaNum = comando.substring(p2 + 1, p3).toInt();
  int tiempoIntegracion = comando.substring(p3 + 1).toInt();

  // Configura la Ganancia en el sensor
  switch (gananciaNum) {
    case 1:  sensor.setGain(AS7265X_GAIN_1X); break;
    case 3:  sensor.setGain(AS7265X_GAIN_37X); break;
    case 16: sensor.setGain(AS7265X_GAIN_16X); break;
    case 64: sensor.setGain(AS7265X_GAIN_64X); break;
    default: sensor.setGain(AS7265X_GAIN_16X); break;
  }

  // Configura el Tiempo de Integración (1 a 255)
  if (tiempoIntegracion < 1) tiempoIntegracion = 1;
  if (tiempoIntegracion > 255) tiempoIntegracion = 255;
  sensor.setIntegrationCycles(tiempoIntegracion);

  // Toma la medición física
  if (modo == "WRAW" || modo == "WCALIBRATED") {
    sensor.takeMeasurementsWithBulb(); 
  } else {
    sensor.takeMeasurements();
  }

  //Temperatura en °C de los 3 chips
  uint8_t tempIR  = sensor.getTemperature(0); // AS72651 (NIR)
  uint8_t tempVIS = sensor.getTemperature(1); // AS72652 (Visible)
  uint8_t tempUV  = sensor.getTemperature(2); // AS72653 (UV)

  // Construye el objeto JSON
  String json = "{\"id\":\"" NOMBRE_DISPOSITIVO "\",\"modo\":\"" + modo + "\",";

  if (modo == "RAW" || modo == "WRAW") {
    json += "\"A\":" + String(sensor.getA()) + ",";
    json += "\"B\":" + String(sensor.getB()) + ",";
    json += "\"C\":" + String(sensor.getC()) + ",";
    json += "\"D\":" + String(sensor.getD()) + ",";
    json += "\"E\":" + String(sensor.getE()) + ",";
    json += "\"F\":" + String(sensor.getF()) + ",";
    json += "\"G\":" + String(sensor.getG()) + ",";
    json += "\"H\":" + String(sensor.getH()) + ",";
    json += "\"I\":" + String(sensor.getI()) + ",";
    json += "\"J\":" + String(sensor.getJ()) + ",";
    json += "\"K\":" + String(sensor.getK()) + ",";
    json += "\"L\":" + String(sensor.getL()) + ",";
    json += "\"R\":" + String(sensor.getR()) + ",";
    json += "\"S\":" + String(sensor.getS()) + ",";
    json += "\"T\":" + String(sensor.getT()) + ",";
    json += "\"U\":" + String(sensor.getU()) + ",";
    json += "\"V\":" + String(sensor.getV()) + ",";
    json += "\"W\":" + String(sensor.getW()) + ",";
  } else {
    json += "\"A\":" + String(sensor.getCalibratedA(), 2) + ",";
    json += "\"B\":" + String(sensor.getCalibratedB(), 2) + ",";
    json += "\"C\":" + String(sensor.getCalibratedC(), 2) + ",";
    json += "\"D\":" + String(sensor.getCalibratedD(), 2) + ",";
    json += "\"E\":" + String(sensor.getCalibratedE(), 2) + ",";
    json += "\"F\":" + String(sensor.getCalibratedF(), 2) + ",";
    json += "\"G\":" + String(sensor.getCalibratedG(), 2) + ",";
    json += "\"H\":" + String(sensor.getCalibratedH(), 2) + ",";
    json += "\"I\":" + String(sensor.getCalibratedI(), 2) + ",";
    json += "\"J\":" + String(sensor.getCalibratedJ(), 2) + ",";
    json += "\"K\":" + String(sensor.getCalibratedK(), 2) + ",";
    json += "\"L\":" + String(sensor.getCalibratedL(), 2) + ",";
    json += "\"R\":" + String(sensor.getCalibratedR(), 2) + ",";
    json += "\"S\":" + String(sensor.getCalibratedS(), 2) + ",";
    json += "\"T\":" + String(sensor.getCalibratedT(), 2) + ",";
    json += "\"U\":" + String(sensor.getCalibratedU(), 2) + ",";
    json += "\"V\":" + String(sensor.getCalibratedV(), 2) + ",";
    json += "\"W\":" + String(sensor.getCalibratedW(), 2) + ",";
  }

  // 3 temperaturas
  json += "\"tUV\":" + String(tempUV) + ",";
  json += "\"tVIS\":" + String(tempVIS) + ",";
  json += "\"tIR\":" + String(tempIR) + "}";

  // Notifica a la app por BLE
  sensorCharacteristic.writeValue(json);

  // Monitor Serial
  Serial.println("--------------------------------");
  Serial.print("Enviado (");
  Serial.print(json.length());
  Serial.println(" bytes):");
  Serial.println(json);
}