#include <Encoder.h>
#include <Button.h>
#include <SPI.h>
#include <Wire.h>
#include <CapacitiveSensor.h>
#include <math.h>
//#include <Adafruit_GFX.h>
//#include <Adafruit_SSD1306.h>
#include "U8glib.h"
#include "PinChangeInterrupt.h"

#define OLED_RESET 4
//Adafruit_SSD1306 display(OLED_RESET);
//U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_NONE);
U8GLIB_SH1106_128X64 u8g(U8G_I2C_OPT_DEV_0|U8G_I2C_OPT_FAST);

// This block's ID
// This must be unique for every block (from 0 to 4)
const int ID = 4;

int counter = 0;
// TEMP
// DEBUG - parameter change detection
bool change = false;
bool viewChange = false;
bool soloChange = false;

/*
  struct Effect {
  char name[32];
  int fx1 = 0;
  int fx2 = 0;
  int fx3 = 0;
  } effect;
  struct Effect effects[4];
*/

/* ------------- PINS ------------- */

// LED Pin
const int mutePin = A1;
const int soloPin = 11;

// Button Pins
Button buttonNext = Button(8, PULLUP);
Button buttonPrev = Button(9, PULLUP);
Button buttonMute = Button(10, PULLUP);
CapacitiveSensor   cs_4_2 = CapacitiveSensor(13,12);

// My External Methods
void printToOLED(void);

// Stores currently selected effect
//typedef enum {REVERB, COMPRESSOR, EQUALISER} effect;
//effect currentEffect = REVERB;

// ------------ EFFECTS ------------
// Storing the selected effect and each parameter
// REVERB=1, COMPRESSOR=2, EQ=3, SENDS=4
int currentEffect = 1;
int maxEffect = 3;
// Reverb
int reverbFx1 = 0;
int reverbFx2 = 40;
int reverbFx3 = 60;
// Compressor
int compressorFx1 = 0;
int compressorFx2 = 64;
int compressorFx3 = 30;
// Equaliser
int equaliserFx1 = 50;
int equaliserFx2 = 64;
int equaliserFx3 = 64;
float eqCurve[128];
// Sends
int sendFx1 = 0;
int sendFx2 = 0;
int sendFx3 = 0;
// Mixer
bool mute = false;
int solo = 0;

// Encoder 1 pins
const int PinCLK1 = 2;                 // Used for generating interrupts using CLK signal
const int PinDT1 = 3;                  // Used for reading DT signal
//const int PinSW1=4;                    // Used for the push button switch
Encoder enc1(PinDT1, PinCLK1);
static long oldPos1  = -999;

// Encoder 2 pins
const int PinCLK2 = 5;                 // Used for generating interrupts using CLK signal
const int PinDT2 = 4;                  // Used for reading DT signal
//const int PinSW2=4;                    // Used for the push button switch
Encoder enc2(PinDT2, PinCLK2);
static long oldPos2  = -999;

// Encoder 3 pins
const int PinCLK3 = 7;                 // Used for generating interrupts using CLK signal
const int PinDT3 = 6;                  // Used for reading DT signal
//const int PinSW3=4;                    // Used for the push button switch
Encoder enc3(PinDT3, PinCLK3);
static long oldPos3  = -999;

void initialiseEnc(){
  switch (currentEffect) {
    case 1:
      enc1.write(-reverbFx1);
      enc2.write(-reverbFx2);
      enc3.write(-reverbFx3);
      break;
    case 2:
      enc1.write(-compressorFx1);
      enc2.write(-compressorFx2);
      enc3.write(-compressorFx3);
      break;
    case 3:
      enc1.write(-equaliserFx1);
      enc2.write(-equaliserFx2);
      enc3.write(-equaliserFx3);
      break;
    case 4:
      enc1.write(-sendFx1);
      enc2.write(-sendFx2);
      enc3.write(-sendFx3);
      break;
    default:
      break;
  }
}

void setEffect(int id) {
  currentEffect = id;

  // Bounds
  if (currentEffect > maxEffect) currentEffect = 1;
  if (currentEffect < 1) currentEffect = maxEffect;
  viewChange = true; //Debug

  // Correct encoder positions
  initialiseEnc();
  //Serial.println(enc1.read());

}

void toggleMute() {
  if (mute) mute = false;
  else {
    mute = true;
  }
  change = true;
}


void printScreenDebug() {
  if (currentEffect == 1) {
    Serial.println(F("REVERB"));
  }
  if (currentEffect == 2) {
    Serial.println(F("COMPRESSOR"));
  }
  if (currentEffect == 3) {
    Serial.println(F("EQUALISER"));
  }
  if (currentEffect == 4) {
    Serial.println(F("SENDS"));
  }
}

// This function is responsible for lighting the LEDs correctly
void printToLED() {
  if (mute){
    digitalWrite(mutePin, HIGH); 
  }
  else{
    digitalWrite(mutePin, LOW); 
  }
  if (solo){
    digitalWrite(soloPin, HIGH);
    digitalWrite(mutePin, LOW);   // Override the mute LED
  }
  else {
    digitalWrite(soloPin, LOW); 
  }
  
  
}

void printToOLED() {
  /*
  u8g.firstPage();  
  do {
    clearOLED();
  } while( u8g.nextPage() );*/
  u8g.firstPage();  
  do {
    printToOLEDb();
  } while( u8g.nextPage() );
  if (currentEffect == 3) {
    u8g.firstPage();  
    do {
      printToOLEDb();
    } while( u8g.nextPage() );
  }
}

void clearOLED() {
}

// This function is responsible for drawing everything on the screen
void printToOLEDb() {
  //u8g.clearDisplay();
  //u8g.setCursor(2, 2);
  //u8g.setTextSize(1);
  char buffer[16];
  // MUTE/SOLO
  if (solo > 0) {
  u8g.drawStr(128-8, 10,"S");
  }
  else if (mute) {
  u8g.drawStr(128-8, 10,"M");
  }
  if (currentEffect == 1) {
    u8g.drawStr(64-(6*14/2), 10,"<-  REVERB  ->");
    /*
    // TEXT DISPLAY
    display.setCursor(2, 14);
    display.print("Wetness:");
    display.setCursor(76, 14);
    display.print(reverbFx1 * 100 / 127);
    display.setCursor(104, 14);
    display.print("%");

    display.setCursor(2, 26);
    display.print("Size:");
    display.setCursor(76, 26);
    display.print(reverbFx2 * 100 / 127);
    display.setCursor(104, 26);
    display.print("%");

    display.setCursor(2, 38);
    display.print("Decay Time:");
    display.setCursor(76, 38);
    display.print((float)reverbFx3 * 5 / 127);
    display.setCursor(104, 38);
    display.print("s");
    */
    // DIAL DISPLAY
    float dialRatio = 0.8;
    int r0 = 4;
    int r = 14;
    int d1x = 26, d2x = 62, d3x = 98;
    
    u8g.drawCircle(d1x, 32, r);
    int x0 = r0 * cos(((dialRatio - 0.1) * PI) + (reverbFx1 * dialRatio * 2 * PI / 127));
    int y0 = r0 * sin(((dialRatio - 0.1) * PI) + (reverbFx1 * dialRatio * 2 * PI / 127));
    int x1 = r*cos(((dialRatio-0.1) * PI) + (reverbFx1 * dialRatio * 2 * PI / 127));
    int y1 = r*sin(((dialRatio-0.1) * PI) + (reverbFx1 * dialRatio * 2 * PI / 127));
    u8g.drawLine(d1x+x0, 32+y0, d1x+x1, 32+y1);

    u8g.drawCircle(d2x, 32, r);
    x0 = r0 * cos(((dialRatio - 0.1) * PI) + (reverbFx2 * dialRatio * 2 * PI / 127));
    y0 = r0 * sin(((dialRatio - 0.1) * PI) + (reverbFx2 * dialRatio * 2 * PI / 127));
    x1 = r*cos(((dialRatio-0.1) * PI) + (reverbFx2 * dialRatio * 2 * PI / 127));
    y1 = r*sin(((dialRatio-0.1) * PI) + (reverbFx2 * dialRatio * 2 * PI / 127));
    u8g.drawLine(d2x+x0, 32+y0, d2x+x1, 32+y1);

    u8g.drawCircle(d3x, 32, r);
    x0 = r0 * cos(((dialRatio - 0.1) * PI) + (reverbFx3 * dialRatio * 2 * PI / 127));
    y0 = r0 * sin(((dialRatio - 0.1) * PI) + (reverbFx3 * dialRatio * 2 * PI / 127));
    x1 = r*cos(((dialRatio-0.1) * PI) + (reverbFx3 * dialRatio * 2 * PI / 127));
    y1 = r*sin(((dialRatio-0.1) * PI) + (reverbFx3 * dialRatio * 2 * PI / 127));
    u8g.drawLine(d3x+x0, 32+y0, d3x+x1, 32+y1);

    u8g.drawStr(d1x-22, 62,"Dry/Wet");
    u8g.drawStr(d2x-8, 62,"Size");
    u8g.drawStr(d3x-10, 62,"Damp");
    
      /*
      // ROOM VIEW
      int cX = 64; int cY = 40;
      int viewOffsetY = 0;

      // Front wall
      float sizeXinit = reverbFx2;
      sizeXinit = 126;
      float sizeX = 126;
      //float sizeX = (sizeXinit*0.5)+(127*0.5); // Corrections
      float ar = 0.3+((sizeXinit/127)*0.1);
      float sizeY = 127*ar;
      display.drawRect(cX-(sizeX/2), cY-(sizeY/2), sizeX, sizeY, WHITE);

      // Back wall
      float time = ((float)reverbFx2/127);
      //time = 0.8;
      //time = time * ((sizeXinit/127));  // Relative to room size
      time = (time*0.3)+0.06;           // Corrections
      time = 1 - time;                  // Flip
      float backX = sizeX * time;
      float backY = sizeY * time;
      display.drawRect(cX-(backX/2), cY-(backY/2)+viewOffsetY, backX, backY, WHITE);

      // Lines
      display.drawLine(cX-(sizeX/2), cY-(sizeY/2), cX-(backX/2), cY-(backY/2)+viewOffsetY, WHITE);
      display.drawLine(cX+(sizeX/2), cY-(sizeY/2), cX+(backX/2), cY-(backY/2)+viewOffsetY, WHITE);
      display.drawLine(cX-(sizeX/2), cY+(sizeY/2), cX-(backX/2), cY+(backY/2)+viewOffsetY, WHITE);
      display.drawLine(cX+(sizeX/2), cY+(sizeY/2), cX+(backX/2), cY+(backY/2)+viewOffsetY, WHITE);

      display.setCursor(cX-36, cY-10);
      display.print("Dry/Wet:");
      display.setCursor(cX+12, cY-10);
      display.print(reverbFx1*100/127);
      display.setCursor(cX+30, cY-10);
      display.print("%");

      display.setCursor(cX-36, cY+2);
      display.print("Damp:");
      display.setCursor(cX+12, cY+2);
      display.print(reverbFx3*100/127);
      display.setCursor(cX+30, cY+2);
      display.print("%");
      */
  }
  else if (currentEffect == 2) {
    u8g.drawStr(64-(6*18/2), 10,"<-  COMPRESSOR  ->");
    // Define compressor's image coords
    int imageSize = 48;
    int startX = 8; int startY = 62;
    int endX = startX + imageSize; int endY = startY - imageSize;
    // Define other points for line drawing
    float thresh = (float)compressorFx1 / 127;
    float ratio = (float)compressorFx2 / 127;
    int ratioStartX = endX - (thresh * imageSize);
    int ratioStartY = endY + (thresh * imageSize);
    int ratioEndY = endY + (ratio * (ratioStartY - endY));
    // Draw compressor curve
    u8g.drawLine(startX, startY, ratioStartX, ratioStartY);
    u8g.drawLine(ratioStartX, ratioStartY, endX, ratioEndY);
    u8g.drawFrame(startX, endY, imageSize, imageSize);
    // Parameter text
    u8g.drawStr(62, 28,"T:");
    u8g.drawStr(78, 28,"- ");
    
    // First parameter - Threshold
    char buf1[9];
    int value1 = compressorFx1 * 64 / 127;
    sprintf (buf1, "%d", value1);
    u8g.drawStr(88, 28,buf1);
    u8g.drawStr(104, 28,"dB");
    
    // Second parameter - Ratio
    u8g.drawStr(62, 40,"R:");
    char buf2[9];
    int value2 = (int)pow(((float)compressorFx2 * 8 / 127),2);
    sprintf (buf2, "%d", value2+1);
    u8g.drawStr(88, 40,buf2);
    u8g.drawStr(104, 40,":1");
    
    // Third parameter - Attack
    u8g.drawStr(62, 52,"A:");
    //itoa((float)(compressorFx3 / 127), buffer, 10);
    char buf3[9];
    int leftDec = (int)(compressorFx3 * 1.6 / 127);
    int rightDec = (int)(((compressorFx3 * 1.6 / 127)-leftDec)*10);
    sprintf (buf3, "%d.%d", leftDec, rightDec);
    u8g.drawStr(84, 52,buf3);
    //u8g.drawStr(76, 52, buffer);
    u8g.drawStr(104, 52,"s");
  }
  else if (currentEffect == 3) {
    u8g.drawStr(64-(6*17/2), 10,"<-  EQUALISER  ->");
    u8g.drawFrame(0, 12, 128, 52);
    /*
    int lineY = 42;
    int x = 0;
    float freq = equaliserFx1;
    float amp = (equaliserFx2 * 52 / 127) - 26;
    float q = 32 - (equaliserFx3 * 32 / 127) + 1;
    q = 32-q;
    float a = 0;
    float oldy = lineY;
    // Draw the EQ curve
    while (x < 128) {
      float y = amp * exp(-pow(x - freq, 2) / (2 * pow(q, 2)));
      y = lineY - y;
      u8g.drawPixel(x, y);
      oldy = y;
      x++;
    }*/
    int x = 0;
    while (x < 128-2) {
      u8g.drawPixel(x, eqCurve[x]);
      x+=2;
    }
    
  }
  else if (currentEffect == 4) {
    u8g.drawStr(64-(6*13/2), 10,"<-  SENDS  ->");
    float dialRatio = 0.8;
    int r0 = 4;
    int r = 14;
    int d1x = 26, d2x = 62, d3x = 98;
    u8g.drawCircle(d1x, 32, r);
    int x0 = r0 * cos(((dialRatio - 0.1) * PI) + (sendFx1 * dialRatio * 2 * PI / 127));
    int y0 = r0 * sin(((dialRatio - 0.1) * PI) + (sendFx1 * dialRatio * 2 * PI / 127));
    int x1 = r * cos(((dialRatio - 0.1) * PI) + (sendFx1 * dialRatio * 2 * PI / 127));
    int y1 = r * sin(((dialRatio - 0.1) * PI) + (sendFx1 * dialRatio * 2 * PI / 127));
    u8g.drawLine(d1x + x0, 32 + y0, d1x + x1, 32 + y1);
    u8g.drawStr(d1x - 4, 62,"1");
    /*
    u8g.drawCircle(d2x, 32, r);
    x0 = r0 * cos(((dialRatio - 0.1) * PI) + (sendFx2 * dialRatio * 2 * PI / 127));
    y0 = r0 * sin(((dialRatio - 0.1) * PI) + (sendFx2 * dialRatio * 2 * PI / 127));
    x1 = r * cos(((dialRatio - 0.1) * PI) + (sendFx2 * dialRatio * 2 * PI / 127));
    y1 = r * sin(((dialRatio - 0.1) * PI) + (sendFx2 * dialRatio * 2 * PI / 127));
    u8g.drawLine(d2x + x0, 32 + y0, d2x + x1, 32 + y1);
    u8g.drawStr(d2x - 4, 62,"2");

    u8g.drawCircle(d3x, 32, r);
    x0 = r0 * cos(((dialRatio - 0.1) * PI) + (sendFx3 * dialRatio * 2 * PI / 127));
    y0 = r0 * sin(((dialRatio - 0.1) * PI) + (sendFx3 * dialRatio * 2 * PI / 127));
    x1 = r * cos(((dialRatio - 0.1) * PI) + (sendFx3 * dialRatio * 2 * PI / 127));
    y1 = r * sin(((dialRatio - 0.1) * PI) + (sendFx3 * dialRatio * 2 * PI / 127));
    u8g.drawLine(d3x + x0, 32 + y0, d3x + x1, 32 + y1);
    u8g.drawStr(d3x - 4, 62,"3");*/

  }
  //display.display();
}

void computeEqCurve(){
  int lineY = 42;
  int x = 0;
  float freq = equaliserFx1;
  float amp = (equaliserFx2 * 52 / 127) - 26;
  float q = 32 - (equaliserFx3 * 32 / 127) + 1;
  q = 32-q;
  // Draw the EQ curve
  while (x < 128) {
    float y = amp * exp(-pow(x - freq, 2) / (2 * pow(q, 2)));
    y = lineY - y;
    eqCurve[x] = y;
    x++;
  }
}

/*
  void debugPins() {
  Serial.print("PINS" );
  for (int i=1; i<9; i++) {
    Serial.print(", " );Serial.print(i);Serial.print(":" );
    Serial.print(digitalRead(i));
  }
  Serial.println(" ");
  }
*/

// Reads the given encoder and modifies the given FX parameter
void readEncoder(Encoder *enc, long *oldPos, int *param) {
  long newPos = enc->read();
  newPos *= -1;
  if (newPos != *oldPos) {
    // Bound encoder 0 - 127
    if (newPos < 0) {
      newPos = 0;
      enc->write(0);
    }
    else if (newPos > 127) {
      newPos = 127;
      enc->write(-127);
    }
    *oldPos = newPos;
    *param = newPos;
    change = true;
  }
}

void readEncoder1() {
  switch (currentEffect) {
    case 1:
      readEncoder(&enc1, &oldPos1, &reverbFx1);
      break;
    case 2:
      readEncoder(&enc1, &oldPos1, &compressorFx1);
      break;
    case 3:
      readEncoder(&enc1, &oldPos1, &equaliserFx1);
      break;
    case 4:
      readEncoder(&enc1, &oldPos1, &sendFx1);
      break;
    default:
      break;
  }
}

void readEncoder2() {
  switch (currentEffect) {
    case 1:
      readEncoder(&enc2, &oldPos2, &reverbFx2);
      break;
    case 2:
      readEncoder(&enc2, &oldPos2, &compressorFx2);
      break;
    case 3:
      readEncoder(&enc2, &oldPos2, &equaliserFx2);
      break;
    case 4:
      readEncoder(&enc2, &oldPos2, &sendFx2);
      break;
    default:
      break;
  }
}

void readEncoder3() {
  switch (currentEffect) {
    case 1:
      readEncoder(&enc3, &oldPos3, &reverbFx3);
      break;
    case 2:
      readEncoder(&enc3, &oldPos3, &compressorFx3);
      break;
    case 3:
      readEncoder(&enc3, &oldPos3, &equaliserFx3);
      break;
    case 4:
      readEncoder(&enc3, &oldPos3, &sendFx3);
      break;
    default:
      break;
  }
}

// Sends this block's parameters to Max 7 via SERIAL communication
void sendParameters() {
  // All parameters are from 0 - 127
  // Block ID
  Serial.print(ID);

  // Reverb parameters
  Serial.print(" ");
  Serial.print(reverbFx1);
  Serial.print(" ");
  Serial.print(reverbFx2);
  Serial.print(" ");
  Serial.print(reverbFx3);

  // Compression parameters
  Serial.print(" ");
  Serial.print(compressorFx1);
  Serial.print(" ");
  Serial.print(compressorFx2);
  Serial.print(" ");
  Serial.print(compressorFx3);

  // Equaliser parameters
  Serial.print(" ");
  Serial.print(equaliserFx1);
  Serial.print(" ");
  Serial.print(equaliserFx2);
  Serial.print(" ");
  Serial.print(equaliserFx3);

  // Send parameters
  Serial.print(" ");
  Serial.print(sendFx1);
  Serial.print(" ");
  Serial.print(sendFx2);
  Serial.print(" ");
  Serial.print(sendFx3);

    // Mixer parameters
  Serial.print(" ");
  Serial.print(mute);
  Serial.print(" ");
  Serial.print(solo);

  // End line
  Serial.println();
}

void setup() {
  // Setup OLED Screen
  // assign default color value
  if ( u8g.getMode() == U8G_MODE_R3G3B2 ) {
    u8g.setColorIndex(255);     // white
  }
  else if ( u8g.getMode() == U8G_MODE_GRAY2BIT ) {
    u8g.setColorIndex(3);         // max intensity
  }
  else if ( u8g.getMode() == U8G_MODE_BW ) {
    u8g.setColorIndex(1);         // pixel on
  }
  else if ( u8g.getMode() == U8G_MODE_HICOLOR ) {
    u8g.setHiColorByRGB(255,255,255);
  }
  u8g.setFont(u8g_font_tpssr);
  u8g.setRot180();
  printToOLED();

  // PIN SETUPS
  /*
    pinMode(8, INPUT);
    pinMode(PinCLK1,INPUT);
    pinMode(PinDT1,INPUT);
    pinMode(PinCLK2,INPUT);
    pinMode(PinDT2,INPUT);
    pinMode(PinCLK3,INPUT);
    pinMode(PinDT3,INPUT);
  */
  cs_4_2.set_CS_AutocaL_Millis(0xFFFFFFFF);
  pinMode(mutePin, OUTPUT);
  pinMode(soloPin, OUTPUT);
  pinMode(PinCLK1,INPUT_PULLUP);
  attachPCINT (digitalPinToPCINT(PinCLK1),readEncoder1,CHANGE);   // interrupt 0 is always connected to pin 2 on Arduino UNO
  // set pin to input with a pullup, led to output
  pinMode(PinCLK2, INPUT_PULLUP);
  attachPCINT(digitalPinToPCINT(PinCLK2), readEncoder2, CHANGE);
  pinMode(PinCLK3, INPUT_PULLUP);
  attachPCINT(digitalPinToPCINT(PinCLK3), readEncoder3, CHANGE);
  
  Serial.begin (9600);
  //while (!Serial) ;
  Serial.println("Start");
  initialiseEnc();
}

void loop() {
  change = false; // Reset change indicator
  viewChange = false;
  
  /*
  // Choose current effect
  switch (currentEffect) {
    case 1:
      //readEncoder(&enc1, &oldPos1, &reverbFx1);
      //readEncoder(&enc2, &oldPos2, &reverbFx2);
      //readEncoder(&enc3, &oldPos3, &reverbFx3);
      break;
    case 2:
      //readEncoder(&enc1, &oldPos1, &compressorFx1);
      readEncoder(&enc2, &oldPos2, &compressorFx2);
      readEncoder(&enc3, &oldPos3, &compressorFx3);
      break;
    case 3:
      readEncoder(&enc1, &oldPos1, &equaliserFx1);
      readEncoder(&enc2, &oldPos2, &equaliserFx2);
      readEncoder(&enc3, &oldPos3, &equaliserFx3);
      break;
    case 4:
      readEncoder(&enc1, &oldPos1, &sendFx1);
      readEncoder(&enc2, &oldPos2, &sendFx2);
      readEncoder(&enc3, &oldPos3, &sendFx3);
      break;
    default:
      break;
  }*/

  // Read button
  if (buttonNext.uniquePress()) setEffect(currentEffect + 1);
  if (buttonPrev.uniquePress()) setEffect(currentEffect - 1);
  if (buttonMute.uniquePress()) {
    if (solo == 0) {
      solo = ID+1;
    }
    else if (solo > 0) {
      solo = 0;
    }
    //mute = !mute;
    change = true;
  }
   
  // Read capacative sensor
  /*
  long capTotal =  cs_4_2.capacitiveSensor(30);
  int threshold = 160;
  //if (capTotal > 30) Serial.println(capTotal);
  if (capTotal > threshold) {
    solo = ID+1; 
    //mute = false;
    change = true;
  }
  else if (solo > 0) {
    solo = 0;
    change = true;
  }*/
  
  
  
  // If state has changed, send to Max 7
  if (change) {
    sendParameters();   // Send block data to Max 7
    printToLED();
  }

  if (viewChange || change) {
    computeEqCurve();   // Precompute the EQ curve
    printToOLED();      // Print to OLED screen
  }

  // If view state has changed, print
  if (viewChange) {
    //printScreenDebug();
  }

  //debugPins();
  //delay(100);
  counter++;
  //Serial.println(counter);
}
