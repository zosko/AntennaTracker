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
  int32_t lat;
  int32_t lng;
  float alt;
  int sats;
} dataTracker;

void setup() {
  Serial.begin(57600);
  decoder.begin(2, &inav);
  packetProtocol.init(&Serial, sizeof(dataTracker));
}
void loop() {
  decodeResult = decoder.decode();
  if (decodeResult != SENSOR_NO_DATA_ID) {
    dataTracker.lat = inav.getLat();
    dataTracker.lng = inav.getLon();
    dataTracker.alt = inav.getAltitude();
    dataTracker.sats = inav.getGpsSats();
    packetProtocol.send((byte *)&dataTracker, sizeof(dataTracker));
  }
}
