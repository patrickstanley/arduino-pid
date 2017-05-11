#include <Adafruit_MAX31856.h>
#include <PID_v1.h>
#define RelayPin 2

const byte numChars = 32;
char receivedChars[numChars]; // an array to store the received data

boolean newData = false;

// Use software SPI: CS, DI, DO, CLK
Adafruit_MAX31856 max = Adafruit_MAX31856(4, 5, 6, 7);

//Define Variables we'll be connecting to
double workingSet, Setpoint, Input, Output;
int WindowSize = 1000;
unsigned long prevprintMillis = 0;
const long printdelay = 5000;
unsigned long windowStartTime;
unsigned long prevramptimer = 0;

//Specify the links and initial tuning parameters
PID myPID(&Input, &Output, &workingSet,250,50,8, DIRECT);

void setup() {
  pinMode(RelayPin, OUTPUT);
  Serial.begin(9600);
  Serial.println("PID Controller Test");
  Serial.println("Setpoint\t Working SP\t TC Temp\t Output");
  
  pinMode(RelayPin, OUTPUT);

  max.begin();
  max.setThermocoupleType(MAX31856_TCTYPE_K);

  windowStartTime = millis();
  
  //initialize the variables we're linked to
  workingSet = 20;
  Setpoint = 20;

  //tell the PID to range between 0 and the full window size
  myPID.SetOutputLimits(0, WindowSize);
  //turn the PID on
  myPID.SetMode(AUTOMATIC);

}

void recvWithEndMarker() {
 static byte ndx = 0;
 char endMarker = '\n';
 char rc;
 
 // if (Serial.available() > 0) {
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
  recvWithEndMarker();
  showNewData();
 
  Input = max.readThermocoupleTemperature();
  myPID.Compute();

//print delay for serial output
  unsigned long printMillis = millis();
  if (printMillis - prevprintMillis >= printdelay) {
    prevprintMillis = printMillis;
    Serial.print(Setpoint);Serial.print("\t");Serial.print(workingSet);Serial.print("\t"); Serial.print(max.readThermocoupleTemperature());Serial.print("\t");Serial.println(Output/WindowSize*100);
  }

  
//turn the output pin on/off based on pid output
 if(millis() - windowStartTime>WindowSize)
  { //time to shift the Relay Window
    windowStartTime += WindowSize;
  }
 if(Output < millis() - windowStartTime) digitalWrite(RelayPin,LOW);
  else digitalWrite(RelayPin,HIGH);

//sets ramp rate functionality
unsigned long ramptimer = millis();
unsigned long ramprate = 6000;//milliseconds for 1C change
  if(workingSet != Setpoint)
  {
   if(ramptimer - prevramptimer >= ramprate && workingSet < Setpoint) 
   {
    workingSet++;
    prevramptimer = ramptimer;
   }
   if(ramptimer - prevramptimer >= ramprate && workingSet > Setpoint) 
   {
    workingSet--;
    prevramptimer = ramptimer;
   }
  }
}

