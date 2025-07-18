#include "config.h"
#include "sensors.h"

#if HAVE_WIFI
#include "WiFiManager.h"
#endif

#if HAVE_NETWORK
#include "ServiceDiscovery.h"
#include "HomeAssistant.h"
#else
#include <ArduinoJson.h>
#endif

FuFarmSensors sensors;
FuFarmSensorsData sensorsData;
static boolean calibrationMode = false;

// Wifi control
#if HAVE_WIFI
WiFiManager wifiManager;
#if HOME_ASSISTANT_MQTT_TLS
#ifdef ARDUINO_ARCH_ESP32
#include <WiFiClientSecure.h>
WiFiClientSecure tcpClient;
#else
WiFiSSLClient tcpClient;
#endif
#else
WiFiClient tcpClient;
#endif
WiFiUDP udpClient;
#endif

// Network
#if HAVE_NETWORK
#if NETWORK_SERVICE_DISCOVERY
ServiceDiscovery discovery(udpClient);
#endif
FuFarmHomeAssistant ha(tcpClient);
#else
JsonDocument doc;
#endif // HAVE_NETWORK

void setup()
{
  Serial.begin(9600);

  // https://www.arduino.cc/reference/en/language/functions/analog-io/analogreference/
#if defined(ARDUINO_ARCH_ESP32)
  // no need to set the reference voltage on ESP32-S3 because it offers reads in millivolts
  analogReadResolution(12); // change to 12-bit resolution
#elif defined(ARDUINO_AVR_LEONARDO)
  analogReference(DEFAULT); // Set the default voltage of the reference voltage
#elif defined(ARDUINO_UNOR4_WIFI)
  analogReference(AR_DEFAULT); // AR_DEFAULT: 5V on the Uno R4 WiFi
  analogReadResolution(14);    // change to 14-bit resolution
#elif defined(ARDUINO_AVR_UNO_WIFI_REV2)
  analogReference(VDD); // VDD: Vdd of the ATmega4809. 5V on the Uno WiFi Rev2
#else // any other board we have not validated
#pragma message "⚠️ Unable to set analogue reference voltage. Board not supported."
#endif

  sensors.begin();

  // if calibration toggle is shorted then we are in calibration mode
#ifdef SUPPORTS_CALIBRATION
  pinMode(CALIBRATION_TOGGLE_PIN, INPUT_PULLUP);
  calibrationMode = digitalRead(CALIBRATION_TOGGLE_PIN) == 0;
  if (calibrationMode)
  {
    Serial.println(F("Calibration toggle has been shorted hence enter calibration mode. To exit, remove the short and reset."));
    Serial.println(F("EC and PH sensors take calibration commands via Serial Monitor/Terminal."));
    Serial.println(F("Available commands for PH sensor:"));
    Serial.println(F("enterph -> enter the calibration mode for PH"));
    Serial.println(F("calph   -> calibrate with the standard buffer solution, two buffer solutions(4.0 and 7.0) will be automatically recognized"));
    Serial.println(F("exitph  -> save the calibrated parameters and exit from calibration mode"));
    Serial.println(F("Available commands for EC sensor:"));
    Serial.println(F("enterec -> enter the calibration mode"));
    Serial.println(F("calec   -> calibrate with the standard buffer solution, two buffer solutions(1413us/cm and 12.88mS/cm) will be automatically recognized"));
    Serial.println(F("exitec  -> save the calibrated parameters and exit from calibration mode"));
    return; // no further initialization can happen when in calibration mode
  }
  else
  {
    Serial.println(F("Assuming sensors have been calibrated. To enter calibration mode, short the calibration toggle and reset."));
  }
#endif

#if HAVE_WIFI
  wifiManager.begin();
#if HOME_ASSISTANT_MQTT_TLS && defined(ARDUINO_ARCH_ESP32)
  tcpClient.setCACert(root_ca_certs);
#endif
#endif // HAVE_WIFI

#if HAVE_NETWORK
  ha.setUniqueDeviceId(wifiManager.macAddress(), MAC_ADDRESS_LENGTH);
  Serial.print(F("Home Assistant Unique Device ID: "));
  Serial.println(ha.getUniqueId());
#if NETWORK_SERVICE_DISCOVERY
  discovery.begin(wifiManager.localIP(), wifiManager.hostname(), wifiManager.macAddress());
  discovery.onHaEndpointUpdated([](NetworkEndpoint *endpoint) { ha.connect(endpoint); });
#else
  NetworkEndpoint haEndpoint = {
    .type = NetworkEndpointType::DNS,
    .ip = INADDR_NONE,
    .hostname = HOME_ASSISTANT_MQTT_HOST,
    .port = HOME_ASSISTANT_MQTT_PORT,
  };
  ha.connect(&haEndpoint);
#endif
#endif // HAVE_NETWORK
} // end setup

static uint32_t timepoint = millis();

void loop()
{
  if (calibrationMode)
  {
    sensors.calibration();
    return; // nothing else can happen when in calibration mode
  }

  if ((millis() - timepoint) >= SAMPLE_WINDOW_MILLIS)
  {
    timepoint = millis();
    sensors.read(&sensorsData);
#if HAVE_NETWORK
    ha.update(&sensorsData);
#else
    // populate json
#if defined(HAVE_DHT22) || defined(HAVE_AHT20)
    doc["tempair"] = sensorsData.temperature.air;
    doc["humidity"] = sensorsData.humidity;
#endif
#ifdef HAVE_ENS160
    doc["aqi"] = sensorsData.airQuality.index;
    doc["tvoc"] = sensorsData.airQuality.tvoc;
    doc["eco2"] = sensorsData.airQuality.eco2;
#endif
#ifdef HAVE_TEMP_WET
    doc["tempwet"] = sensorsData.temperature.wet;
#endif
#ifdef HAVE_CO2
    doc["co2"] = sensorsData.co2;
#endif
#ifdef HAVE_EC
    doc["ec"] = sensorsData.ec;
#endif
#ifdef HAVE_PH
    doc["ph"] = sensorsData.ph;
#endif
#ifdef HAVE_FLOW
    doc["flow"] = sensorsData.flow;
#endif
#ifdef HAVE_LIGHT
    doc["light"] = sensorsData.light;
#endif
#ifdef HAVE_MOISTURE
    doc["moisture"] = sensorsData.moisture;
#endif
#ifdef HAVE_WATER_LEVEL_STATE
    doc["water_level"] = sensorsData.waterLevelState;
#endif

    serializeJson(doc, Serial);
    Serial.println();
    Serial.flush();
#endif
  }

#if HAVE_WIFI
  wifiManager.maintain();
#endif // HAVE_WIFI
#if HAVE_NETWORK
#if NETWORK_SERVICE_DISCOVERY
  discovery.maintain();
#endif
  ha.maintain();
#endif // HAVE_NETWORK

  // This delay should be short so that the networking stuff is maintained correctly.
  // Network maintenance includes checking for WiFi connection, server connection, and sending PINGs.
  // Updating of sensor values happens over a longer delay by checking elapsed time above.
  delay(500);
}

void beforeReset()
{
  // TODO: save configuration and/or state so that we can resume after reboot (will be useful with auto dosing)
}
