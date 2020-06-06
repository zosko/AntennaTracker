#include "FrSkySportSensor.h"
#include "FrSkySportSensorInav.h"
#include "FrSkySportSingleWireSerial.h"
#include "FrSkySportDecoder.h"
#include "SimplePacket.h"

FrSkySportSensorInav inav;
FrSkySportDecoder decoder(true);

uint16_t decodeResult;

// PACKET PROTOCOL
SimplePacket packetProtocol;

struct TrackerStruct {
  byte lat[4];
  byte lng[4];
  byte alt[2];
  byte gps_sats;
  byte distance[2];
  byte speed;
  byte voltage[2];
  byte rssi;
  byte current;
  byte heading[2];
  byte flight_mode;
  byte fuel;
  byte pitch;
  byte roll;
} dataTracker;

void setup() {
  Serial.begin(57600);
  decoder.begin(2, &inav);
  packetProtocol.init(&Serial, sizeof(dataTracker));
}
void buffer_append_int32(byte* buffer, int32_t number) {
  buffer[0] = number >> 24;
  buffer[1] = number >> 16;
  buffer[2] = number >> 8;
  buffer[3] = number;
}
void buffer_append_int16(byte* buffer, int32_t number) {
  buffer[0] = number >> 8;
  buffer[1] = number;
}
void loop() {
  decodeResult = decoder.decode();
  if (decodeResult != SENSOR_NO_DATA_ID) {

    buffer_append_int32(dataTracker.lat, inav.getLat());
    buffer_append_int32(dataTracker.lng, inav.getLon());
    buffer_append_int16(dataTracker.alt, inav.getAltitude());
    buffer_append_int16(dataTracker.distance, inav.getDistance());
    buffer_append_int16(dataTracker.heading, inav.getHeading());
    buffer_append_int16(dataTracker.voltage, inav.getVoltage() * 100);

    dataTracker.gps_sats = inav.getGpsState() % 100;
    dataTracker.speed = (int)inav.getSpeed();
    dataTracker.rssi = (int)inav.getRssi();
    dataTracker.current = (int)inav.getCurrent();
    dataTracker.flight_mode = (int)inav.getFlightMode();
    dataTracker.fuel = (int)inav.getFuel();
    dataTracker.pitch = map(inav.getPitch(), -100, 100, 0, 200);
    dataTracker.roll = map(inav.getRoll(), -100, 100, 0, 200);

    packetProtocol.send((byte *)&dataTracker, sizeof(dataTracker));
  }
}
