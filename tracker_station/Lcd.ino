//LCD

void init_lcdscreen() {
  char extract[20];

  LCD.begin(20, 4);
  delay(20);
  LCD.backlight();
  delay(250);
  LCD.noBacklight();
  delay(250);
  LCD.backlight();
  delay(250);
  LCD.setCursor(0, 0);
  LCD.print(string_load1.copy(extract));
  LCD.setCursor(0, 1);
  LCD.print(string_load2.copy(extract));
  LCD.setCursor(0, 3);
  delay(1500); //delay to init lcd in time.

}

void store_lcdline( int i, char sbuffer[20] ) {

  switch (i) {
    case 1:
      strcpy(lcd_line1, sbuffer);
      break;
    case 2:
      strcpy(lcd_line2, sbuffer);
      break;
    case 3:
      strcpy(lcd_line3, sbuffer);
      break;
    case 4:
      strcpy(lcd_line4, sbuffer);
      break;
    default:
      break;
  }

}

void refresh_lcd() {
  // refreshing lcd at defined update.
  // update lines
  LCD.setCursor(0, 0);
  LCD.print(lcd_line1);
  LCD.setCursor(0, 1);
  LCD.print(lcd_line2);
  LCD.setCursor(0, 2);
  LCD.print(lcd_line3);
  LCD.setCursor(0, 3);
  LCD.print(lcd_line4);
}

void lcddisp_menu() {
  Menu const* displaymenu_current = displaymenu.get_current_menu();
  MenuComponent const* displaymenu_sel = displaymenu_current->get_selected();

  uint8_t selected_item;
  uint8_t menu_components_number;
  uint8_t m;
  selected_item = displaymenu_current->get_cur_menu_component_num();
  menu_components_number = displaymenu_current->get_num_menu_components();
  for (int n = 1; n < 5 ; n++)  {
    char currentline[21];
    if ( menu_components_number >= n ) {
      if (menu_components_number <= 4)
        m = n;
      else if (selected_item < (menu_components_number - selected_item - 1))
        m =  selected_item + n ;
      else
        m =  menu_components_number - (menu_components_number - n - 1);
      MenuComponent const* displaymenu_comp = displaymenu_current->get_menu_component(m - 1);
      sprintf(currentline, displaymenu_comp->get_name());
      for ( int l = strlen(currentline); l < 19 ; l++ ) {
        strcat(currentline, " ");
      }
      if (displaymenu_sel == displaymenu_comp)
        strcat(currentline, "<");
      else
        strcat(currentline, " ");
    }
    else {
      string_load2.copy(currentline);
    }
    store_lcdline(n, currentline);
  }
}


// SET_HOME SCREEN
void lcddisp_sethome() {
  for ( int i = 1 ; i < 5; i++ ) {
    char currentline[21] = "";
    char extract[21];
    switch (i) {
      case 1:
        //line1
        if (!telemetry_ok) {
          strcpy(currentline, "NO TELEMETRY");
        }
        else if (telemetry_ok) {
          sprintf(currentline, "SATS:%d",uav_satellites_visible);
        }
        break;
      case 2:
        //line 2
        if (!telemetry_ok)
          string_shome1.copy(currentline); // waiting for data
        else
        {
          if (!gps_fix)
            string_shome2.copy(currentline);  // waiting for gps fix
          else {
            sprintf(currentline, "%s%dm", string_shome3.copy(extract), uav_alt);
          }
        }
        break;

      case 3:
        if (!gps_fix) strcpy(currentline, string_shome4.copy(extract));
        else {
          char bufferl[10];
          char bufferL[10];
          sprintf(currentline, "%s %s", dtostrf(uav_lat / 10000000.0, 5, 5, bufferl), dtostrf(uav_lon / 10000000.0, 5, 5, bufferL));
        }
        break;
      case 4:
        if (!gps_fix)
          strcpy(currentline, string_shome5.copy(extract));
        else
          string_shome6.copy(currentline);
        break;
    }

    for ( int l = strlen(currentline); l < 20 ; l++ )
      strcat(currentline, " ");
    store_lcdline(i, currentline);
  }
}

void lcddisp_setbearing() {
  for (int i = 1 ; i < 5; i++) {
    char currentline[21] = "";
    char extract[21];
    switch (i) {
      case 1:
        if (!telemetry_ok)
        {
          strcpy(currentline, "NO TELEMETRY");
        }
        else if (telemetry_ok)
          sprintf(currentline, "SATS:%d",uav_satellites_visible);
        break;
      case 2:
        string_load2.copy(currentline);
        break;
      case 3:
        string_shome8.copy(currentline);
        break;
      case 4:
        string_shome9.copy(currentline); break;
      default:
        break;

    }
    for ( int l = strlen(currentline); l < 20 ; l++ ) {
      strcat(currentline, " ");
    }
    store_lcdline(i, currentline);
  }
}

void lcddisp_homeok() {
  for ( int i = 1 ; i < 5; i++ ) {
    char currentline[21] = "";
    switch (i) {
      case 1:
        if (!telemetry_ok) {
          strcpy(currentline, "NO TELEMETRY");
        }
        else if (telemetry_ok) sprintf(currentline, "SATS:%d",uav_satellites_visible);
        break;
      case 2:
        string_shome10.copy(currentline); break;
      case 3:
        string_shome11.copy(currentline); break;
      case 4:
        string_shome12.copy(currentline); break;
    }
    for ( int l = strlen(currentline); l < 20 ; l++ ) {
      strcat(currentline, " ");
    }
    store_lcdline(i, currentline);
  }
}

void lcddisp_tracking() {
  for ( int i = 1 ; i < 5; i++ ) {
    char currentline[21] = "";
    switch (i) {
      case 1:
        if (!telemetry_ok)
          strcpy(currentline, "NO TELEMETRY");
        else if (telemetry_ok)
          sprintf(currentline, "SATS:%d",uav_satellites_visible);
        break;
      case 2:
        sprintf(currentline, "Alt:%dm", rel_alt);
        break;
      case 3:
        sprintf(currentline, "Dist:%dm", home_dist / 100);
        break;
      case 4:
        char bufferl[10];
        char bufferL[10];
        sprintf(currentline, "%s %s", dtostrf(uav_lat / 10000000.0, 5, 5, bufferl), dtostrf(uav_lon / 10000000.0, 5, 5, bufferL));
        break;
    }
    for ( int l = strlen(currentline); l < 20 ; l++ ) {
      strcat(currentline, " ");
    }
    store_lcdline(i, currentline);
  }
}

void lcddisp_testservo() {
  for ( int i = 1 ; i < 5; i++ ) {
    char currentline[21] = "";
    char extract[21];
    switch (i) {
      case 1:
        string_servos3.copy(currentline);  break;
      case 2:
        string_servos4.copy(currentline); break;
      case 3:
        string_load2.copy(currentline); break;
      case 4:
        string_shome5.copy(currentline); break;
    }
    for ( int l = strlen(currentline); l < 20 ; l++ ) {
      strcat(currentline, " ");
    }
    store_lcdline(i, currentline);
  }
}

// SERVO CONFIGURATION

int config_servo(int servotype, int valuetype, int value ) {
  // servo configuration screen function return configured value
  //check long press left right
  if (right_button.holdTime() >= 700 && right_button.isPressed() ) {
    value += 20;
    delay(500);
  }
  else if ( left_button.holdTime() >= 700 && left_button.isPressed() ) {
    value -= 20;
    delay(500);
  }
  char currentline[21];
  char extract[21];
  if (servotype == 1) {
    string_servos1.copy(currentline);                              // Pan servo
    store_lcdline(1, currentline);
  }
  else if (servotype == 2) {
    string_servos2.copy(currentline);                              // Tilt servo
    store_lcdline(1, currentline);
  }
  string_load2.copy(currentline);
  store_lcdline(2, currentline);
  switch (valuetype)
  {
    case 1: sprintf(currentline, "min endpoint: <%4d>",  value); break;          //minpwm
    case 2: sprintf(currentline, "min angle: <%3d>    ", value); break;         //minangle
    case 3: sprintf(currentline, "max endpoint: <%4d>",  value); break;          //maxpwm
    case 4: sprintf(currentline, "max angle: <%3d>    ", value); break;         //maxangle
  }
  store_lcdline(3, currentline);
  string_shome5.copy(currentline);
  store_lcdline(4, currentline);
  return value;

}
