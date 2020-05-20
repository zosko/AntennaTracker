# R9M_Inav_antenna_tracker
R9M 2019 - iNav tracker - Antenna Tracker<br />
<br />
TRACKER WIRING<br />
<br />
[PARTS]          [ARDUINO MEGA]<br />
PAN SERVO    ---- PIN 11<br />
TILT SERVO   ---- PIN 12<br />
LCD SDA      ---- PIN 20<br />
LCD SCL      ---- PIN 21<br />
BUTTON LEFT  ---- PIN 32<br />
BUTTON RIGHT ---- PIN 34<br />
BUTTON ENTER ---- PIN 36<br />
BLUETOOTH TX ---- PIN 19<br />
<br />
MODULE WIRING<br />
[R9M]                   [ARDUINO]<br />
S.Port ----> 4k7 ohm ---> PIN 2<br />
VCC    ---- VCC<br />
GND    ---- GND<br />
<br />
BLUETOOTH WIRING<br />
[BLUETOOTH]    [ARDUINO]<br />
VCC ---- VCC<br />
GND ---- GND<br />
RX  ---- TX<br />
<br />
# INSTALLATION<br />
1. Install Arduino 1.8.12<br />
2. Extract libraries.zip to Documents/Arduino/libraries<br />
3. Open tracker_station.ino and install it on Mega<br />
4. Open bt_r9m.ino and install it on Arduino Pro<br />
