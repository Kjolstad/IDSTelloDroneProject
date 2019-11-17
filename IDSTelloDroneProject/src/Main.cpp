#include <Arduino.h>
#include <WiFi.h>
#include <WiFiUdp.h>
#include <TelloDrone.h>
#include <string>
#include <Wire.h>
#include <MPU6050_tockn.h>
#include <sstream>

MPU6050 mpu6050(Wire);

TelloDrone drone;

// These are the pin variables for the buttons.
// Making variables makes it easier to change throughout the code.
// Mainly used while testing, while it works there's no need to change them.
int btnCmdPin = 4;
int btnTakeoffPin = 18;

// These are the variables for the linear potentiometer.
int sliderPin = 33;
int sliderUpDown = 0;

// These are the variables for the Gyro.
float gyroLeftRight = 0.0;
float gyroFwdBwd = 0.0;

void sendCommand(string cmd)
{
  Serial.print("Executing command: ");
  Serial.println(cmd.c_str());
  drone.sendCommand(cmd);   
  Serial.println(drone.getResponse().c_str());
}

void SlidePot()
{
  // Data from Slider goes from 0 to 4095
  // Down < 0-1400 < deadzone < 2400-4095 up

  // Here I'm controlling what the linear potentiometer should do,
  // If the value from the analog pin is past certain thresholds, it should act accordingly.
 
  if (analogRead(sliderPin) > 3000 && analogRead(sliderPin) < 4000 && analogRead(sliderPin) > 2400)
  {
    sliderUpDown = 50;

  } else if (analogRead(sliderPin) > 4000)
  {
    sliderUpDown = 100;
  } else if (analogRead(sliderPin) < 1400 && analogRead(sliderPin) > 0 && analogRead(sliderPin) < 2400)
  {
    sliderUpDown = -50;

  } else if (analogRead(sliderPin) < 20)
  {
    sliderUpDown = -100;
  } else 
  {
    sliderUpDown = 0;
  }
}

void Buttons()
{
  // Here we have 2 buttons, one for the command, and one for takeoff.

  if (digitalRead(btnCmdPin) == HIGH)
  {
    sendCommand("command");
  }

  if (digitalRead(btnTakeoffPin) == HIGH)
  {
    sendCommand("takeoff");
  }
}


void GyroData() 
{
  //TODO, Get Gyro data to variables, and be able to calculate an offset
    mpu6050.update();
    float gyroX = mpu6050.getAngleX();
    float gyroY = mpu6050.getAngleY();


  // Two have a sense of control over the very delicate Gyroscope.
  // I use the following code, to only change the directions with increments of 25.
  // This also makes sure that if there's a spike in the gyrodata, it doesn't fly off into space.
  if (gyroX > 20 && gyroX < 50)
  {

    gyroLeftRight = 25;

  } else if (gyroX > 50)
    {
    gyroLeftRight = 25;
    } else if (gyroX < 20 && gyroX > -20)
      {
        gyroLeftRight = 0;
      } else if (gyroX < -20 && gyroX > -50)
        {
          gyroLeftRight = -25;
    
        } else if (gyroX < -50)
          {
            gyroLeftRight = -25;
          }

  if (gyroY > 20 && gyroY < 50)
    {

      gyroFwdBwd = 25;

  } else if (gyroY > 50)
    {
      gyroFwdBwd = 25;
    } else if (gyroY < 20 && gyroY > -20)
      {
        gyroFwdBwd = 0;

      } else if (gyroY < -20 && gyroY > -50)
        {
          gyroFwdBwd = -25;
    
        } else if (gyroY < -50)
          {
            gyroFwdBwd = -25;
          }



}


void setup(){
  // Initilize hardware serial:
  // TODO, maybe I should make it 11550 baud rate?
  Serial.begin(9600);
  drone.connect("TELLO-FE3067", "");
  Wire.begin();

  // GYRO Pins: SCL= 22, SDA= 21
  mpu6050.begin();
  mpu6050.calcGyroOffsets(true);

  //Pins for the Slider
  // 33
  pinMode(sliderPin, INPUT);

  // Pins for the 2 buttons, 4, 18
  pinMode(btnCmdPin, INPUT_PULLDOWN);
  pinMode(btnTakeoffPin, INPUT_PULLDOWN);

}


void loop(){

  
  // Gyro Data
    GyroData();

  // Buttons Data
    Buttons();

  // Slidepot Data
    SlidePot();

  //only send data when drone is connected to wifi
  // Command sent to the drone has to be:
  // "rc left/right forward/backward up/down yaw"
  // (-100 - 100)
    if(drone.connected)
  {
    // Here I'm converting the Float values of the gyro to string values, 
    // so they can be send with a command.
    stringstream gLR;
    gLR << (int)gyroLeftRight;
    string gyroLR = gLR.str();

    stringstream gFB;
    gFB << (int)gyroFwdBwd;
    string gyroFB = gFB.str();

    stringstream sUD;
    sUD << sliderUpDown;
    string sliderUD = sUD.str();

    stringstream gY;
    gY << 0;
    string gyroY = gY.str();

    string rCCommand;
    string prevRCCommand;
    
    rCCommand = "rc " + gyroLR + " " + gyroFB + " " + sliderUD + " " + gyroY;

    
    sendCommand(rCCommand);

  }

  delay(100);
}



