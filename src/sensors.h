#ifndef SENSORS_H
#define SENSORS_H

#include "config.h"

#ifdef HAVE_EC
#include <DFRobot_EC.h>
#endif

#ifdef HAVE_PH
#include <DFRobot_PH.h>
#endif

#ifdef HAVE_DHT22
#include <DHTesp.h>
#endif

#ifdef HAVE_TEMP_WET
#include <OneWire.h>
#endif

#ifdef HAVE_AHT20
#include <DFRobot_AHT20.h>
#endif

#ifdef HAVE_ENS160
#include <DFRobot_ENS160.h>
#endif

#include "config.h"

struct FuFarmSensorsData
{
  int light;
  float humidity;
  float flow;
  int co2;
  struct FuFarmSensorsTemperature
  {
    float air;
    float wet;
  } temperature;
  float ec;
  float ph;
  int moisture;
  bool waterLevelState;
  struct FuFarmSensorsAirQuality
  {
    // air quality index
    // https://www.home-assistant.io/integrations/waqi/
    uint16_t index;

    // Total Volatile Organic Compounds
    // range: 0–65000, unit: ppb (parts per billion)
    uint16_t tvoc;

    // equivalent CO2 concentration
    // range: 400–65000, unit: ppm (parts per million)
    uint16_t eco2;
  } airQuality;
};

class FuFarmSensors
{
public:
  FuFarmSensors(void (*sen0217InterruptHandler)() = nullptr);
  ~FuFarmSensors();
  void begin();                                           // initialization
  void calibration(unsigned long readIntervalMs = 1000U); // calibration, should be called in a loop, ideally a config mode
  void read(FuFarmSensorsData *dest);                     // read all sensor data
  void sen0217Interrupt();                                // should be called from the interrupt handler passed in the constructor

private:
#ifdef HAVE_DHT22
  DHTesp dht; // Temperature and Humidity
#elif HAVE_AHT20
  DFRobot_AHT20 aht20; // Temperature and Humidity
#endif

#ifdef HAVE_ENS160
  DFRobot_ENS160_I2C ens160; // Air Quality & MultiGas
#endif

#ifdef HAVE_TEMP_WET
  OneWire ds; // Wet temperature chip i/o
#endif

#ifdef HAVE_EC
  DFRobot_EC ecProbe; // EC probe
#endif

#ifdef HAVE_PH
  DFRobot_PH phProbe; // pH probe
#endif

#ifdef HAVE_FLOW
  volatile int pulseCount; // Flow Sensor
  void (*sen0217InterruptHandler)();
#endif

private:
  int readLight();
  int readCO2();
  float readEC(float temperature);
  float readPH(float temperature);
  float readFlow();
  int readMoisture();
  float readTempWet();
  bool readWaterLevelState();

private:
  char buffer[10];
  uint8_t bufferIndex;
  bool cmdSerialDataAvailable();
  char *strupr(char *str);
  uint16_t convertENS160AQItoHA(uint8_t indexRaw);
};

#endif // SENSORS_H
