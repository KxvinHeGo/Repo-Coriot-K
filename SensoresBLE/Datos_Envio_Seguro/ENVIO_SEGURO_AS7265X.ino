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

  Serial.println("Iniciando Coriot Connect...");

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
      if (sensorCharacteristic.written()) {
        String comandoRecibido = sensorCharacteristic.value();
        comandoRecibido.trim();

        Serial.print("Comando recibido: ");
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

  // Configuración de Ganancia
  switch (gananciaNum) {
    case 1:  sensor.setGain(AS7265X_GAIN_1X); break;
    case 3:  sensor.setGain(AS7265X_GAIN_37X); break;
    case 16: sensor.setGain(AS7265X_GAIN_16X); break;
    case 64: sensor.setGain(AS7265X_GAIN_64X); break;
    default: sensor.setGain(AS7265X_GAIN_16X); break;
  }

  // Configuración de Integración
  if (tiempoIntegracion < 1) tiempoIntegracion = 1;
  if (tiempoIntegracion > 255) tiempoIntegracion = 255;
  sensor.setIntegrationCycles(tiempoIntegracion);

  if (modo == "WRAW" || modo == "WCALIBRATED") {
    sensor.takeMeasurementsWithBulb(); 
  } else {
    sensor.takeMeasurements();
  }

  uint8_t tempIR  = sensor.getTemperature(0);
  uint8_t tempVIS = sensor.getTemperature(1);
  uint8_t tempUV  = sensor.getTemperature(2);

  String json = "{\"id\":\"" NOMBRE_DISPOSITIVO "\",\"modo\":\"" + modo + "\",";

  if (modo == "RAW" || modo == "WRAW") {
    json += "\"A\":" + String(sensor.getA()) + ",\"B\":" + String(sensor.getB()) + ",\"C\":" + String(sensor.getC()) + ",";
    json += "\"D\":" + String(sensor.getD()) + ",\"E\":" + String(sensor.getE()) + ",\"F\":" + String(sensor.getF()) + ",";
    json += "\"G\":" + String(sensor.getG()) + ",\"H\":" + String(sensor.getH()) + ",\"I\":" + String(sensor.getI()) + ",";
    json += "\"J\":" + String(sensor.getJ()) + ",\"K\":" + String(sensor.getK()) + ",\"L\":" + String(sensor.getL()) + ",";
    json += "\"R\":" + String(sensor.getR()) + ",\"S\":" + String(sensor.getS()) + ",\"T\":" + String(sensor.getT()) + ",";
    json += "\"U\":" + String(sensor.getU()) + ",\"V\":" + String(sensor.getV()) + ",\"W\":" + String(sensor.getW()) + ",";
  } else {
    json += "\"A\":" + String(sensor.getCalibratedA(), 2) + ",\"B\":" + String(sensor.getCalibratedB(), 2) + ",";
    json += "\"C\":" + String(sensor.getCalibratedC(), 2) + ",\"D\":" + String(sensor.getCalibratedD(), 2) + ",";
    json += "\"E\":" + String(sensor.getCalibratedE(), 2) + ",\"F\":" + String(sensor.getCalibratedF(), 2) + ",";
    json += "\"G\":" + String(sensor.getCalibratedG(), 2) + ",\"H\":" + String(sensor.getCalibratedH(), 2) + ",";
    json += "\"I\":" + String(sensor.getCalibratedI(), 2) + ",\"J\":" + String(sensor.getCalibratedJ(), 2) + ",";
    json += "\"K\":" + String(sensor.getCalibratedK(), 2) + ",\"L\":" + String(sensor.getCalibratedL(), 2) + ",";
    json += "\"R\":" + String(sensor.getCalibratedR(), 2) + ",\"S\":" + String(sensor.getCalibratedS(), 2) + ",";
    json += "\"T\":" + String(sensor.getCalibratedT(), 2) + ",\"U\":" + String(sensor.getCalibratedU(), 2) + ",";
    json += "\"V\":" + String(sensor.getCalibratedV(), 2) + ",\"W\":" + String(sensor.getCalibratedW(), 2) + ",";
  }

  json += "\"tUV\":" + String(tempUV) + ",\"tVIS\":" + String(tempVIS) + ",\"tIR\":" + String(tempIR) + "}";

  Serial.println("--------------------------------");
  Serial.print("Enviando JSON (");
  Serial.print(json.length());
  Serial.println(" bytes):");
  Serial.println(json);

  // Envío fragmentado para estabilidad
  enviarJsonPorFragmentos(json);
}

void enviarJsonPorFragmentos(String mensaje) {
  int longitud = mensaje.length();
  int tamanoFragmento = 60;
  
  for (int i = 0; i < longitud; i += tamanoFragmento) {
    String fragmento = mensaje.substring(i, min(i + tamanoFragmento, longitud));
    sensorCharacteristic.writeValue(fragmento);
    delay(35); // Necesario para el correcto procesamiento de los datos
  }
}