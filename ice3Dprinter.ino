/////////////////////////////////////////////////////////////////
//  Autor: Carlos Campos
//  Project: Ice 3D printer
/////////////////////////////////////////////////////////////////
#include <OneWire.h>
#include <SPI.h>
#include <Wire.h>
#include <AutoPID.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>

#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 32 // OLED display height, in pixels
#define OLED_RESET     -1 // Reset pin # (or -1 if sharing Arduino reset pin)

#define ONE_WIRE_BUS 12    //Sensors Pins
#define STEP_PIN 8
#define DIR_PIN 7
#define POT_PIN 2
#define FAN_PIN 10

#define OUTPUT_MIN 300
#define OUTPUT_MAX 12000
#define KP 1200
#define KI 7
#define KD 200000

Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);

char buf[5];

#define N_Sensors 3
byte addr[N_Sensors][8] = {                                   //Temperature sensors have a physical address
  {0x28, 0xFF, 0xF0, 0x30, 0xA2, 0x17, 0x05, 0xA2},
  {0x28, 0xEE, 0x7D, 0x28, 0x2C, 0x15, 0x00, 0xF5},
  {0x28, 0xFF, 0xC1, 0x5F, 0xA2, 0x17, 0x05, 0xE1}
};

float temps[N_Sensors];

double temperature, outputVal;
double setPoint = 5;
AutoPID myPID(&temperature, &setPoint, &outputVal, OUTPUT_MIN, OUTPUT_MAX, KP, KI, KD);

OneWire  ds(ONE_WIRE_BUS);  ////////////// on pin 2 (a 4.7K resistor is necessary)
////////////////////////////////////////Temperature Sensors configuration

long period = 2000;
long onTime = 50;

long lastTime = 100;
float fanOutput = 255;

void setup() {

  startDisplay();

  Serial.begin(9600);
  pinMode(STEP_PIN, OUTPUT);
  pinMode(DIR_PIN, OUTPUT);
  pinMode(FAN_PIN, OUTPUT);
  digitalWrite(FAN_PIN, HIGH);
  digitalWrite(DIR_PIN, HIGH);
  askTemperatures();
  delay(1000);
  //myPID.setBangBang(4);
  myPID.setTimeStep(1000);
}

void loop()
{
  int potVal = analogRead(POT_PIN);
  if (potVal != 0) {
    //    outputVal = map(potVal,0,1023,0,OUTPUT_MAX);
    //    tone(STEP_PIN, outputVal);
    potVal = map(potVal, 0, 1023, -6, 6);
    setPoint = potVal;
  }

  if (millis() - lastTime > 1000) {
    lastTime = millis();
    getTemperatures();
    temperature = temps[2];
    myPID.run();
    tone(STEP_PIN, outputVal);
    askTemperatures();

    refreshDisplay();
  }
}


void askTemperatures() {
  /////////////Ask for temperatures
  ds.reset();
  ds.skip();
  ds.write(0x44, 1);
  ////////////// Wait/////////////
}

///////////////////get Temperatures
void getTemperatures() {

  byte data[12];

  for (int j = 0; j < N_Sensors; j++) {
    ds.reset();
    ds.select(addr[j]);
    ds.write(0xBE);                     // Read Scratchpad
    for (int i = 0; i < 9; i++) {           // we need 9 bytes
      data[i] = ds.read();
    }
    int16_t raw = (data[1] << 8) | data[0];
    temps[j] = (float)raw / 16.0;
    //Serial.print(temps[j]);
    //Serial.print("\t");
  }
  //Serial.println();
}
/////////////////////get Temperatures



void refreshDisplay() {
  refreshSerial();
  //refreshOLED();
  //refreshAlterOLED();
}

void refreshSerial() {
  Serial.print(temps[2]);
  Serial.print(" ");
  Serial.print(setPoint);
  Serial.print(" ");
  Serial.print(outputVal / 1000);
  Serial.print("\n");
}

//void refreshOLED() {
//  display.setTextSize(1);      // Normal 1:1 pixel scale
//  display.setTextColor(WHITE); // Draw white text
//  display.cp437(true);         // Use full 256 char 'Code Page 437' font
//
//  display.clearDisplay();
//  display.setCursor(0, 0);
//  display.print("ST=");
//  display.println(setPoint);
//  display.print("T0=");
//  display.println(temps[0]);
//  display.print("T1=");
//  display.println(temps[1]);
//  display.print("T2=");
//  display.println(temps[2]);
//
//  display.setCursor(64, 8);
//  display.print("pO=");
//  display.print(pumpOutput / 255);
//  display.setCursor(64, 16);
//  display.print("fO=");
//  display.print(fanOutput / 255);
//
//  display.display();
//}

void refreshAlterOLED() {
  display.setTextSize(4);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.clearDisplay();
  display.setCursor(0, 0);
  display.print(temps[0]);
  display.display();
}

void displayLogo(void) {
  display.clearDisplay();

  display.setTextSize(1);      // Normal 1:1 pixel scale
  display.setTextColor(WHITE); // Draw white text
  display.cp437(true);         // Use full 256 char 'Code Page 437' font

  display.setCursor(10, 0);
  display.setTextSize(2); // Draw 2X-scale text
  display.println(F("   El \nIceOPrinto"));

  display.display();
  delay(2000);
}

void startDisplay() {
  // SSD1306_SWITCHCAPVCC = generate display voltage from 3.3V internally
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) { // Address 0x3C for 128x32
    Serial.println(F("SSD1306 allocation failed"));
    for (;;); // Don't proceed, loop forever
  }
  //displayLogo();
}
