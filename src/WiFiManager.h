#include "config.h"

#ifndef WIFI_MANAGER_H
#define WIFI_MANAGER_H

#if HAVE_WIFI

#include <WiFi.h>

/**
 * This class is a wrapper for the WiFi logic.
 * It is where all the WiFi related code is located.
 */
class WiFiManager
{
public:
  /**
   * Creates a new instance of the WiFiManager class.
   * Please note that only one instance of the class can be initialized at the same time.
   */
  WiFiManager();

  /**
   * Cleanup resources created and managed by the WiFiManager class.
   */
  ~WiFiManager();

  /**
   * Scan for networks and connect to the one configured.
   */
  void begin();

  /**
   * This method should be called periodically inside the main loop of the firmware.
   * It's safe to call this method in some interval (like 5ms).
   */
  void maintain();

  /**
   * Get the current time from RTC if available or the WiFi module.
   *
   * @param info Pointer to a tm structure to store the current time.
   */
  bool getCurrentTime(struct tm *info);

private:
  uint8_t _status;

private:
#ifndef ARDUINO_ARCH_ESP32
  void printMacAddress(uint8_t mac[]);
#endif
#if !WIFI_SKIP_LIST_NETWORKS
  void listNetworks();
#endif
  void connect();
};

#endif // HAVE_WIFI

#endif // WIFI_MANAGER_H
