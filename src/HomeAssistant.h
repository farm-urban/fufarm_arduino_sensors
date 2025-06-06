#include "config.h"

#ifndef HOME_ASSISTANT_H
#define HOME_ASSISTANT_H

#if USE_HOME_ASSISTANT

#include <ArduinoHA.h>
#include "ServiceDiscovery.h"
#include "sensors.h"

/**
 * This class is a wrapper for the Home Assistant logic.
 * It is where all the Home Assistant related code is located.
 */
class FuFarmHomeAssistant
{
public:
  /**
   * Creates a new instance of the FuFarmHomeAssistant class.
   * Please note that only one instance of the class can be initialized at the same time.
   *
   * @param client The Etherclient or WiFiClient that's going to be used for the network communication.
   */
  FuFarmHomeAssistant(Client &client);

  /**
   * Cleanup resources created and managed by the FuFarmHomeAssistant class.
   */
  ~FuFarmHomeAssistant();

  /**
   * Returns pointer to the unique ID. It can be nullptr if the device has no ID assigned.
   */
  inline const char *getUniqueId() const { return device.getUniqueId(); }

  /**
   * Sets unique ID of the device based on the given byte array.
   * Each byte is converted into a hex string representation, so the final length of the unique ID will be twice as given.
   *
   * @param uniqueId Bytes array that's going to be converted into the string.
   * @param length Number of bytes in the array.
   */
  void setUniqueDeviceId(const uint8_t *value, uint8_t length);

  /**
   * Sets parameters of the MQTT connection using the IP address and port.
   * Also sets device_class, units, expiry, and other properties of each sensor.
   * Connection to the broker will be done in the first loop cycle.
   * This class automatically reconnects to the broker if connection is lost.
   *
   * Authentication parameters (username, password) are pulled from the build flags.
   *
   * @param endpoint Endpoint of the MQTT broker containing Domain or IP address and Port.
   */
  void connect(const NetworkEndpoint *endpoint);

  /**
   * This method should be called periodically inside the main loop of the firmware.
   * It's safe to call this method in some interval (like 5ms).
   */
  void maintain();

  /**
   * Publishes MQTT messages for configured sensors.
   * If the connection has not been established, nothing is published.
   * In some cases, if a sensor value is the same as the previous one the MQTT message won't be published.
   * This means MQTT messages are not always produced when the update method is called.
   *
   * @param source All values for configured sensors.
   * @param force Forces to update the state without comparing it to a previous known state.
   */
  void update(FuFarmSensorsData *source, const bool force = true);

  /**
   * Returns the current state of the MQTT connection.
   * @return true if connected, false otherwise.
   */
  inline bool connected() { return mqtt.isConnected(); }

private:
  HADevice device;
  HAMqtt mqtt;

private:
#ifdef HAVE_WATER_LEVEL_STATE
  HABinarySensor waterLevel;
#endif
#ifdef HAVE_LIGHT
  HASensorNumber light;
#endif
#if defined(HAVE_DHT22) || defined(HAVE_AHT20)
  HASensorNumber temperature, humidity;
#endif
#ifdef HAVE_ENS160
  HASensorNumber aqi, tvoc, eco2;
#endif
#ifdef HAVE_FLOW
  HASensorNumber flow;
#endif
#ifdef HAVE_TEMP_WET
  HASensorNumber liquidtemp;
#endif
#ifdef HAVE_CO2
  HASensorNumber co2;
#endif
#ifdef HAVE_EC
  HASensorNumber ec;
#endif
#ifdef HAVE_PH
  HASensorNumber ph;
#endif
#ifdef HAVE_MOISTURE
  HASensorNumber moisture;
#endif
};

#endif // USE_HOME_ASSISTANT

#endif // HOME_ASSISTANT_H
