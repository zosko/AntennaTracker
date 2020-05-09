/*############################################## CONFIGURATION ####################################################
  # Comment/uncomment/edit according to your needs.
  ##################################################################################################################*/
#define CONFIG_VERSION 1000 // Changing this will reset eeprom to default values
//########## BOARD ################################################################################################
#define MEGA // Arduino Mega board

//Minimum distance in meters where it will stop moving servos.
#define DONTTRACKUNDER  5

// Default tilt angle used when not tracking.
#define DEFAULTELEVATION  15

//########### LCD ##################################################################################################
#define I2CADRESS 0x27

//#################################### SERVOS ENDPOINTS #############################################################
#define PAN_MAXPWM 450     //max pan servo pwm value
#define PAN_MAXANGLE 90     //Max angle clockwise (on the right) relative to PAN_MAXPWM. 
#define PAN_MINPWM 2400     //min pan servo pwm valuemin pan servo pwm value
#define PAN_MINANGLE 90      //Max angle counter clockwise (on the left) relative to PAN_MINPWM.

#define TILT_MAXPWM 630   //max tilt pwm value 
#define TILT_MAXANGLE 180    //max tilt angle considering 0° is facing toward.
#define TILT_MINPWM 2400    //min tilt pwm value
#define TILT_MINANGLE 0     //minimum tilt angle. Considering 0 is facing toward, a -10 value would means we can tilt 10° down.



//########################################### BOARDS PINOUTS #########################################################
//DON'T EDIT THIS IF YOU DON'T KNOW WHAT YOU'RE DOINGG
//pinout for Arduino Mega 1280/2560
#ifdef MEGA
#define PAN_SERVOPIN     11	//PWM Pin for pan servo
#define TILT_SERVOPIN    12   //PWM Pin for tilt servo
#define LEFT_BUTTON_PIN  32   //Any Digital pin
#define RIGHT_BUTTON_PIN 34   //Any Digital pin
#define ENTER_BUTTON_PIN 36   //Any Digital pin
#endif

//###############################################END OF CONFIG#######################################################
