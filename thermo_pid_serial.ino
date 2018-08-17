#include <Adafruit_MAX31856.h> //loads library for thermocouple reader
#include <PID_v1.h> //loads PID library
#include <RunningAverage.h> //loads class for Running Avgerage calculations
#define RelayPin 2 //set pin number which is connected to power relay for device

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data

boolean newData = false; //trigger to determine if data needs to be read

Adafruit_MAX31856 max = Adafruit_MAX31856(4, 5, 6, 7); // Software SPI, Pin number for: CS, DI, DO, CLK

//Define Variables
double workingSet, Setpoint, Input, Output;
int WindowSize = 1000; //Length of cycle for power on/off
unsigned long prevprintMillis = 0;
const long printdelay = 5000; //delay amount before printing via serial
unsigned long windowStartTime;
unsigned long prevRampTimer = 0;
unsigned long MaxOP = .95; // Max Operating Power

RunningAverage myRA(10); //Set up running average with # of measurements

PID myPID(&Input, &Output, &workingSet,250,50,8, DIRECT); //Specify the links and initial tuning parameters

void setup() {
  pinMode(RelayPin, OUTPUT); //define RelayPin as an output

  //staring serial communications
  Serial.begin(9600);
  Serial.println("PID Controller Output");
  Serial.println("Setpoint\t Working SP\t TC Temp\t Output");//tab deliminated for logging

  //setup for thermocouple reader
  max.begin();
  max.setThermocoupleType(MAX31856_TCTYPE_K);//change type if needed

  windowStartTime = millis(); //start timer for duty cycle window

  myRA.clear(); // explicitly start clean

  //initialize the variables we're linked to
  workingSet = 20;
  Setpoint = 20;

  myPID.SetOutputLimits(0, MaxOP * WindowSize); //tell the PID to range between 0 and the full window size
  myPID.SetMode(AUTOMATIC);  //turn the PID on
}

//fuction to recieve data via serial and process when endmarker is recieved
void recvWithEndMarker() {
 static byte ndx = 0;
 char endMarker = '\n'; //define line endmarker for client
 char rc;

  while (Serial.available() > 0 && newData == false) {
    rc = Serial.read();
    if (rc != endMarker) {
      receivedChars[ndx] = rc;
      ndx++;
      if (ndx >= numChars) {
        ndx = numChars - 1;
      }
    }
    else {
      receivedChars[ndx] = '\0'; // terminate the string
      ndx = 0;
      newData = true;
    }
  }
}

//Function to repeat new setpoint for confirmation
void showNewData() {
 if (newData == true) {
 String recievedString = String(receivedChars);
 Serial.print("Setpoint changed to ... ");
 Serial.println(recievedString.toFloat());
 Setpoint = recievedString.toFloat();
 newData = false;
 }
}

void loop() {
  recvWithEndMarker(); //check for new setpoint
  showNewData(); //sets new setpoint and displays it
  myRA.addValue(max.readThermocoupleTemperature());//add themocouple read to rolling average
  Input = myRA.getAverage();//sets PID input to rolling average value
  myPID.Compute(); //calculate PID output

//print delay for serial output so not to flood logs
  unsigned long printMillis = millis();
  if (printMillis - prevprintMillis >= printdelay) {
    prevprintMillis = printMillis;
    Serial.print(Setpoint);Serial.print("\t");Serial.print(workingSet);Serial.print("\t"); Serial.print(max.readThermocoupleTemperature());Serial.print("\t");Serial.println(Output/WindowSize*100);
  }

//turn the output pin on/off based on pid output
 if(millis() - windowStartTime>=WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
 if(Output < millis() - windowStartTime) digitalWrite(RelayPin,LOW); //HIGH and LOW set to default off
  else digitalWrite(RelayPin,HIGH);

//sets ramp rate functionality
unsigned long ramptimer = millis();
unsigned long ramprate = 6000;//milliseconds for 1C change
  if(workingSet != Setpoint)
  {
   if(ramptimer - prevRampTimer >= ramprate && workingSet < Setpoint)
   {
    workingSet++;
    prevRampTimer = ramptimer;
   }
   if(ramptimer - prevRampTimer >= ramprate && workingSet > Setpoint)
   {
    workingSet--;
    prevRampTimer = ramptimer;
   }
  }
}
