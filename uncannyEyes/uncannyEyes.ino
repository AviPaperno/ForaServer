//Попытка через координаты

//------------------------------------------------------------------------
// Uncanny eyes for PJRC Teensy 3.1 with Adafruit 1.5" OLED (product #1431)
// or 1.44" TFT LCD (#2088).  This uses Teensy-3.1-specific features and
// WILL NOT work on normal Arduino or other boards!  Use 72 MHz (Optimized)
// board speed -- OLED does not work at 96 MHz.
//
// Adafruit invests time and resources providing this open source code,
// please support Adafruit and open-source hardware by purchasing products
// from Adafruit!
//
// Written by Phil Burgess / Paint Your Dragon for Adafruit Industries.
// MIT license.  SPI FIFO insight from Paul Stoffregen's ILI9341_t3 library.
// Inspired by David Boccabella's (Marcwolf) hybrid servo/OLED eye concept.
//--------------------------------------------------------------------------

#include <SPI.h>
#include <Adafruit_GFX.h>      // Core graphics lib for Adafruit displays
// Enable ONE of these #includes -- HUGE graphics tables for various eyes:
#//include "defaultEye.h"        // Standard human-ish hazel eye
#//include "Arc1_Eye.h"    // Large iris, no sclera
//#include "dragonEye.h"         // Slit pupil fiery dragon/demon eye
//#include "goatEye.h"           // Horizontal pupil goat/Krampus eye
//#include "HeartEye.h"
#include "MyEye.h"           //
//#include "MyEyeHuman1.h"           //
//
// Then tweak settings below, e.g. change IRIS_MIN/MAX or disable TRACKING.

// DISPLAY HARDWARE CONFIG -------------------------------------------------

#include <Adafruit_ST7735.h> // TFT display library (enable one only)
//#include <Adafruit_SSD1351.h>  // OLED display library -OR-

#ifdef _ADAFRUIT_ST7735H_
typedef Adafruit_ST7735  displayType; // Using TFT display(s)
#else
typedef Adafruit_SSD1351 displayType; // Using OLED display(s)
#endif

#define DISPLAY_DC      9 // Data/command pin for BOTH displays
#define DISPLAY_RESET   8 // Reset pin for BOTH displays
#define SELECT_L_PIN    10 // LEFT eye chip select pin
#define SELECT_R_PIN   7 // RIGHT eye chip select pin

// INPUT CONFIG (for eye motion -- enable or comment out as needed) --------

//#define JOYSTICK_X_PIN A0 // Analog pin for eye horiz pos (else auto)
//#define JOYSTICK_Y_PIN A1 // Analog pin for eye vert position (")
//#define JOYSTICK_X_FLIP   // If set, reverse stick X axis
//#define JOYSTICK_Y_FLIP   // If set, reverse stick Y axis
#define TRACKING          // If enabled, eyelid tracks pupil
#define IRIS_PIN       A2 // Photocell or potentiometer (else auto iris)
//#define IRIS_PIN_FLIP     // If set, reverse reading from dial/photocell
#define IRIS_SMOOTH       // If enabled, filter input from IRIS_PIN
#define IRIS_MIN      120 // Clip lower analogRead() range from IRIS_PIN
#define IRIS_MAX      720 // Clip upper "
#define WINK_L_PIN      0 // Pin for LEFT eye wink button
#define BLINK_PIN       1 // Pin for blink button (BOTH eyes)
#define WINK_R_PIN      2 // Pin for RIGHT eye wink button
#define AUTOBLINK         // If enabled, eyes blink autonomously

// Probably don't need to edit any config below this line, -----------------
// unless building a single-eye project (pendant, etc.), in which case one
// of the two elements in the eye[] array further down can be commented out.

// Eye blinks are a tiny 3-state machine.  Per-eye allows winks + blinks.
#define NOBLINK 0     // Not currently engaged in a blink
#define ENBLINK 1     // Eyelid is currently closing
#define DEBLINK 2     // Eyelid is currently opening
typedef struct {
  int8_t   pin;       // Optional button here for indiv. wink
  uint8_t  state;     // NOBLINK/ENBLINK/DEBLINK
  uint32_t  duration;  // Duration of blink state (micros) *** why is this a signed integer??? changed to unsigned ***
  uint32_t startTime; // Time (micros) of last state change
} eyeBlink;

struct {
  displayType display; // OLED/TFT object
  uint8_t     cs;      // Chip select pin
  eyeBlink    blink;   // Current blink state
} eye[] = { // OK to comment out one of these for single-eye display:
  displayType(SELECT_L_PIN,DISPLAY_DC,0),SELECT_L_PIN,{WINK_L_PIN,NOBLINK},
  displayType(SELECT_R_PIN,DISPLAY_DC,0),SELECT_R_PIN,{WINK_R_PIN,NOBLINK},
};
#define NUM_EYES (sizeof(eye) / sizeof(eye[0]))

//-------------------------------------------------------------------------  
// Begin Iris Rotation code Variables //
  int16_t Rotation = -67;  // Iris Initial Rotation for static position, 0 if rotating
  int8_t RotationZ = -1;  // Iris Rotation Delay counter. Leave set at -1 for initial value.
  int16_t RotationS = 00;  // Iris Rotation Step Increment, (how much Iris rotates each time it turns, can be a negative number)
  int8_t RotationR = 1;  // Iris Rotation Delay Rate ( > 0 ),  (1 through n, larger number greater delay)
// End Iris Rotation code Variables //
//-------------------------------------------------------------------------  
// Eyelid Pattern Variables
//  int16_t cEyelid = ((((255>>3)<<6) | (231>>2))<<5) | (33>>3);  // convert RGB to RGB16_5:6:5
  uint16_t cEyelid1 = 0;  // eyelid color 1
  uint16_t cEyelid2 = 0;  // eyelid color 2
  uint16_t cEyelid3 = 0;  // eyelid color 3
  uint16_t cEyelid4 = 0;  // eyelid color 4
  uint8_t pattern_offset =0; // eyelid pattern offset for even odd/rows

// Arrays are [Y][X]

const  int8_t pattern_sizeX = 18;
const  int8_t pattern_sizeY = 14;
const  int8_t pattern[14][18] = {
   {1,4,4,3,3,2,2,2,1,1,1,2,2,2,3,3,4,4}, 
   {1,4,4,3,3,2,2,2,1,1,1,2,2,2,3,3,4,4},
   {1,4,4,3,3,2,2,2,1,1,1,2,2,2,3,3,4,4},
   {1,4,4,3,3,2,2,2,1,1,1,2,2,2,3,3,4,4},
   {1,4,4,3,3,2,2,2,1,1,1,2,2,2,3,3,4,4},
   {1,4,4,3,3,2,2,2,1,1,1,2,2,2,3,3,4,4},
   {1,4,4,3,3,2,2,2,1,1,1,2,2,2,3,3,4,4},
   {1,4,4,3,3,2,2,2,2,2,2,2,2,2,3,3,4,4},
   {1,4,4,3,3,2,2,2,2,2,2,2,2,2,3,3,4,4},
   {1,4,4,3,3,3,2,2,2,2,2,2,2,3,3,3,4,4},
   {1,4,4,4,3,3,3,3,3,3,3,3,3,3,3,4,4,4},
   {1,1,4,4,4,3,3,3,3,3,3,3,3,3,4,4,4,1},
   {1,1,2,4,4,4,4,4,4,4,4,4,4,4,4,4,2,1},
   {1,1,2,2,2,4,4,4,4,4,4,4,4,4,2,2,2,1}
    } ;


// End Eyelid Pattern Variables
//-------------------------------------------------------------------------  

uint8_t Select = 0;

//-------------------------------------------------------------------------  
// Convert to RGB565
// The function removes the lower 3 bits from Red and Blue, and the lower 2 bits from Green and applies the Intensity value.
// The Intensity value of 255 is full brightness, 0 is full black.  Some color information is lost in the conversion to RGB565.

uint16_t RGBi888toRGB565(uint8_t R8, uint8_t G8,  uint8_t B8, uint8_t I8) // change the brightness of individual 8 bit RGB color channels and convert to RGB565 
{
//  return ((int((R8 * I8) / 255)>>3)<<11) | ((int((G8*I8) / 255)>>2)<<5) | (int((B8 * I8) / 255)>>3);
  return ((((R8 * I8) / 255) & 248)<<8) | ((((G8 * I8) / 255) & 252)<<3) | (((B8 * I8) / 255)>>3);
}

uint16_t RGBi24toRGB565(uint32_t rgb, uint8_t I8) //Change the brightness of an RGB24 bit color value and convert result to RGB565 
{
//  return (((((rgb>>16) * I8) / 255) & 248)<<8) | ((((((rgb>>8) & 255) * I8) / 255) & 252)<<3) | ((((rgb & 255) * I8) / 255)>>3);
  return (((((rgb & 0xFF0000) * I8) / 255) & 0xF80000)>>8) | (((((rgb & 0x00FF00) * I8) / 255) & 0x00FC00)>>5) | ((((rgb & 0x0000FF) * I8) / 255)>>3);
}

uint16_t RGBi565(uint16_t rgb, uint8_t I8)  //Change the brightness of an RGB565 color value
{
//  return (((((rgb>>11) * I8) / 255) & 31)<<11) | ((((((rgb>>5) & 63) * I8) / 255))<<3) | ((((rgb & 255) * I8)/255)>>3);
  return ((((rgb & 0xF800) * I8) / 255) & 0xF800) | ((((rgb & 0x7E0) * I8) / 255) & 0x7E0) | ((((rgb & 0x1F) * I8)/255) & 0x1F);
}


//-------------------------------------------------------------------------  

// INITIALIZATION -- runs once at startup ----------------------------------

void setup(void) {

  uint8_t e;
    pattern_offset = pattern_sizeX / 2;
//-------------------------------------------------------------------------  
// Eyelid Pattern and ColorVariables
  

// eyelid colors, note convert the caucasian flesh tones to RGB then use the RGBi24toRGB565 converter to adjust to darker skin tones
// RGB565   RGB24      RGB888  
// 0xFEA0 = 0xF86A00 = 248,106,0   //Supposed to be gold
// 0xD566 = 0xD05606 = 208,86,6    //Supposed to be gold
// 0xCCAE = 0xC84A0E = 200,74,14   //Supposed to be brass
// 0xF6B8 = 0xF06A18 = 240,106,24  //Caucasian Flesh Tones
// 0xDD11 = 0xD85011 = 216,80,17   //Caucasian Flesh Tones
// 0xE594 = 0xE05814 = 224,88,20   //Caucasian Flesh Tones
// 0xED93 = 0xE85813 = 232,88,19   //Caucasian Flesh Tones
// 0xFDD4 = 0xF85C14 = 248,92,20   //Caucasian Flesh Tones
// 0xDD74 = 0xD85614 = 216,86,20   //Caucasian Flesh Tones
// 0xE5F5 = 0xE05E15 = 224,94,21   //Caucasian Flesh Tones
// 0xE532 = 0xE05212 = 224,82,18   //Caucasian Flesh Tones
// 0xCCB0 = 0xC84A10 = 200,74,16   //Caucasian Flesh Tones
// 0xAC0E = 0xA8400E = 168,64,14   //Caucasian Flesh Tones
// 0xFDD4 = 0xF85C14 = 248,92,20   //Caucasian Flesh Tones
// 0x1882 = 0x180802 = 24,8,2      //FDD4 at 10% Ebony Flesh Tones, same result should be achieved with RGBi888toRGB565(248,92,20,26)
// 0xFEA8 = 0xF86A08 = 248,106,8   //Minion Yellow1
// 0xFF03 = 0xF87003 = 248,112,3   //Minion Yellow2
// 0xFE8B = 0xF8680B = 248,104,11  //Minion Yellow3
//        = 0xC2EA9A = 194,234,154 //Test Color (217=85%,166=65%,115=45%,64=25%)

//  cEyelid1 = RGBi888toRGB565(194,234,154,217);  //eyelid at 85% brightness.
//  cEyelid2 = RGBi888toRGB565(194,234,154,166);  //eyelid at 65% brightness.
//  cEyelid3 = RGBi888toRGB565(194,234,154,115);  //eyelid at 45% brightness
//  cEyelid4 = RGBi888toRGB565(194,234,154,64);  //eyelid at 25% brightness.
  cEyelid1 = RGBi24toRGB565(0x000000,256);  //eyelid at 85% brightness.
  cEyelid2 = RGBi24toRGB565(0x000000,256);  //eyelid at 65% brightness.
  cEyelid3 = RGBi24toRGB565(0x000000,256); //eyelid at 45% brightness
  cEyelid4 = RGBi24toRGB565(0x000000,256); //eyelid at 25% brightness.
//-------------------------------------------------------------------------  
  
  Serial.begin(115200);

  randomSeed(analogRead(A3)); // Seed random() from floating analog input

  // Both displays share a common reset line; 0 is passifed to display
  // constructor (so no reset in begin()) -- must reset manually here:
  pinMode(DISPLAY_RESET, OUTPUT);
  digitalWrite(DISPLAY_RESET, LOW);  delay(1);
  digitalWrite(DISPLAY_RESET, HIGH); delay(50);

  for(e=0; e<NUM_EYES; e++) { // Deselect all
    pinMode(eye[e].cs, OUTPUT);
    digitalWrite(eye[e].cs, HIGH);
  }
  for(e=0; e<NUM_EYES; e++) {
    digitalWrite(eye[e].cs, LOW); // Select one eye for init
#ifdef _ADAFRUIT_ST7735H_ // TFT
    eye[e].display.initR(INITR_18BLACKTAB);
    eye[e].display.fillScreen(ST7735_BLACK);
#else // OLED
    eye[e].display.begin();
#endif
    if(eye[e].blink.pin >= 0) pinMode(eye[e].blink.pin, INPUT_PULLUP);
    digitalWrite(eye[e].cs, HIGH); // Deselect
  }
#ifdef BLINK_PIN
  pinMode(BLINK_PIN, INPUT_PULLUP);
#endif

  // One of the displays is configured to mirror on the X axis.  Simplifies
  // eyelid handling in the drawEye() function -- no need for distinct
  // L-to-R or R-to-L inner loops.  Just the X coordinate of the iris is
  // then reversed when drawing this eye, so they move the same.  Magic!
#ifdef _ADAFRUIT_ST7735H_ // TFT
  digitalWrite(eye[0].cs , LOW);
  digitalWrite(DISPLAY_DC, LOW);
  SPI.transfer(ST7735_MADCTL);
  digitalWrite(DISPLAY_DC, HIGH);
  SPI.transfer(0x48); // MADCTL_MY | MADCTL_BGR
  digitalWrite(eye[0].cs , HIGH);
#else 
  eye[0].display.writeCommand(SSD1351_CMD_SETREMAP);
  eye[0].display.writeData(0x76);
#endif
}

// EYE-RENDERING FUNCTION --------------------------------------------------

//SPI.begin(); 
//SPISettings settings(24000000, MSBFIRST, SPI_MODE3); // Teensy 3.1 max SPI (works at 72MHz, but not really 24MHz is actually 18MHz)
SPISettings settings(19000000, MSBFIRST, SPI_MODE3); // Teensy 3.1 SSD1351 OLED max SPI (Works at 72MHz-120MHz(oled), will downscale to cpu clock/4 or /8 as necessary to keep below the oled 20MHz limit)
//SPI_CLOCK_DIV2;
//SPI.setClockDivider(0x02);
//SPISettings settings(15000000, MSBFIRST, SPI_MODE3); // Teensy 3.1 max SPI in 96MHz (oled)

void drawEye( // Renders one eye.  Inputs must be pre-clipped & valid.
  uint8_t  e,       // Eye array index; 0 or 1 for left/right
  uint32_t iScale,  // Scale factor for iris
  uint8_t  scleraX, // First pixel X offset into sclera image
  uint8_t  scleraY, // First pixel Y offset into sclera image
  uint8_t  uT,      // Upper eyelid threshold value
  uint8_t  lT,
  uint8_t choose = 0 
  ) {    // Lower eyelid threshold value

  uint8_t  screenX, screenY, scleraXsave;
  int16_t  irisX, irisY;
  uint16_t p, a;
  uint32_t d;
//--------------------------------------------------------------------------------------
//Eyelid Variables
static  uint8_t I, J, K;  // modulus values for pattern
//--------------------------------------------------------------------------------------
  
  // Set up raw pixel dump to entire screen.  Although such writes can wrap
  // around automatically from end of rect back to beginning, the region is
  // reset on each frame here in case of an SPI glitch.
  SPI.beginTransaction(settings);
//  delayMicroseconds(10000);
//  SPI.setClockDivider(SPI_CLOCK_DIV2);
 
#ifdef _ADAFRUIT_ST7735H_ // TFT
  if (e==0) {  eye[e].display.setRotation(3);}
  else {eye[e].display.setRotation(1);}
  //eye[e].display.initR(INITR_18BLACKTAB);
  eye[e].display.setAddrWindow(16, 0, 143, 128);
#else // OLED
  eye[e].display.writeCommand(SSD1351_CMD_SETROW);    // Y range
  eye[e].display.writeData(0); eye[e].display.writeData(SCREEN_HEIGHT - 1);
  eye[e].display.writeCommand(SSD1351_CMD_SETCOLUMN); // X range
  eye[e].display.writeData(0); eye[e].display.writeData(SCREEN_WIDTH  - 1);
  eye[e].display.writeCommand(SSD1351_CMD_WRITERAM);  // Begin write
#endif
  digitalWrite(eye[e].cs, LOW);                       // Chip select
  digitalWrite(DISPLAY_DC, HIGH);                     // Data mode
  // Now just issue raw 16-bit values for every pixel...

  scleraXsave = scleraX; // Save initial X value to reset on each line
  irisY       = scleraY - (SCLERA_HEIGHT - IRIS_HEIGHT) / 2;
  for(screenY=0; screenY<SCREEN_HEIGHT; screenY++, scleraY++, irisY++) {
    scleraX = scleraXsave;
    irisX   = scleraXsave - (SCLERA_WIDTH - IRIS_WIDTH) / 2;
    for(screenX=0; screenX<SCREEN_WIDTH; screenX++, scleraX++, irisX++) {
//--------------------------------------------------------------------------------------
//Colored Eyelid Code
      K = (screenY / pattern_sizeY) % 2; 
      J = screenY % pattern_sizeY;
      I = ((pattern_offset * K) + screenX) % pattern_sizeX;

      if((lower[screenY][SCREEN_WIDTH-1-screenX] <= (lT)) ||
         (upper[screenY][SCREEN_WIDTH-1-screenX] <= (uT))) {             // Covered by eyelid
          switch  (pattern[J][I]) {
            case 1:{
              p = cEyelid1;
                }break;
            case 2:{
              p = cEyelid2;
                }break;
            case 3:{
              p = cEyelid3;
                }break;
            case 4:{
              p = cEyelid4;
                }break;
            default:{
              p = 0;
                }break;}
//--------------------------------------------------------------------------------------
      } else if((irisY < 0) || (irisY >= IRIS_HEIGHT) ||
                (irisX < 0) || (irisX >= IRIS_WIDTH)) { // In sclera
        p = sclera[scleraY][scleraX];
      } else {                                          // Maybe iris...
        p = polar[irisY][irisX];          // Polar angle/dist
        d = (iScale * (p & 0x7F)) / 128;                // Distance (Y)
        if(d < IRIS_MAP_HEIGHT) {                       // Within iris area
          a =IRIS_MAP_WIDTH- (IRIS_MAP_WIDTH * (p >> 7)) / 512;        // Angle (X)
          a = (a + Rotation + IRIS_MAP_WIDTH) % IRIS_MAP_WIDTH;
          // Initial Rotation of iris
          if (e==0) p = iris[d][a];     
          else {
           p=iris[d][IRIS_MAP_WIDTH-a];// Pixel = iris
//            else p=iris1[d][IRIS_MAP_WIDTH-a];// RASK
          }
        } else {                                        // Not in iris
          p = sclera[scleraY][scleraX];                 // Pixel = sclera
        }
      }


      // SPI FIFO technique from Paul Stoffregen's ILI9341_t3 library:
      while(KINETISK_SPI0.SR & 0xC000); // Wait for space in FIFO 0xC000
      KINETISK_SPI0.PUSHR = p | SPI_PUSHR_CTAS(1) | SPI_PUSHR_CONT;
    }
  }

  KINETISK_SPI0.SR |= SPI_SR_TCF;         // Clear transfer flag
  while((KINETISK_SPI0.SR & 0xF000) ||    // Wait for SPI FIFO to drain 0xF000
       !(KINETISK_SPI0.SR & SPI_SR_TCF)); // Wait for last bit out
  digitalWrite(eye[e].cs, HIGH);          // Deselect
  SPI.endTransaction();
  delay(1); // is this really necessary?
//  delayMicroseconds(10000);
}


// EYE ANIMATION -----------------------------------------------------------

const uint8_t ease[] = { }; // n

int16_t tmpx = 512;
int16_t tmpy = 512;
String inString = ""; 

#ifdef AUTOBLINK
uint32_t timeOfLastBlink = 0L, timeToNextBlink = 0L;
#endif

void frame( // Process motion for a single frame of left or right eye
  uint16_t        iScale) {     // Iris scale (0-1023) passed in
  static uint32_t frames   = 0; // Used in frame rate calculation
  static uint8_t  eyeIndex = 0; // eye[] array counter
  int16_t         eyeX, eyeY;
  uint32_t        t = micros(); // Time at start of function

 // Serial.println((++frames * 1000) / millis()); // Show frame rate (framerate)

  if(++eyeIndex >= NUM_EYES) eyeIndex = 0; // Cycle through eyes, 1 per call

  // X/Y movement



 // Serial.write("OKOK");
  // Read X/Y from joystick, constrain to circle
  int16_t dx, dy;
  int32_t d;
  
    while (Serial.available() > 0) {
    char inChar = Serial.read();
    if (isDigit(inChar)) {
      // convert the incoming byte to a char and add it to the string:
      inString += (char)inChar;
      Serial.println(inString);
    }
    // if you get a newline, print the string, then the string's value:
    if (inChar == 'g')
    {
      tmpx = inString.toInt();
      inString = "";
    }

    else if 
    (inChar == 'v')
    {
      tmpy = inString.toInt();
      inString = "";
    }

    else if (inChar == 'n')
    {
      if (Select == 1) Select = 0;
      else Select = 1;
    }
    }
  
  eyeX = tmpx; // Raw (unclipped) X/Y reading
  
  eyeY = tmpy;
  //Serial.print(tmp); Serial.print(" ");Serial.print(eyeX); Serial.print(" ");Serial.println(eyeY);

  if (Select == 1)
{

  dx = (eyeX * 2) - 1023; // A/D exact center is at 511.5.  Scale coords
  dy = (eyeY * 2) - 1023; // X2 so range is -1023 to +1023 w/center at 0.
  if((d = (dx * dx + dy * dy)) > (1023 * 1023)) { // Outside circle
    d    = (int32_t)sqrt((float)d);               // Distance from center
    eyeX = ((dx * 1023 / d) + 1023) / 2;          // Clip to circle edge,
    eyeY = ((dy * 1023 / d) + 1023) / 2;          // scale back to 0-1023
  }
}
else { // Autonomous X/Y eye motion
      // Periodically initiates motion to a new random point, random speed,
      // holds there for random period until next motion.

  static boolean  eyeInMotion      = false;
  static int16_t  eyeOldX=512, eyeOldY=512, eyeNewX=512, eyeNewY=512;
  static uint32_t eyeMoveStartTime = 0L;
  static int32_t  eyeMoveDuration  = 0L;

  int32_t dt = t - eyeMoveStartTime;      // uS elapsed since last eye event
  if(eyeInMotion) {                       // Currently moving?
    if(dt >= eyeMoveDuration) {           // Time up?  Destination reached.
      eyeInMotion      = false;           // Stop moving
      eyeMoveDuration  = random(3000000); // 0-3 sec stop
      eyeMoveStartTime = t;               // Save initial time of stop
      eyeX = eyeOldX = eyeNewX;           // Save position
      eyeY = eyeOldY = eyeNewY;
    } else { // Move time's not yet fully elapsed -- interpolate position
      int16_t e = ease[255 * dt / eyeMoveDuration] + 1;   // Ease curve
      eyeX = eyeOldX + (((eyeNewX - eyeOldX) * e) / 256); // Interp X
      eyeY = eyeOldY + (((eyeNewY - eyeOldY) * e) / 256); // and Y
    }
  } else {                                // Eye stopped
    eyeX = eyeOldX;
    eyeY = eyeOldY;
    if(dt > eyeMoveDuration) {            // Time up?  Begin new move.
      int16_t  dx, dy;
      uint32_t d;
      do {                                // Pick new dest in circle
        eyeNewX = random(1024);
        eyeNewY = random(1024);
     //   dx      = (eyeNewX * 2) - 1023;
        dy      = (eyeNewY * 2) - 1023;
      } while((d = (dx * dx + dy * dy)) > (1023 * 1023)); // Keep trying
      eyeMoveDuration  = random(72000, 144000); // ~1/14 - ~1/7 sec
      eyeMoveStartTime = t;               // Save initial time of move
      eyeInMotion      = true;            // Start move on next frame
    }
  }

} // JOYSTICK_X_PIN etc.

  // Blinking

#ifdef AUTOBLINK
  // Similar to the autonomous eye movement above -- blink start times
  // and durations are random (within ranges).
  if((t - timeOfLastBlink) >= timeToNextBlink) { // Start new blink?
    timeOfLastBlink = t;
    uint32_t blinkDuration = random(36000, 72000); // ~1/28 - ~1/14 sec
    // Set up durations for both eyes (if not already winking)
    for(uint8_t e=0; e<NUM_EYES; e++) {
      if(eye[e].blink.state == NOBLINK) {
        eye[e].blink.state     = ENBLINK;
        eye[e].blink.startTime = t;
        eye[e].blink.duration  = blinkDuration;
      }
    }
    timeToNextBlink = blinkDuration * 3 + random(4000000);
  }
#endif

  if(eye[eyeIndex].blink.state) { // Eye currently blinking?
    // Check if current blink state time has elapsed
    if((t - eye[eyeIndex].blink.startTime) >= eye[eyeIndex].blink.duration) {
      // Yes -- increment blink state, unless...
      if((eye[eyeIndex].blink.state == ENBLINK) &&  // Enblinking and...
        ((digitalRead(BLINK_PIN) == LOW) ||         // blink or wink held...
          digitalRead(eye[eyeIndex].blink.pin) == LOW)) {
        // Don't advance state yet -- eye is held closed instead
      } else { // No buttons, or other state...
        if(++eye[eyeIndex].blink.state > DEBLINK) { // Deblinking finished?
          eye[eyeIndex].blink.state = NOBLINK;      // No longer blinking
        } else { // Advancing from ENBLINK to DEBLINK mode
          eye[eyeIndex].blink.duration *= 2; // DEBLINK is 1/2 ENBLINK speed
          eye[eyeIndex].blink.startTime = t;
        }
      }
    }
  } else { // Not currently blinking...check buttons!
    if(digitalRead(BLINK_PIN) == LOW) {
      // Manually-initiated blinks have random durations like auto-blink
      uint32_t blinkDuration = random(36000, 72000);
      for(uint8_t e=0; e<NUM_EYES; e++) {
        if(eye[e].blink.state == NOBLINK) {
          eye[e].blink.state     = ENBLINK;
          eye[e].blink.startTime = t;
          eye[e].blink.duration  = blinkDuration;
        }
      }
    } else if(digitalRead(eye[eyeIndex].blink.pin) == LOW) { // Wink!
      eye[eyeIndex].blink.state     = ENBLINK;
      eye[eyeIndex].blink.startTime = t;
      eye[eyeIndex].blink.duration  = random(45000, 90000);
    }
  }

  // Process motion, blinking and iris scale into renderable values

  // Iris scaling: remap from 0-1023 input to iris map height pixel units
  iScale = ((IRIS_MAP_HEIGHT + 1) * 1024) /
           (1024 - (iScale * (IRIS_MAP_HEIGHT - 1) / IRIS_MAP_HEIGHT));

  // Scale eye X/Y positions (0-1023) to pixel units used by drawEye()
  eyeX = map(eyeX, 0, 1023, 0, SCLERA_WIDTH  - 128);
  eyeY = map(eyeY, 0, 1023, 0, SCLERA_HEIGHT - 128);
  if(eyeIndex == 1) eyeX = (SCLERA_WIDTH - 128) - eyeX; // Mirrored display

  // Horizontal position is offset so that eyes are very slightly crossed
  // to appear fixated (converged) at a conversational distance.  Number
  // here was extracted from my posterior and not mathematically based.
  // I suppose one could get all clever with a range sensor, but for now...
  eyeX += 4;
  if(eyeX > (SCLERA_WIDTH - 128)) eyeX = (SCLERA_WIDTH - 128);

  // Eyelids are rendered using a brightness threshold image.  This same
  // map can be used to simplify another problem: making the upper eyelid
  // track the pupil (eyes tend to open only as much as needed -- e.g. look
  // down and the upper eyelid drops).  Just sample a point in the upper
  // lid map slightly above the pupil to determine the rendering threshold.
  static uint8_t uThreshold = 128;
  uint8_t        lThreshold, n;
#ifdef TRACKING
  int16_t sampleX = SCLERA_WIDTH  / 2 - (eyeX / 2), // Reduce X influence
          sampleY = SCLERA_HEIGHT / 2 - (eyeY + IRIS_HEIGHT / 4);
  // Eyelid is slightly asymmetrical, so two readings are taken, averaged
  if(sampleY < 0) n = 0;
  else            n = (upper[sampleY][sampleX] +
                       upper[sampleY][SCREEN_WIDTH - 1 - sampleX]) / 2;
  uThreshold = (uThreshold * 3 + n) / 4; // Filter/soften motion
  // Lower eyelid doesn't track the same way, but seems to be pulled upward
  // by tension from the upper lid.
  lThreshold = 254 - uThreshold;
#else // No tracking -- eyelids full open unless blink modifies them
  uThreshold = lThreshold = 0;
#endif

  // The upper/lower thresholds are then scaled relative to the current
  // blink position so that blinks work together with pupil tracking.
  if(eye[eyeIndex].blink.state) { // Eye currently blinking?
    uint32_t s = (t - eye[eyeIndex].blink.startTime);
    if(s >= eye[eyeIndex].blink.duration) s = 255;   // At or past blink end
    else s = 255 * s / eye[eyeIndex].blink.duration; // Mid-blink
    s          = (eye[eyeIndex].blink.state == DEBLINK) ? 1 + s : 256 - s;
    n          = (uThreshold * s + 254 * (257 - s)) / 256;
    lThreshold = (lThreshold * s + 254 * (257 - s)) / 256;
  } else {
    n          = uThreshold;
  }

  // Pass all the derived values to the eye-rendering function:
  drawEye(eyeIndex, iScale, eyeX, eyeY, n, lThreshold, Select);
}


// AUTONOMOUS IRIS SCALING (if no photocell or dial) -----------------------

#if !defined(IRIS_PIN) || (IRIS_PIN < 0)

// Autonomous iris motion uses a fractal behavior to similate both the major
// reaction of the eye plus the continuous smaller adjustments that occur.

uint16_t oldIris = (IRIS_MIN + IRIS_MAX) / 2, newIris;

void split( // Subdivides motion path into two sub-paths w/randimization
  int16_t  startValue, // Iris scale value (IRIS_MIN to IRIS_MAX) at start
  int16_t  endValue,   // Iris scale value at end
  uint32_t startTime,  // micros() at start
  int32_t  duration,   // Start-to-end time, in microseconds
  int16_t  range) {    // Allowable scale value variance when subdividing

  if(range >= 8) {     // Limit subdvision count, because recursion
    range    /= 2;     // Split range & time in half for subdivision,
    duration /= 2;     // then pick random center point within range:
    int16_t  midValue = (startValue + endValue - range) / 2 + random(range);
    uint32_t midTime  = startTime + duration;
    split(startValue, midValue, startTime, duration, range); // First half
    split(midValue  , endValue, midTime  , duration, range); // Second half
  } else {             // No more subdivisons, do iris motion...
    int32_t dt;        // Time (micros) since start of motion
    int16_t v;         // Interim value
    while((dt = (micros() - startTime)) < duration) {
      v = startValue + (((endValue - startValue) * dt) / duration);
      if(v < IRIS_MIN)      v = IRIS_MIN; // Clip just in case
      else if(v > IRIS_MAX) v = IRIS_MAX;
      frame(v);        // Draw frame w/interim iris scale value
    }
  }
}

#endif // !IRIS_PIN


// MAIN LOOP -- runs continuously after setup() ----------------------------

void loop() {

//------- Begin Iris Rotation Code ----------//
    RotationZ = (1 + RotationZ) % RotationR; // Iris Rotation Delay
    if(RotationZ < 1) Rotation = (Rotation + RotationS) % IRIS_MAP_WIDTH;
//------- End Iris Rotation Code ----------//
    
#ifdef IRIS_PIN && (IRIS_PIN >= 0) // Interactive iris  ***

  uint16_t v = analogRead(IRIS_PIN);       // Raw dial/photocell reading
#ifdef IRIS_PIN_FLIP
  v = 1023 - v;
#endif
  v = map(v, 0, 1023, IRIS_MIN, IRIS_MAX); // Scale to iris range
#ifdef IRIS_SMOOTH // Filter input (gradual motion)
  static uint16_t irisValue = (IRIS_MIN + IRIS_MAX) / 2;
  irisValue = ((irisValue * 15) + v) / 16;
  frame(irisValue);
#else // Unfiltered (immediate motion)
  frame(v);
#endif // IRIS_SMOOTH

#else  // Autonomous iris scaling -- invoke recursive function

  newIris = random(IRIS_MIN, IRIS_MAX);
  split(oldIris, newIris, micros(), 10000000L, IRIS_MAX - IRIS_MIN);
  oldIris = newIris;

#endif // IRIS_PIN
}
