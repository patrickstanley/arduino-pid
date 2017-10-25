# Arduino PID Relay Furnace Controller with Serial Connectivity

The purpose of this Arduino sketch is to power a heating furnace (controlled by a relay), with PID controls.
In addition serial communications are added to monitor, log, and interact with the controller.

## Hardware and other libraries
This was built to be used on a Arduino Uno R3, but should be easily run on a variety of different boards. For temperature sensing, an [Adafruit MAX31856](https://www.adafruit.com/product/3263) is used with the [library](https://github.com/adafruit/Adafruit_MAX31856) from them. Also used is the PID controler [library by br3ttb](https://github.com/br3ttb/Arduino-PID-Library). All I have really done is put the two together with the ability to communicate over serial.

## Variables
Here are a list of variables for things you may want to change based on your setup.

*   RelayPin: The physical pin your relay is connected to.
*   Adafruit_MAX31856(CS, DI, DO, CLK): Pins for your MAX31856
*   WindowSize: Length of on/off cycle, needed to account for AC power
*   printdelay: Delay time in milliseconds for printout on serial
*   MaxOP: Maximum operating power, used to extend life of elements
*   myPID(&Input, &Output, &workingSet,P,I,D, DIRECT): PID need to be adjusted for your setup. If inverse behavior is experienced (e.g. heats when it should cool) switch DIRECT.
*   workingSet and Setpoint: Inital setpoint for power on
*   ramprate: Sets a ramp rate for the working setpoint. Measured in milliseconds for 1C change.

## Serial communications
Baud rate is set to 9600. Output is tab deliminated of Setpoint, Working Setpoint, Thermocouple Temperature, and Output Power. To change set point, send a new number over serial and press return. You may have to adjust your serial monitor to send '\n' as end of line.
