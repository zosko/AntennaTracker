# R9M_Inav_antenna_tracker
R9M 2019 - iNav tracker - Antenna Tracker

TRACKER WIRING

[PARTS]          [ARDUINO MEGA]
PAN SERVO    ---- PIN 11
TILT SERVO   ---- PIN 12
LCD SDA      ---- PIN 20
LCD SCL      ---- PIN 21
BUTTON LEFT  ---- PIN 32
BUTTON RIGHT ---- PIN 34
BUTTON ENTER ---- PIN 36
BLUETOOTH TX ---- PIN 19

MODULE WIRING
[R9M]                   [ARDUINO]
S.Port ----> 4k7 ohm ---> PIN 2
VCC    ---- VCC
GND    ---- GND

BLUETOOTH WIRING
[BLUETOOTH]    [ARDUINO]
VCC ---- VCC
GND ---- GND
RX  ---- TX

# INSTALLATION
1. Install Arduino 1.8.12
2. Extract libraries.zip to Documents/Arduino/libraries
3. Open tracker_station.ino and install it on Mega
4. Open bt_r9m.ino and install it on Arduino Pro