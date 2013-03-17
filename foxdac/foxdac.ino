/******************************************************************************
 * Libraries
 ******************************************************************************/

/*
 * This library covers our OLED.
 *
 * http://u8glib.googlecode.com/files/u8glib_arduino_v1.09.zip
 */
#include <U8glib.h>

/******************************************************************************
 * Structures
 ******************************************************************************/

struct location
{
  const u8g_fntpgm_uint8_t *font;
  int x;
  int y;
};

/******************************************************************************
 * Settings
 ******************************************************************************/

// Switch
#define debounce_time 250  // Time in milliseconds to debounce switch
#define SELECTPIN A0      // Switch to select function
#define hold_time 1500  // Time in milliseconds to access main menu from top screen
unsigned long debounceMillis = 0;  // Stores last recorded time for switch debounce interval
boolean button_already_pressed = false;

/* Rotary encoder handler for arduino.
 *
 * Copyright 2011 Ben Buxton. Licenced under the GNU GPL Version 3.
 * Contact: bb@cactii.net
 *
 * Quick implementation of rotary encoder routine.
 *
 * More info: http://www.buxtronix.net/2011/10/rotary-encoders-done-properly.html
 *
 */

// Half-step mode?
#define HALF_STEP
// Arduino pins the encoder is attached to. Attach the center to ground.
#define ROTARY_PIN1 2
#define ROTARY_PIN2 3
// define to enable weak pullups.
#define ENABLE_PULLUPS


#ifdef HALF_STEP
// Use the half-step state table (emits a code at 00 and 11)
const char ttable[6][4] = {
  {0x3 , 0x2, 0x1,  0x0}, {0x83, 0x0, 0x1,  0x0},
  {0x43, 0x2, 0x0,  0x0}, {0x3 , 0x5, 0x4,  0x0},
  {0x3 , 0x3, 0x4, 0x40}, {0x3 , 0x5, 0x3, 0x80}
};
#else
// Use the full-step state table (emits a code at 00 only)
const char ttable[7][4] = {
  {0x0, 0x2, 0x4,  0x0}, {0x3, 0x0, 0x1, 0x40},
  {0x3, 0x2, 0x0,  0x0}, {0x3, 0x2, 0x1,  0x0},
  {0x6, 0x0, 0x4,  0x0}, {0x6, 0x5, 0x0, 0x80},
  {0x6, 0x5, 0x4,  0x0},
};
#endif
volatile char state = 0;

/* Call this once in setup(). */
void rotary_init() {
  pinMode(ROTARY_PIN1, INPUT);
  pinMode(ROTARY_PIN2, INPUT);
#ifdef ENABLE_PULLUPS
  digitalWrite(ROTARY_PIN1, HIGH);
  digitalWrite(ROTARY_PIN2, HIGH);
#endif
}

/* Read input pins and process for events. Call this either from a
 * loop or an interrupt (eg pin change or timer).
 *
 * Returns 0 on no event, otherwise 0x80 or 0x40 depending on the direction.
 */
char rotary_process() {
  char pinstate = (digitalRead(ROTARY_PIN2) << 1) | digitalRead(ROTARY_PIN1);
  state = ttable[state & 0xf][pinstate];
  return (state & 0xc0);
}

// Volume
#define volume_default 0x64 //-50 dB this is 50x2=100 or 0x64. Sabre32 is 0 to -127dB in .5dB steps
#define volume_min 0xC6     //-99dB this is 99X2=198 or 0xC6 -Can go higher but LCD shows 2 digits
#define volume_max 0x00     //-0 dB -Maximum volume is -0 dB
#define dim 0x8C          //-70 dB this is 70x2=140 or 0x8C. Dim volume
#define ramp 10           // Additional msec delay per 1 db for ramping up to volume after dim

byte volume_current = volume_default;  // Variable to hold the current attenuation value

// OLED
#define OLED_DEVICE U8GLIB_NHD31OLED_2X_BW
static const int pin_oled_sck = 52;
static const int pin_oled_mosi = 51;
static const int pin_oled_cs = 53;
static const int pin_oled_a0 = 9;

static const struct location menu_splash_loc = {u8g_font_fub30, 40, 50};
static const double menu_splash_secs = 1.0;
static const char menu_splash_text[] = "FOXDAC";

static const struct location menu_input_source_loc = {u8g_font_9x15, 0, 32};
static const struct location menu_sample_rate_loc = {u8g_font_9x15, 0, 57};
static const struct location menu_vol_loc = {u8g_font_fub49n, 95, 57};
static const int menu_vol_width = 40;
static const struct location menu_db_loc = {u8g_font_fub30, 195, 57};

#define MENU_ITEM_FONT u8g_font_9x15
static const int menu_items_y_offsets[] = {10, 23, 36, 49, 62};
static const int menu_num_lines = sizeof(menu_items_y_offsets) / sizeof(int);

/******************************************************************************
 * Global Objects
 ******************************************************************************/

OLED_DEVICE screen(pin_oled_cs, pin_oled_a0);

/******************************************************************************
 * Menu Functions
 ******************************************************************************/
 
void screen_print(const struct location *loc, char *str)
{ 
  screen.setFont(loc->font);
  screen.drawStr(loc->x, loc->y, str);
}

/******************************************************************************
 * Top-Level Functions
 ******************************************************************************/

void setup()
{
  rotary_init();
  
  pinMode(SELECTPIN, INPUT);      // Button switch or pin for encoder built-in switch 
  digitalWrite(SELECTPIN, HIGH);  // Enable pull-up resistor for switch

  screen.firstPage();
  do {
    screen_print(&menu_splash_loc, "FOXDAC");
  } while(screen.nextPage());

  delay(menu_splash_secs * 1000);

  draw(volume_default);
}

void loop()
{  
  char result = rotary_process();
  
  if (result == 0x40 && volume_current < volume_min)
  {
    volume_current+=2;
    draw(volume_current);
  }
  else if (result == -0x80 && volume_current > volume_max)
  {
    volume_current-=2;
    draw(volume_current);
  }

  if (digitalRead(SELECTPIN) == LOW && button_already_pressed == false)
  {
    debounceMillis = millis();
    button_already_pressed = true;
  }
  else if (digitalRead(SELECTPIN) == HIGH)
  {
    button_already_pressed = false;
  }
  
  if (button_already_pressed && millis() - debounceMillis > hold_time)
  {
    draw(volume_default);
  }
  
}

void draw(byte volume_current)
{
  char volume[4];
  snprintf(volume, sizeof(volume), "%d", volume_current/-2);
  
  // Draw the main screen.
  screen.firstPage();
  do {
    screen_print(&menu_input_source_loc, "SPDIF");
    screen_print(&menu_sample_rate_loc, "44,100 Hz");

    screen.setFont(menu_vol_loc.font);
    screen.drawStr(menu_vol_loc.x + menu_vol_width, menu_vol_loc.y, volume);

    screen_print(&menu_db_loc, "dB");
  } while(screen.nextPage());
}

