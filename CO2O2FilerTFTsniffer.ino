/* This Software takes readings from a CO2 sensor and a O2 sensor and records them
 *  to a SD Card
 */

#include <Adafruit_GFX.h>    // Core graphics library
#include <Adafruit_ST7735.h> // Hardware-specific library
#include <Wire.h>
#include <NDIR_I2C.h> 
#include <SPI.h>
#include <SD.h>

// For the breakout, you can use any 2 or 3 pins
// These pins will also work for the 1.8" TFT shield
#define TFT_CS     10
#define TFT_RST    9  // you can also connect this to the Arduino reset
                      // in which case, set this #define pin to 0!
#define TFT_DC     6

// Option 1 (recommended): must use the hardware SPI pins
// (for UNO thats sclk = 13 and sid = 11) and pin 10 must be
// an output. This is much faster - also required if you want
// to use the microSD card (see the image drawing example)
//Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS,  TFT_DC, TFT_RST);

// Option 2: use any pins but a little slower!
#define TFT_SCLK 13   // set these to be whatever pins you like!
#define TFT_MOSI 11   // set these to be whatever pins you like!
Adafruit_ST7735 tft = Adafruit_ST7735(TFT_CS, TFT_DC, TFT_MOSI, TFT_SCLK, TFT_RST);


#include <Adafruit_ADS1015.h>
Adafruit_ADS1015 ads;     /* Use thi for the 12-bit version */
// Set the pins used
#define cardSelect 4
double  calibrationv; //used to store calibrated value
int sensorcheck=0;//to check health on sensor. If value is 0 sensor works, if value is 1 sensor out of range or not connected
int Sensor_lowrange=58;//When sensor is healthy and new it reads 58 on low
int Sensor_highrange=120;//When sensor is healthy and new it reads 106 on high
int current_function=0;
float triangleTimer = 0;
float timerChecker = 0;

int i = 0;
File logfile;
NDIR_I2C mySensor(0x4D); //Adaptor's I2C address (7-bit, default: 0x4D)
char filename[15];

void setup()
{
  
   // Use this initializer if you're using a 1.8" TFT
  tft.initR(INITR_BLACKTAB);   // initialize a ST7735S chip, black tab

  ads.setGain(GAIN_SIXTEEN);    // 16x gain  +/- 0.256V  1 bit = 0.125mV  0.0078125mV
  ads.begin();
  calibrationv=calibrate();
  tft.fillScreen(ST7735_BLACK);
  tft.setRotation(1); 
    Serial.begin(9600);
 if (!SD.begin(cardSelect)) {
    Serial.println("Card init. failed!");
   
  }
  
  strcpy(filename, "ANALOG00.TXT");//this section sets up new files for each new reading
  for (uint8_t i = 0; i < 100; i++) {
    filename[6] = '0' + i/10;
    filename[7] = '0' + i%10;
    // create if does not exist, do not open existing, write, sync after write
    if (! SD.exists(filename)) {
      break;
    }
  }
   logfile = SD.open(filename, FILE_WRITE);
  if( ! logfile ) {
    Serial.print("Couldnt create "); 
    Serial.println(filename);
    
  }
  Serial.print("Writing to "); 
  Serial.println(filename);
  mySensor.begin();
   if (!mySensor.begin()) {
       
   
        tft.println("ERROR: Failed to connect to the sensor.");
        while(1);
    }
  triangleTimer = millis();
   
  tft.fillScreen(ST7735_BLACK);
  tft.setCursor(20, 30);
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(2);
  tft.println("SNORKEL");
  tft.setCursor(20, 60);
  tft.println("SNIFFER");
  while(millis() - triangleTimer < 10000);
    
    
    
  
  tft.fillScreen(ST7735_BLACK);
  
   
      

  if ((calibrationv > Sensor_highrange) || (calibrationv < Sensor_lowrange))
   {
    sensorcheck=1;
     current_function=1;//Sensor needs to be calibrated
     need_calibrating();//print need calibrating message
   } 
   
   
}

void loop() {
    double modr;//Variable to hold mod value in
    int16_t adc0=0;
    double result;//After calculations holds the current O2 percentage
    double currentmv; //the current mv put out by the oxygen sensor;
    double calibratev;
 
  
  timerChecker = millis();
    if (mySensor.measure()) {
        Serial.print("CO2 Concentration is ");
        
        Serial.print(mySensor.ppm);
        Serial.println("ppm");
    } else {
        Serial.println("Sensor communication error.");
    }
      
     for(int i=0; i<=19; i++)//section takes average of 20 readings of O2 sensor
       {
         adc0=adc0+ads.readADC_SingleEnded(0);
       }
      
      
      currentmv = adc0/20;
      calibratev=calibrationv;
      result=(currentmv/calibratev)*20.9;//current reading divided by calibration set 
     
     
      Serial.print("O2%");
     
      Serial.print(" ");
      
      Serial.println(result,1);
      logfile.print("CO2 PPM  "); logfile.print(mySensor.ppm);
      logfile.print("  O2%");
      logfile.print("  ");
      logfile.println(result);
      logfile.flush();
       tft.setTextWrap(false);
  //tft.fillScreen(ST7735_BLACK);
  tft.setCursor(0,5);
  tft.setTextColor(ST7735_WHITE);
  tft.setTextSize(1);
  tft.print("  File  ");
  tft.println(filename);
  tft.setCursor(0, 30);
  tft.setTextColor(ST7735_MAGENTA,ST7735_BLACK);
  tft.setTextSize(2);
  tft.println("  Oxygen");
  tft.print("  ");
  tft.print(result);
  tft.println(" %");
  tft.setTextColor(ST7735_GREEN,ST7735_BLACK);
  tft.setTextSize(2);
  tft.println("  CO2");
  tft.print("  ");
  tft.print(mySensor.ppm);
  tft.println("    ");
  
  
      while(millis()-timerChecker<1000);
    
}
void need_calibrating(){
   
  tft.setTextColor(ST7735_GREEN);
  tft.setTextSize(2);
  tft.println("Sensor Error");
  Serial.println("Sensor error");
  Serial.println("Please");
  Serial.println("calibrate"); 
  Serial.println(calibrationv);
  
  
}
// freshly calibrates sensor at start of program with (hopefully) fresh air....
int calibrate(){
  int16_t adc0=0;
  int result;
  for(int i=0; i<=19; i++)
       {
         adc0=adc0+ads.readADC_SingleEnded(0);
       }
  
  result=adc0/20;
  return result;
}


