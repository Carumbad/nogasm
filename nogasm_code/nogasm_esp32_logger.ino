/*
 * Drastically simplified version of Nogasm which will run on an ESP32 and do
 * nothing but run the nogasm analysis code and log data.
 * 
 * Useful to capture data recordings, or for controlling external devices using
 * a third party platform (NodeRED or Home Assistant etc).
 * 
 */
//=======Libraries===============================

// Manage Libraries -> "RunningAverage" by Rob Tillaart
#include "RunningAverage.h"

//=======Hardware Setup===============================

//Pressure Sensor Analog In
#define BUTTPIN 15

//=======Software/Timing options=====================
#define FREQUENCY 60 //Update frequency in Hz

//Update/render period
#define period (1000/FREQUENCY)
#define longBtnCount (LONG_PRESS_MS / period)

//Running pressure average array length and update frequency
#define RA_HIST_SECONDS 25
#define RA_FREQUENCY 6
#define RA_TICK_PERIOD (FREQUENCY / RA_FREQUENCY)
RunningAverage raPressure(RA_FREQUENCY*RA_HIST_SECONDS);
int sensitivity = 0; //orgasm detection sensitivity, persists through different states

int pressure = 0;
int avgPressure = 0; //Running 25 second average pressure

#define DEFAULT_PLIMIT 600
int pLimit = DEFAULT_PLIMIT; //Limit in change of pressure before the vibrator turns off

//=======Setup=======================================
void setup() {
  pinMode(BUTTPIN,INPUT); //default is 10 bit resolution (1024), 0-3.3

  analogReadResolution(12); //changing ADC resolution to 12 bits (4095)
  //analogReadAveraging(32); //To reduce noise, average 32 samples each read.
  
  raPressure.clear(); //Initialize a running pressure average

  delay(3000); // 3 second delay for recovery

  Serial.begin(115200);
}

//=======Main Loop=============================
void loop() {
  static int sampleTick = 0;
  //Run this section at the update frequency (default 60 Hz)
  if (millis() % period == 0) {
    delay(1);
    
    sampleTick++; //Add pressure samples to the running average slower than 60Hz
    if (sampleTick % RA_TICK_PERIOD == 0) {
      raPressure.addValue(pressure);
      avgPressure = raPressure.getAverage();
    }
    
    pressure = analogRead(BUTTPIN);
    
    //Alert that the Pressure voltage amplifier is railing, and the trim pot needs to be adjusted
    if(pressure > 4030){
      Serial.println("Over pressure!");
    }

    //Report pressure and motor data over USB for analysis / other uses. timestamps disabled by default
    Serial.print(millis()); //Timestamp (ms)
    Serial.print(",");
    Serial.print("0"); //Motor speed is always zero (0-255)
    Serial.print(",");
    Serial.print(pressure); //(Original ADC value - 12 bits, 0-4095)
    Serial.print(",");
    Serial.println(avgPressure); //Running average of (default last 25 seconds) pressure
  }
}
