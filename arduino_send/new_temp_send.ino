#include <SPI.h>
#include <RF24.h>
#include <OneWire.h>
#include <DallasTemperature.h>
#include "PowerManager.h"
#include "printf.h"

#define ONE_WIRE_BUS 2
#define TEMPERATURE_PRECISION 12

typedef struct
{
  uint32_t id;
  float temperature;
} SPacket;

OneWire oneWire(ONE_WIRE_BUS);
DallasTemperature sensors(&oneWire);
RF24 radio(6, 5);
PowerManager pm;

byte sensorAddr[4][8];
byte targetAddr[6] = {0xC6, 0x2F, 0x6B, 0xA5, 0xC2, 0xD3};
int numOfSensors;
SPacket packet[4];

void setup() {
  pm.setup(PM_WAKE_8S);

  // provide temperature sensors with power
  pinMode(3, OUTPUT);
  digitalWrite(3, HIGH);
  delay(200);

  Serial.begin(57600);
  printf_begin();

  sensors.begin();
  numOfSensors = sensors.getDeviceCount();

  Serial.print("Found ");
  Serial.print(numOfSensors, DEC);
  Serial.println(" sensors.");

  for (int i = 0; i < numOfSensors; i++)
  {
    if (sensors.getAddress(sensorAddr[i], i))
    {
      Serial.print("Found sensor ");
      Serial.print(i, DEC);
      Serial.print(" with address: ");
      printAddress(sensorAddr[i]);
      Serial.println();

      packet[i].id =   ((uint32_t)sensorAddr[i][4] << 24)
                       | ((uint32_t)sensorAddr[i][5] << 16)
                       | ((uint32_t)sensorAddr[i][6] << 8)
                       | ((uint32_t)sensorAddr[i][7]);

      Serial.print("Setting resolution to ");
      Serial.println(TEMPERATURE_PRECISION, DEC);

      sensors.setResolution(sensorAddr[i], TEMPERATURE_PRECISION);

      Serial.print("Resolution actually set to: ");
      Serial.print(sensors.getResolution(sensorAddr[i]), DEC);
      Serial.println();
    } else {
      Serial.print("Found ghost sensor at ");
      Serial.print(i, DEC);
      Serial.print(" but could not detect address. Check power and cabling");
    }
  }

  radio.begin();
  radio.setChannel(0x70);
  radio.setPayloadSize(sizeof(SPacket));
  radio.openWritingPipe(targetAddr);
  radio.openReadingPipe(1, sensorAddr[0]);

  radio.printDetails();
  delay(500);
}

void loop() {
  pm.sleep(3);
  sensors.requestTemperatures();
  for (int i = 0; i < numOfSensors; i++)
  {
    packet[i].temperature = sensors.getTempC(sensorAddr[i]);
    /*
    Serial.print(packet[i].id, DEC);
    Serial.print(": ");
    Serial.println(packet[i].temperature, DEC);
    */
  }
  radio.powerUp();
  for (int i = 0; i < numOfSensors; ++i)
  {
    radio.write(&packet[i], sizeof(SPacket));
  }
  radio.powerDown();
}

void printAddress(DeviceAddress deviceAddress)
{
  for (uint8_t i = 0; i < 8; i++)
  {
    if (deviceAddress[i] < 16) Serial.print("0");
    Serial.print(deviceAddress[i], HEX);
  }
}
