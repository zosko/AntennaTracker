#include "Config.h"
#include <avr/pgmspace.h>
#include <Arduino.h>
#include <PWMServo.h>
#include <Wire.h>
#include <Metro.h>
#include <MenuSystem.h>
#include <Button.h>
#include <EEPROM.h>
#include <Flash.h>
#include <EEPROM.h>
#include "tracker_station.h"
#include "FrSkySportSensor.h"
#include "FrSkySportSensorInav.h"
#include "FrSkySportSingleWireSerial.h"
#include "FrSkySportDecoder.h"


/*
   BOF preprocessor bug prevent
*/
#define nop() __asm volatile ("nop")
#if 1
nop();
#endif
/*
   EOF preprocessor bug prevent
*/

//################################### SETTING OBJECTS ###############################################
#include <LiquidCrystal_I2C.h>
LiquidCrystal_I2C LCD(I2CADRESS, 2, 1, 0, 4, 5, 6, 7, 3, POSITIVE);  //   HobbyKing IIC/I2C/TWI Serial 2004 20x4, LCM1602 IIC A0 A1 A2 & YwRobot Arduino LCM1602 IIC V1

//##### LOOP RATES
Metro loop10hz = Metro(100); //10hz loop
Metro loop50hz = Metro(20); // 50hz loop
//##### BUTTONS
Button right_button = Button(RIGHT_BUTTON_PIN, BUTTON_PULLUP_INTERNAL);
Button left_button = Button(LEFT_BUTTON_PIN, BUTTON_PULLUP_INTERNAL);
Button enter_button = Button(ENTER_BUTTON_PIN, BUTTON_PULLUP_INTERNAL);


FrSkySportSensorInav inav;
FrSkySportDecoder decoder(true);

uint32_t currentTime, displayTime;
uint16_t decodeResult;

//#################################### SETUP LOOP ####################################################

void setup() {
  //init setup
  init_menu();
  //retrieve configuration from EEPROM
  EEPROM_read(1, configuration);
  // set temp value for servo pwm config
  servoconf_tmp[0] = configuration.pan_minpwm;
  servoconf_tmp[1] = configuration.pan_maxpwm;
  servoconf_tmp[2] = configuration.tilt_minpwm;
  servoconf_tmp[3] = configuration.tilt_maxpwm;
  home_bearing = configuration.bearing; // use last bearing position of previous session.
  delay(20);
  //clear eeprom & write default parameters if config is empty or wrong
  if (configuration.config_crc != CONFIG_VERSION) {
    clear_eeprom();
    delay(20);
  }
  //init LCD

  init_lcdscreen();
  //start serial com
  init_serial();

  // attach servos
  attach_servo(pan_servo, PAN_SERVOPIN, configuration.pan_minpwm, configuration.pan_maxpwm);
  attach_servo(tilt_servo, TILT_SERVOPIN, configuration.tilt_minpwm, configuration.tilt_maxpwm);

  // move servo to neutral pan & DEFAULTELEVATION tilt at startup
  servoPathfinder(0, DEFAULTELEVATION);

  // setup button callback events
  enter_button.releaseHandler(enterButtonReleaseEvents);
  left_button.releaseHandler(leftButtonReleaseEvents);
  right_button.releaseHandler(rightButtonReleaseEvents);

  delay(500);  // Wait until osd is initialised

  decoder.begin(FrSkySportSingleWireSerial::SOFT_SERIAL_PIN_13, &inav);
}

//######################################## MAIN LOOP #####################################################################
void loop() {

  if (loop10hz.check() == 1) {
    //update buttons internal states
    enter_button.isPressed();
    left_button.isPressed();
    right_button.isPressed();
    //current activity loop
    check_activity();
    //update lcd screen
    refresh_lcd();
  }
  if (loop50hz.check() == 1) {
    //update servos
    if (current_activity == 1) {
      if ((home_dist / 100) > DONTTRACKUNDER) {
        servoPathfinder(Bearing, Elevation); // refresh servo
      }
    }
  }
  get_telemetry();
}

//######################################## ACTIVITIES #####################################################################

void check_activity() {
  if (uav_satellites_visible >= 5) {
    gps_fix = true;
  }
  else
    gps_fix = false;
  switch (current_activity)
  {
    case 0:             //MENU
      Bearing = 0; Elevation = DEFAULTELEVATION;
      lcddisp_menu();
      if (enter_button.holdTime() >= 1000 && enter_button.held()) { //long press
        displaymenu.back();
      }
      break;
    case 1:            //TRACK
      if ((!home_pos) || (!home_bear)) {  // check if home is set before start tracking
        Bearing = 0; Elevation = 0;
        current_activity = 2;             // set bearing if not set.
      } else if (home_bear) {
        antenna_tracking();
        lcddisp_tracking();
        if (enter_button.holdTime() >= 700 && enter_button.held()) { //long press
          current_activity = 0;
        }
      }
      break;
    case 2:            //SET HOME
      if (!home_pos)
        lcddisp_sethome();
      else if (home_pos) {
        if (!home_bear) {
          lcddisp_setbearing();
        }
        else
          lcddisp_homeok();
      }
      if (enter_button.holdTime() >= 700 && enter_button.held()) { //long press
        current_activity = 0;
      }
      break;
    case 3:             //PAN_MINPWM
      servoconf_tmp[0] = config_servo(1, 1, servoconf_tmp[0] );
      if (servoconf_tmp[0] != servoconfprev_tmp[0]) {
        detach_servo(pan_servo);
        attach_servo(pan_servo, PAN_SERVOPIN, servoconf_tmp[0], configuration.pan_maxpwm);
      }
      pan_servo.writeMicroseconds(servoconf_tmp[0]);
      //pan_servo.write(0);
      servoconfprev_tmp[0] = servoconf_tmp[0];
      if (enter_button.holdTime() >= 700 && enter_button.held()) {//long press
        configuration.pan_minpwm = servoconf_tmp[0];
        EEPROM_write(1, configuration);
        detach_servo(pan_servo);
        attach_servo(pan_servo, PAN_SERVOPIN, configuration.pan_minpwm, configuration.pan_maxpwm);
        move_servo(pan_servo, 1, 0, configuration.pan_minangle, configuration.pan_maxangle);
        current_activity = 0;
      }
      break;
    case 4:             //PAN_MINANGLE
      configuration.pan_minangle = config_servo(1, 2, configuration.pan_minangle);
      pan_servo.writeMicroseconds(configuration.pan_minpwm);
      //pan_servo.write(0);
      if (enter_button.holdTime() >= 700 && enter_button.held()) {//long press
        EEPROM_write(1, configuration);
        move_servo(pan_servo, 1, 0, configuration.pan_minangle, configuration.pan_maxangle);
        current_activity = 0;
      }
      break;
    case 5:             //PAN_MAXPWM
      servoconf_tmp[1] = config_servo(1, 3, servoconf_tmp[1] );
      if (servoconf_tmp[1] != servoconfprev_tmp[1]) {
        detach_servo(pan_servo);
        attach_servo(pan_servo, PAN_SERVOPIN, configuration.pan_minpwm, servoconf_tmp[1]);
      }
      pan_servo.writeMicroseconds(servoconf_tmp[1]);
      //pan_servo.write(180);
      servoconfprev_tmp[1] = servoconf_tmp[1];
      if (enter_button.holdTime() >= 700 && enter_button.held()) {//long press
        configuration.pan_maxpwm = servoconf_tmp[1];
        EEPROM_write(1, configuration);
        detach_servo(pan_servo);
        attach_servo(pan_servo, PAN_SERVOPIN, configuration.pan_minpwm, configuration.pan_maxpwm);
        move_servo(pan_servo, 1, 0, configuration.pan_minangle, configuration.pan_maxangle);
        current_activity = 0;
      }
      break;

    case 6:             //PAN_MAXANGLE
      configuration.pan_maxangle = config_servo(1, 4, configuration.pan_maxangle );
      pan_servo.writeMicroseconds(configuration.pan_maxpwm);
      //pan_servo.write(180);
      if (enter_button.holdTime() >= 700 && enter_button.held()) {   //long press
        EEPROM_write(1, configuration);
        move_servo(pan_servo, 1, 0, configuration.pan_minangle, configuration.pan_maxangle);
        current_activity = 0;
      }
      break;
    case 7:             //"TILT_MINPWM"
      servoconf_tmp[2] = config_servo(2, 1, servoconf_tmp[2] );
      if (servoconf_tmp[2] != servoconfprev_tmp[2]) {
        detach_servo(tilt_servo);
        attach_servo(tilt_servo, TILT_SERVOPIN, servoconf_tmp[2], configuration.tilt_maxpwm);
      }
      tilt_servo.writeMicroseconds(servoconf_tmp[2]);
      //tilt_servo.write(0);
      servoconfprev_tmp[2] = servoconf_tmp[2];
      if (enter_button.holdTime() >= 700 && enter_button.held()) {    //long press
        configuration.tilt_minpwm = servoconf_tmp[2];
        EEPROM_write(1, configuration);
        detach_servo(tilt_servo);
        attach_servo(tilt_servo, TILT_SERVOPIN, configuration.tilt_minpwm, configuration.tilt_maxpwm);
        move_servo(tilt_servo, 2, 0, configuration.tilt_minangle, configuration.tilt_maxangle);;
        current_activity = 0;
      }
      break;
    case 8:             //TILT_MINANGLE
      configuration.tilt_minangle = config_servo(2, 2, configuration.tilt_minangle );
      tilt_servo.writeMicroseconds(configuration.tilt_minpwm);
      //tilt_servo.write(0);
      if (enter_button.holdTime() >= 700 && enter_button.held()) {//long press
        EEPROM_write(1, configuration);
        move_servo(tilt_servo, 2, 0, configuration.tilt_minangle, configuration.tilt_maxangle);
        current_activity = 0;
      }
      break;
    case 9:             //"TILT_MAXPWM"
      servoconf_tmp[3] = config_servo(2, 3, servoconf_tmp[3] );
      if (servoconf_tmp[3] != servoconfprev_tmp[3]) {
        detach_servo(tilt_servo);
        attach_servo(tilt_servo, TILT_SERVOPIN, configuration.tilt_minpwm, servoconf_tmp[3]);
      }
      tilt_servo.writeMicroseconds(servoconf_tmp[3]);
      //tilt_servo.write(180);
      servoconfprev_tmp[3] = servoconf_tmp[3];
      if (enter_button.holdTime() >= 700 && enter_button.held()) {//long press
        configuration.tilt_maxpwm = servoconf_tmp[3];
        EEPROM_write(1, configuration);
        detach_servo(tilt_servo);
        attach_servo(tilt_servo, TILT_SERVOPIN, configuration.tilt_minpwm, configuration.tilt_maxpwm);
        move_servo(tilt_servo, 2, 0, configuration.tilt_minangle, configuration.tilt_maxangle);
        current_activity = 0;
      }
      break;
    case 10:                //TILT_MAXANGLE
      configuration.tilt_maxangle = config_servo(2, 4, configuration.tilt_maxangle );
      tilt_servo.writeMicroseconds(configuration.tilt_maxpwm);
      //tilt_servo.write(180);
      if (enter_button.holdTime() >= 700 && enter_button.held()) {//long press
        EEPROM_write(1, configuration);
        move_servo(tilt_servo, 2, 0, configuration.tilt_minangle, configuration.tilt_maxangle);
        current_activity = 0;
      }
      break;
    case 11:               //TEST_SERVO
      test_servos();
      if (enter_button.holdTime() >= 700 && enter_button.held()) {//long press
        current_activity = 0;
        test_servo_cnt = 360;
        test_servo_step = 1;
        servoPathfinder(0, 0);
      }
      break;
  }
}

//######################################## BUTTONS #####################################################################

void enterButtonReleaseEvents(Button &btn)
{
  //Serial.println(current_activity);
  if ( enter_button.holdTime() < 700 ) { // normal press
    if ( current_activity == 0 ) { //button action depends activity state
      displaymenu.select();
    }
    else if ( current_activity == 2 ) {
      if ((gps_fix) && (!home_pos)) {
        //saving home position
        home_lat = uav_lat;
        home_lon = uav_lon;
        home_alt = uav_alt;
        home_pos = true;
        calc_longitude_scaling(home_lat);  // calc lonScaleDown
      }
      else if ((gps_fix) && (home_pos) && (!home_bear)) {

        home_bearing = calc_bearing(home_lon, home_lat, uav_lon, uav_lat); // store bearing relative to north
        home_bear = true;

        configuration.bearing = home_bearing;
        EEPROM_write(1, configuration);
        home_sent = 0;  // resend an OFrame to osd
      }
      else if ((gps_fix) && (home_pos) && (home_bear)) {
        // START TRACKING
        current_activity = 1;
      }
    }

  }

}



void leftButtonReleaseEvents(Button &btn)
{
  if ( left_button.holdTime() < 700 ) {
    if (current_activity == 0) {
      displaymenu.prev();
    }
    else if ( current_activity != 0 && current_activity != 1 && current_activity != 2 ) {
      //We're in a setting area: Left button decrase current value.
      switch (current_activity) {
        case 3:   servoconf_tmp[0]--;            break;
        case 4:   configuration.pan_minangle--;  break;
        case 5:   servoconf_tmp[1]--;            break;
        case 6:   configuration.pan_maxangle--;  break;
        case 7:   servoconf_tmp[2]--;            break;
        case 8:   configuration.tilt_minangle--; break;
        case 9:   servoconf_tmp[3]--;            break;
        case 10:  configuration.tilt_maxangle--; break;
      }
    }
    else if (current_activity == 2) {
      if (gps_fix && home_pos && home_bear) {
        current_activity = 0;
      }
    }
    else if (current_activity == 1 && home_pos && home_bear)
      home_bearing--;
  }
}


void rightButtonReleaseEvents(Button &btn)
{
  if ( right_button.holdTime() < 700 ) {

    if (current_activity == 0) {
      displaymenu.next();
    }
    else if ( current_activity != 0 && current_activity != 1 && current_activity != 2 ) {
      //We're in a setting area: Right button decrase current value.
      switch (current_activity) {
        case 3:  servoconf_tmp[0]++;            break;
        case 4:  configuration.pan_minangle++;  break;
        case 5:  servoconf_tmp[1]++;            break;
        case 6:  configuration.pan_maxangle++;  break;
        case 7:  servoconf_tmp[2]++;            break;
        case 8:  configuration.tilt_minangle++; break;
        case 9:  servoconf_tmp[3]++;            break;
        case 10: configuration.tilt_maxangle++; break;
      }
    }
    else if (current_activity == 2) {
      if (home_pos && home_bear) {
        // reset home pos
        home_pos = false;
        home_bear = false;
        home_sent = 0;
      }
    }
    else if (current_activity == 1 && home_pos && home_bear) {
      home_bearing++;
    }
  }
}

//########################################################### MENU #######################################################################################

void init_menu() {
  rootMenu.add_item(&m1i1Item, &screen_tracking); //start track
  rootMenu.add_item(&m1i2Item, &screen_sethome); //set home position
  rootMenu.add_menu(&m1m3Menu); //configure
  m1m3Menu.add_menu(&m1m3m1Menu); //config servos
  m1m3m1Menu.add_menu(&m1m3m1m1Menu); //config pan
  m1m3m1m1Menu.add_item(&m1m3m1m1l1Item, &configure_pan_minpwm); // pan min pwm
  m1m3m1m1Menu.add_item(&m1m3m1m1l2Item, &configure_pan_maxpwm); // pan max pwm
  m1m3m1m1Menu.add_item(&m1m3m1m1l3Item, &configure_pan_minangle); // pan min angle
  m1m3m1m1Menu.add_item(&m1m3m1m1l4Item, &configure_pan_maxangle); // pan max angle
  m1m3m1Menu.add_menu(&m1m3m1m2Menu); //config tilt
  m1m3m1m2Menu.add_item(&m1m3m1m2l1Item, &configure_tilt_minpwm); // tilt min pwm
  m1m3m1m2Menu.add_item(&m1m3m1m2l2Item, &configure_tilt_maxpwm); // tilt max pwm
  m1m3m1m2Menu.add_item(&m1m3m1m2l3Item, &configure_tilt_minangle); // tilt min angle
  m1m3m1m2Menu.add_item(&m1m3m1m2l4Item, &configure_tilt_maxangle); // tilt max angle
  m1m3m1Menu.add_item(&m1m3m1i3Item, &configure_test_servo);
  displaymenu.set_root_menu(&rootMenu);
}



//menu item callback functions

void screen_tracking(MenuItem* p_menu_item) {
  current_activity = 1;
}

void screen_sethome(MenuItem* p_menu_item) {
  current_activity = 2;
}

void configure_pan_minpwm(MenuItem* p_menu_item) {
  current_activity = 3;
}

void configure_pan_minangle(MenuItem* p_menu_item) {
  current_activity = 4;
}

void configure_pan_maxpwm(MenuItem* p_menu_item) {
  current_activity = 5;
}

void configure_pan_maxangle(MenuItem* p_menu_item) {
  current_activity = 6;
}

void configure_tilt_minpwm(MenuItem* p_menu_item) {
  current_activity = 7;
}

void configure_tilt_minangle(MenuItem* p_menu_item) {
  current_activity = 8;
}

void configure_tilt_maxpwm(MenuItem* p_menu_item) {
  current_activity = 9;
}

void configure_tilt_maxangle(MenuItem* p_menu_item) {
  current_activity = 10;
}

void configure_test_servo(MenuItem* p_menu_item) {
  current_activity = 11;
}

//######################################## TELEMETRY FUNCTIONS #############################################
void init_serial() {
  Serial.begin(57600);
}
//Preparing adding other protocol
void get_telemetry() {
    if (millis() - lastpacketreceived > 2000) {
      telemetry_ok = false;
    }
    sport_read();
}
int sport_read(void) {
  decodeResult = decoder.decode();

  if (decodeResult != SENSOR_NO_DATA_ID) {
    telemetry_ok = true;

    uav_lat = inav.getLat();
    uav_lon = inav.getLon();
    uav_alt = inav.getAltitude();
    uav_satellites_visible = inav.getGpsSats();
    //    home_dist = inav.getDistanceFromHome() * 100; // meter to cm

    Serial.print(" sat:"); Serial.print(uav_satellites_visible);
    Serial.print(" alt:"); Serial.print(uav_alt);
    Serial.print(" lat:"); Serial.print(uav_lat);
    Serial.print(" lon:"); Serial.println(uav_lon);
    Serial.print(" dist:"); Serial.print(home_dist);

    lastpacketreceived = millis();
  }
}

//######################################## SERVOS #####################################################################




void move_servo(PWMServo &s, int stype, int a, int mina, int maxa) {

  if (stype == 1) {
    //convert angle for pan to pan servo reference point: 0° is pan_minangle
    if (a <= 180) {
      a = mina + a;
    } else if ((a > 180) && (a < (360 - mina))) {
      //relevant only for 360° configs
      a = a - mina;
    } else if ((a > 180) && (a > (360 - mina)))
      a = mina - (360 - a);
    // map angle to microseconds
    int microsec = map(a, 0, mina + maxa, configuration.pan_minpwm, configuration.pan_maxpwm);
    s.writeMicroseconds( microsec );
  }
  else if (stype == 2) {
    //map angle to microseconds
    int microsec = map(a, mina, maxa, configuration.tilt_minpwm, configuration.tilt_maxpwm);
    s.writeMicroseconds( microsec );
  }
}

void servoPathfinder(int angle_b, int angle_a) {  // ( bearing, elevation )
  //find the best way to move pan servo considering 0° reference face toward
  if (angle_b <= 180) {
    if ( configuration.pan_maxangle >= angle_b ) {
      //define limits
      if (angle_a <= configuration.tilt_minangle) {
        // checking if we reach the min tilt limit
        angle_a = configuration.tilt_minangle;
      } else if (angle_a > configuration.tilt_maxangle) {
        //shouldn't happend but just in case
        angle_a = configuration.tilt_maxangle;
      }
    } else if ( configuration.pan_maxangle < angle_b ) {
      //relevant for 180° tilt config only, in case bearing is superior to pan_maxangle
      angle_b = 180 + angle_b;
      if (angle_b >= 360) {
        angle_b = angle_b - 360;
      }
      // invert pan axis
      if ( configuration.tilt_maxangle >= ( 180 - angle_a )) {
        // invert pan & tilt for 180° Pan 180° Tilt config
        angle_a = 180 - angle_a;
      }
      else if (configuration.tilt_maxangle < ( 180 - angle_a )) {
        // staying at nearest max pos
        angle_a = configuration.tilt_maxangle;
      }
    }
  }
  else if ( angle_b > 180 ) {
    if ( configuration.pan_minangle > 360 - angle_b ) {
      if (angle_a < configuration.tilt_minangle) {
        // checking if we reach the min tilt limit
        angle_a = configuration.tilt_minangle;
      }
    } else if ( configuration.pan_minangle <= 360 - angle_b ) {
      angle_b = angle_b - 180;
      if ( configuration.tilt_maxangle >= ( 180 - angle_a )) {
        // invert pan & tilt for 180/180 conf
        angle_a = 180 - angle_a;
      }
      else if (configuration.tilt_maxangle < ( 180 - angle_a)) {
        // staying at nearest max pos
        angle_a = configuration.tilt_maxangle;
      }
    }
  }
  move_servo(pan_servo, 1, angle_b, configuration.pan_minangle, configuration.pan_maxangle);
  move_servo(tilt_servo, 2, angle_a, configuration.tilt_minangle, configuration.tilt_maxangle);
}



void test_servos() {
  lcddisp_testservo();
  switch (test_servo_step) {
    case 1:
      if (test_servo_cnt > 180) {
        servoPathfinder(test_servo_cnt, (360 - test_servo_cnt) / 6);
        test_servo_cnt--;
      }
      else
        test_servo_step = 2;
      break;
    case 2:
      if (test_servo_cnt < 360) {
        servoPathfinder(test_servo_cnt, (360 - test_servo_cnt) / 6);
        test_servo_cnt++;
      }
      else {
        test_servo_step = 3;
        test_servo_cnt = 0;
      }
      break;
    case 3:
      if (test_servo_cnt < 360) {
        servoPathfinder(test_servo_cnt, test_servo_cnt / 4);
        test_servo_cnt++;
      }
      else {
        test_servo_step = 4;
        test_servo_cnt = 0;
      }
      break;
    case 4:
      if (test_servo_cnt < 360) {
        servoPathfinder(test_servo_cnt, 90 - (test_servo_cnt / 4));
        test_servo_cnt++;
      }
      else {
        // finished
        test_servo_step = 1;
        current_activity = 0;
        servoPathfinder(0, 0);
      }
      break;
  }
}

//######################################## TRACKING #############################################

void antenna_tracking() {
  // Tracking general function
  //only move servo if gps has a 3D fix, or standby to last known position.
  if (gps_fix && telemetry_ok) {
    rel_alt = uav_alt - home_alt; // relative altitude to ground in decimeters
    calc_tracking( home_lon, home_lat, uav_lon, uav_lat, rel_alt); //calculate tracking bearing/azimuth
    //set current GPS bearing relative to home_bearing
    if (Bearing >= home_bearing) {
      Bearing -= home_bearing;
    }
    else
      Bearing += 360 - home_bearing;
  }
}



void calc_tracking(int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2, int32_t alt) {
  //calculating Bearing & Elevation  in degree decimal
  Bearing = calc_bearing(lon1, lat1, lon2, lat2);
  Elevation = calc_elevation(alt);
}


int16_t calc_bearing(int32_t lon1, int32_t lat1, int32_t lon2, int32_t lat2) {
  float dLat = (lat2 - lat1);
  float dLon = (float)(lon2 - lon1) * lonScaleDown;
  home_dist = sqrt(sq(fabs(dLat)) + sq(fabs(dLon))) * 1.113195; // home dist in cm.
  int16_t b = (int)round( -90 + (atan2(dLat, -dLon) * 57.295775));
  if (b < 0) b += 360;
  return b;
}

int16_t calc_elevation(int32_t alt) {
  float at = atan2(alt, home_dist);
  at = at * 57, 2957795;
  int16_t e = (int16_t)round(at);
  return e;
}

void calc_longitude_scaling(int32_t lat) {
  float rads       = (abs((float)lat) / 10000000.0) * 0.0174532925;
  lonScaleDown = cos(rads);
}