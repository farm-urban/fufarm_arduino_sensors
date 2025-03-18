#ifndef CONFIG_H
#define CONFIG_H

// #define MOCK ; // Uncomment to skip wifi connection for testing sensors

#ifdef MOCK
#define SAMPLE_WINDOW 5000
#else
// Time in milliseconds - 5 minutes = 1000 * 60 * 5 = 300000
#define SAMPLE_WINDOW 60000
#endif

#ifdef SENSORS_LIGHT_PIN
  #define HAVE_LIGHT
#endif

#ifdef SENSORS_CO2_PIN
  #define HAVE_CO2
#endif

#ifdef SENSORS_EC_PIN
  #define HAVE_EC
#endif

#ifdef SENSORS_PH_PIN
  #define HAVE_PH
#endif

#ifdef SENSORS_MOISTURE_PIN
  #define HAVE_MOISTURE
#endif

#ifdef SENSORS_DHT22_PIN
  #define HAVE_TEMP_HUMIDITY

  // ensure only digital pins configured
  #ifdef ARDUINO
    #if SENSORS_DHT22_PIN < 0 || SENSORS_DHT22_PIN > 13
      #error "Pin configured for DHT22 must a digital one."
    #endif
  #else // any other board we have not validated
    #pragma message "⚠️ Unable to validate pin configured for DHT22."
  #endif
#endif

#ifdef SENSORS_SEN0217_PIN
  #define HAVE_FLOW

  // For this flow sensor, only interrupt pins should be used. Configured on a rising edge
  // https://www.arduino.cc/reference/en/language/functions/external-interrupts/attachinterrupt/
  // In case, that does not work, try the other another language via:
  // https://www.arduino.cc/reference/cs/language/functions/external-interrupts/attachinterrupt/

  #if defined(ARDUINO_AVR_UNO_WIFI_REV2) // all digital pins
    #if SENSORS_SEN0217_PIN < 0 || SENSORS_SEN0217_PIN > 13
      #error "Pin configured for SEN0217 (flow sensor) must support interrupts."
    #endif
  #elif defined(ARDUINO_AVR_LEONARDO) // only 0, 1, 2, 3, 7
    #if SENSORS_SEN0217_PIN == 0
    #elif SENSORS_SEN0217_PIN == 1
    #elif SENSORS_SEN0217_PIN == 2
    #elif SENSORS_SEN0217_PIN == 3
    #elif SENSORS_SEN0217_PIN == 7
    #else
      #error "Pin configured for SEN0217 (flow sensor) must support interrupts."
    #endif
  #else // any other board we have not validated
    #pragma message "⚠️ Unable to validate if pin configured for SEN0217 (flow sensor) allows interrupts required."
  #endif
#endif

#ifdef SENSORS_SEN0204_PIN
  #define HAVE_WATER_LEVEL_STATE
#endif

#ifdef SENSORS_DS18S20_PIN
  #define HAVE_TEMP_WET
#endif

#ifdef CALIBRATION_TOGGLE_PIN
  #define SUPPORTS_CALIBRATION

  // ensure only digital pins configured
  #ifdef ARDUINO
    #if CALIBRATION_TOGGLE_PIN < 0 || CALIBRATION_TOGGLE_PIN > 13
      #error "Pin configured for calibration toggle must a digital one."
    #endif
  #else // any other board we have not validated
    #pragma message "⚠️ Unable to validate pin configured for calibration."
  #endif
#endif

#ifndef USE_HOME_ASSISTANT
  #define USE_HOME_ASSISTANT 0
#endif

// Validation of the build configuration
#if !defined(HAVE_TEMP_WET) && (defined(HAVE_EC) || defined(HAVE_PH))
  #pragma message "⚠️ Without DS18S20 (wet temperature), calibration of EC and PH sensors is done using air temperature which may not be as accurate!"
#endif

#if !defined(HAVE_LIGHT) && !defined(HAVE_CO2) && \
    !defined(HAVE_EC) && !defined(HAVE_PH) && \
    !defined(HAVE_MOUSTIRE) && !defined(HAVE_TEMP_HUMIDITY) && \
    !defined(HAVE_FLOW) && !defined(HAVE_TEMP_WET) &&\
    !defined(HAVE_WATER_LEVEL_STATE) &&\
    !defined(MOCK) 
  #error "At least one sensor must be configured unless mocking"
#endif

// WiFi
#if defined(ARDUINO_AVR_UNO_WIFI_REV2)
  #define HAVE_WIFI 1
#elif defined(ARDUINO_AVR_LEONARDO)
  #undef USE_HOME_ASSISTANT
#endif

#endif // CONFIG_H
