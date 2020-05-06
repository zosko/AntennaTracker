//pan/tilt servos
PWMServo pan_servo;
PWMServo tilt_servo;

/* ########################################  VARIABLES #####################################################*/
//Telemetry variables
int32_t      uav_lat = 0;                    // latitude
int32_t      uav_lon = 0;                    // longitude
float        lonScaleDown = 0.0;             // longitude scaling
int      uav_satellites_visible = 0;     // number of satellites
int32_t      uav_alt = 0;                    // altitude (dm)
int32_t      rel_alt = 0;                    // relative altitude to home

long lastpacketreceived;

//home
int32_t home_lon;
int32_t home_lat;
int32_t home_alt;
int16_t home_bearing = 0;
uint32_t home_dist;
uint8_t home_sent = 0;

//tracking
int Bearing;
int Elevation;
int servoBearing = 0;
int servoElevation = 0;

//lcd
char lcd_line1[21];
char lcd_line2[21];
char lcd_line3[21];
char lcd_line4[21];


//status
int current_activity = 0; // Activity status 0: Menu , 1: Track, 2: SET_HOME, 3: PAN_MINPWM, 4: PAN_MINANGLE, 5: PAN_MAXPWM,
// 6: PAN_MAXANGLE, 7: TILT_MINPWM, 8: TILT_MINANGLE, 9: TILT_MAXPWM, 10: TILT_MAXANGLE, 11: TEST_SERVO
boolean gps_fix      = false;
boolean telemetry_ok = false;
boolean home_pos     = false;
boolean home_bear    = false;

//servo temp configuration before saving
int servoconf_tmp[4];
int servoconfprev_tmp[4];
uint8_t test_servo_step = 1;
uint16_t test_servo_cnt = 360;
//baudrate selection

/*##################################### STRINGS STORED IN FLASH #############################################*/

FLASH_STRING(string_load1,      "    INAV TRACKER    ");
FLASH_STRING(string_load2,      "                    ");
FLASH_STRING(string_shome1,     "  Waiting for Data  ");
FLASH_STRING(string_shome2,     "   No GPS 3D FIX    ");
FLASH_STRING(string_shome3,     "3D FIX! Alt:");
FLASH_STRING(string_shome4,     "    Please Wait.    ");
FLASH_STRING(string_shome5,     "(long press to quit)");
FLASH_STRING(string_shome6,     " Save Home pos now? ");
FLASH_STRING(string_shome7,     " Set Heading:       ");
FLASH_STRING(string_shome8,     " Move UAV 20m ahead ");
FLASH_STRING(string_shome9,     " & press enter      ");
FLASH_STRING(string_shome10,    "    HOME IS SET     ");
FLASH_STRING(string_shome11,    "Enter:Start Tracking");
FLASH_STRING(string_shome12,    "<< Menu     Reset >>");
FLASH_STRING(string_servos1,    "    [PAN SERVO]     ");
FLASH_STRING(string_servos2,    "    [TILT SERVO]    ");
FLASH_STRING(string_servos3,    "   TESTING SERVOS   ");
FLASH_STRING(string_servos4,    "   CONFIGURATION    ");
/*########################################### MENU ##################################################*/
MenuSystem displaymenu;
Menu rootMenu("");
MenuItem m1i1Item("START");
MenuItem m1i2Item("SET HOME");
Menu m1m3Menu("CONFIG");
Menu m1m3m1Menu("SERVOS");
Menu m1m3m1m1Menu("PAN");
MenuItem m1m3m1m1l1Item("MINPWM");
MenuItem m1m3m1m1l2Item("MAXPWM");
MenuItem m1m3m1m1l3Item("MINANGLE");
MenuItem m1m3m1m1l4Item("MAXANGLE");
Menu m1m3m1m2Menu("TILT");
MenuItem m1m3m1m2l1Item("MINPWM");
MenuItem m1m3m1m2l2Item("MAXPWM");
MenuItem m1m3m1m2l3Item("MINANGLE");
MenuItem m1m3m1m2l4Item("MAXANGLE");
MenuItem m1m3m1i3Item("TEST");


/*##################################### COMMON FUNCTIONS #############################################*/
void attach_servo(PWMServo &s, int p, int min, int max) {
  // called at setup() or after a servo configuration change in the menu
  if (!s.attached()) {
    s.attach(p, min, max);
  }
}

void detach_servo(PWMServo &s) {
  // called at setup() or after a servo configuration change in the menu
  if (s.attached()) {
    s.detach();
  }
}

template <class T> int EEPROM_write(int ee, const T& value)
{
  const byte* p = (const byte*)(const void*)&value;
  unsigned int i;
  cli();
  for (i = 0; i < sizeof(value); i++)
    EEPROM.write(ee++, *p++);
  sei();
  return i;
}

template <class T> int EEPROM_read(int ee, T& value)
{
  byte* p = (byte*)(void*)&value;
  unsigned int i;
  cli();
  for (i = 0; i < sizeof(value); i++)
    *p++ = EEPROM.read(ee++);
  sei();
  return i;
}


//Configuration stored in EEprom
struct config_t // 28 bytes
{
  int config_crc;
  int pan_minpwm;
  int pan_minangle;
  int pan_maxpwm;
  int pan_maxangle;
  int tilt_minpwm;
  int tilt_minangle;
  int tilt_maxpwm;
  int tilt_maxangle;
  int bearing;
} configuration;



void clear_eeprom() {
  // clearing eeprom
  cli();
  for (int i = 0; i < 1025; i++) {
    EEPROM.write(i, 0);
  }
  configuration.config_crc = CONFIG_VERSION;  // config version check
  configuration.pan_minpwm = PAN_MINPWM;
  configuration.pan_minangle = PAN_MINANGLE;
  configuration.pan_maxpwm = PAN_MAXPWM;
  configuration.pan_maxangle = PAN_MAXANGLE;
  configuration.tilt_minpwm = TILT_MINPWM;
  configuration.tilt_minangle = TILT_MINANGLE;
  configuration.tilt_maxpwm = TILT_MAXPWM;
  configuration.tilt_maxangle = TILT_MAXANGLE;
  configuration.bearing = 0;
  EEPROM_write(1, configuration);

  sei();
}
