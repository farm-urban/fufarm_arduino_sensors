#include "WiFiManager.h"

#if HAVE_WIFI

#include "reboot.h"

#ifdef ARDUINO_ARCH_ESP32
#include "esp_eap_client.h"
#endif

WiFiManager::WiFiManager() : _status(WL_IDLE_STATUS)
{
}

WiFiManager::~WiFiManager()
{
}

void WiFiManager::begin()
{
#ifndef ARDUINO_ARCH_ESP32
  // check if the WiFi module is present
  if (WiFi.status() == WL_NO_MODULE)
  {
    Serial.println(F("Communication with WiFi module failed!"));
    while (true)
      ; // don't continue
  }

  // check firmware version of the module
  String fv = WiFi.firmwareVersion();
  if (fv < WIFI_FIRMWARE_LATEST_VERSION)
  {
    Serial.println(F("Please upgrade the firmware"));
  }
#endif

#ifdef ARDUINO_ARCH_ESP32
  // Set WiFi to station mode and disconnect from an AP if it was previously connected.
  WiFi.mode(WIFI_STA);
#endif

  WiFi.disconnect(); // Clear network stack https://forum.arduino.cc/t/mqtt-with-esp32-gives-timeout-exceeded-disconnecting/688723/5

#if !WIFI_SKIP_LIST_NETWORKS
  listNetworks();
#endif
  connect(true);
}

void WiFiManager::maintain()
{
  connect(false);
}

void WiFiManager::printMacAddress(uint8_t mac[])
{
  for (int i = 5; i >= 0; i--)
  {
    if (mac[i] < 16)
    {
      Serial.print(F("0"));
    }
    Serial.print(mac[i], HEX);
    if (i > 0)
    {
      Serial.print(F(":"));
    }
  }
}

#ifdef ARDUINO_ARCH_ESP32
const __FlashStringHelper *encryptionTypeToString(wifi_auth_mode_t mode)
{
  switch (mode)
  {
  case WIFI_AUTH_OPEN:
    return F("open");
  case WIFI_AUTH_WEP:
    return F("WEP");
  case WIFI_AUTH_WPA_PSK:
    return F("WPA");
  case WIFI_AUTH_WPA2_PSK:
    return F("WPA2");
  case WIFI_AUTH_WPA_WPA2_PSK:
    return F("WPA+WPA2");
  case WIFI_AUTH_WPA2_ENTERPRISE:
    return F("WPA2-EAP");
  case WIFI_AUTH_WPA3_PSK:
    return F("WPA3");
  case WIFI_AUTH_WPA2_WPA3_PSK:
    return F("WPA2+WPA3");
  case WIFI_AUTH_WAPI_PSK:
    return F("WAPI");
  default:
    return F("unknown");
  }
}
#else
const __FlashStringHelper *encryptionTypeToString(uint8_t type)
{
  // read the encryption type and print out the name:
  switch (type)
  {
  case ENC_TYPE_WEP:
    return F("WEP");
  case ENC_TYPE_TKIP:
    return F("WPA");
  case ENC_TYPE_CCMP:
    return F("WPA2");
  case ENC_TYPE_NONE:
    return F("None");
  case ENC_TYPE_AUTO:
    return F("Auto");
  case ENC_TYPE_UNKNOWN:
  default:
    return F("Unknown");
  }
}
#endif

#if !WIFI_SKIP_LIST_NETWORKS
void WiFiManager::listNetworks()
{
  Serial.println(F("** Scan Networks **"));
  int8_t count = WiFi.scanNetworks();
  if (count == -1)
  {
    Serial.println(F("Couldn't scan for WiFi networks"));
    return; // nothing more to do here
  }

  // 1) Print header
  Serial.println();
  Serial.println(F("Id  | SSID                             | Signal (dBm) | Ch | Encryption"));
  Serial.println(F("----+----------------------------------+--------------+----+------------"));

  // 2) For each network, build a fixed-width line
  for (int8_t i = 0; i < count; i++) {
    // Build into a buffer with fixed column widths:
    //   • Index:      width  3, right-aligned
    //   • SSID:       width 32, left-aligned (crop or pad with spaces if shorter)
    //   • Signal:     width  5, right-aligned (the "(dBm)" is in header)
    //   • Channel:    width  2, right-aligned
    //   • Encryption: width 10, left-aligned
    // 3 + 32 + 5 + spaces + 2 + 10 + (4 * 3 separators) + 1 EOF byte ≈ 64 – 72 bytes total
    char lineBuf[80];
    snprintf(
      lineBuf, sizeof(lineBuf),
      "%3d | %-32.32s | %5d        | %2d | %-10.10s",
      i,
#ifdef ARDUINO_ARCH_ESP32
      WiFi.SSID(i).c_str(), // Use c_str() to get a const char* from String
#else
      WiFi.SSID(i),
#endif
      WiFi.RSSI(i),
      WiFi.channel(i),
      // Use reinterpret_cast to convert from __FlashStringHelper* to const char*
      reinterpret_cast<const char*>(
        encryptionTypeToString(WiFi.encryptionType(i)))
    );
    Serial.println(lineBuf);
  }
  Serial.println();

  // Delete the scan result to free memory for code below.
#ifdef ARDUINO_ARCH_ESP32
  WiFi.scanDelete();
#endif
}
#endif

void WiFiManager::connect(bool initial)
{
  // if already connected, there is nothing more to do
  uint8_t status = WiFi.status();
  if (status == WL_CONNECTED)
  {
    return;
  }

  // detect disconnection
  if (_status == WL_CONNECTED && _status != status)
  {
    Serial.println(F("WiFi disconnected"));
  }

  Serial.print(F("Attempting to connect to WiFi SSID: "));
  Serial.println(WIFI_SSID);

#ifdef ARDUINO_ARCH_ESP32
#if defined(WIFI_ENTERPRISE_IDENTITY)
  esp_eap_client_set_identity((uint8_t *)WIFI_ENTERPRISE_IDENTITY, strlen(WIFI_ENTERPRISE_IDENTITY));
#endif
#if defined(WIFI_ENTERPRISE_USERNAME)
  esp_eap_client_set_username((uint8_t *)WIFI_ENTERPRISE_USERNAME, strlen(WIFI_ENTERPRISE_USERNAME));
#endif
#if defined(WIFI_ENTERPRISE_PASSWORD)
  esp_eap_client_set_password((uint8_t *)WIFI_ENTERPRISE_PASSWORD, strlen(WIFI_ENTERPRISE_PASSWORD));
#endif
#ifdef WIFI_ENTERPRISE_PASSWORD
  esp_wifi_sta_enterprise_enable();
#endif
#endif

#if defined(WIFI_ENTERPRISE_PASSWORD) && !defined(ARDUINO_ARCH_ESP32)
  status = WiFi.beginEnterprise(WIFI_SSID,
                                WIFI_ENTERPRISE_USERNAME,
                                WIFI_ENTERPRISE_PASSWORD,
                                WIFI_ENTERPRISE_IDENTITY,
                                WIFI_ENTERPRISE_CA);
#elif defined(WIFI_PASSPHRASE)
  status = WiFi.begin(WIFI_SSID, WIFI_PASSPHRASE);
#else
  status = WiFi.begin(WIFI_SSID);
#endif

  uint32_t started = millis();
  while (status != WL_CONNECTED)
  {
    // Timeout reached – perform a reset
    if ((millis() - started) > WIFI_CONNECTION_REBOOT_TIMEOUT_MILLIS) {
      Serial.println(F(" taken too long. Rebooting ...."));
      reboot();
    }

    delay(500);
    Serial.print(F("."));
    status = WiFi.status();
  }
  _status = WL_CONNECTED;

  Serial.println();
  Serial.println(F("WiFi connected successfully!"));
  Serial.print(F("SSID: "));
  Serial.println(WiFi.SSID());

  uint8_t mac[MAC_ADDRESS_LENGTH];
  WiFi.BSSID(mac);
  Serial.print(F("BSSID: "));
  printMacAddress(mac);
  Serial.println();

  Serial.print(F("Signal strength (RSSI): "));
  Serial.print(WiFi.RSSI());
  Serial.println(F(" dBm"));

  Serial.print(F("IP Address: "));
  Serial.println(WiFi.localIP());

  WiFi.macAddress(_macAddress);
  Serial.print(F("MAC address: "));
  printMacAddress(_macAddress);
  Serial.println();

  // For the first time, set the hostname using the mac address.
  // Change the hostname to a more useful name. E.g. a default value like "esp32s3-594E40" changes to "fufarm-594E40"
  // The WiFi stack needs to have been activated by scanning or connecting hence why this is done last. Otherwise just zeros.
  if (initial)
  {
    snprintf(_hostname, sizeof(_hostname), "fufarm-%02X%02X%02X", _macAddress[3], _macAddress[4], _macAddress[5]);
    WiFi.setHostname(_hostname);
    Serial.print("Set hostname to ");
    Serial.println(_hostname);
  }
}

#endif // HAVE_WIFI
