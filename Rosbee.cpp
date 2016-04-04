/**
*               __
*    _________ / /_  ____  ________  ____________  _____
*   /___/ __ \/ __ \/ __ \/ ___/ _ \/ ___/ ___/ / / / _ \
*  / / / /_/ / /_/ / /_/ / /  /  __(__  ) /__/ /_/ /  __/
* /_/  \____/_.___/\____/_/   \___/____/\___/\__,_/\___/
*
*
* @file Rosbee.cpp
* @date Created: 16-3-2015
* @version 2.0
*
* @author Nathan Schaaphuizen
*
* @section LICENSE
* License: newBSD
*
* Copyright © 2015, HU University of Applied Sciences Utrecht.
* All rights reserved.
*
* Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
* - Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
* - Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
* - Neither the name of the HU University of Applied Sciences Utrecht nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
*
* THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
* "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
* THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
* ARE DISCLAIMED. IN NO EVENT SHALL THE HU UNIVERSITY OF APPLIED SCIENCES UTRECHT
* BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
* CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
* GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
* HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
* LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
* OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
**/

#include <simpletools.h>
#include "Qik.h"
#include "Encoder.h"
#include "Uart.h"

/// Entry point of application.
int main(){
  //Pin connected to the rx pin of the Qik motor controller.
  int qTx = 25;
  //Pin connected to the tx pin of the Qik motor controller.
  int qRx = 26;
  //Baud rate used to communicate with the Qik motor controller.
  int qBaud = 115200;
  
  //Pin connected to the photointerrupter of the encoder on the left motor.
  int leftWheel = 0;
  //Pin connected to the photointerrupter of the encoder on the right motor.
  int rightWheel = 3;
  
  //Uart object for communication.
  Uart uart;
  //Qik object for motor control.
  Qik qik{qTx,qRx,qBaud};
  //Stop  motor 1.
  //This is done so the rosbee won't drive away and/or stop while the 
  //program is rebooted.
  qik.setMotorSpeed(Qik::Motor::M0,0);
 
  //Stop  motor 2.
  //This is done so the rosbee won't drive away and/or stop while the 
  //program is rebooted.
  qik.setMotorSpeed(Qik::Motor::M1,0);
  
  //Encoder object for the left motor.
  int side = 0;
  Encoder enc0{leftWheel, side};
  print("after encoder init \n");
  //Encoder object fot the right motor.
  side = 1;
  Encoder enc1{rightWheel, side};

  
  //Variables used for communcation.
  //cmd = command byte received.
  //value = follow byte received.
  //rtn = byte to be send.
  //intRtn = int(4 bytes) to be send.
  //speed = motor speed (can be negative).
  char cmd, value, rtn;
  int intRtn;
  signed char speed;
  
  
  /*
   *
   *  TEST CODE
   *
   */
  if (1) {
  // Sleep 5 seconds before starting test
  pause(5000);
  // Using the encoder count of one specific Rosbee, might be different for others
  int totalCounts360WheelTurn = 3000; // In encoder counts
  int wheelCircumference = 386; // In mm
  
  // Keep track of amount of encoder ticks
  int startEncoderCountenc0 = enc0.getEncoderCount();
  int startEncoderCountenc1 = enc1.getEncoderCount();
  int enc0Count = 0;
  int enc1Count = 0;
  int encoderCount = 0;
  
  // Motor power
  int motorPower = 30;
  
  // 3m forward
  int distanceToGo = 4000; // In mm
  
  // Target encoder count
  float encoderCountTarget = totalCounts360WheelTurn * ((float)distanceToGo / wheelCircumference);
  
  // Turn both left and right motors on at a given power
  qik.setMotorSpeed(Qik::Motor::M0, motorPower);
  qik.setMotorSpeed(Qik::Motor::M1, motorPower);
  
  // While robot hasn't reached the distanceToGo
  while(encoderCount < encoderCountTarget) {
    int encoderCountenc0 = enc0.getEncoderCount() - startEncoderCountenc0;
    int encoderCountenc1 = enc1.getEncoderCount() - startEncoderCountenc1;
    
    //if the speed of enc0 is less then enc1 the motor power needs to be a bit faster
    if(enc0.getSpeed() < enc1.getSpeed()){
        //drive a bit faster
       qik.setMotorSpeed(Qik::Motor::M0, motorPower++);
    }      
    
    //else if the speed of enc0 is more then enc1 the motor power needs to be a bit slower
    else if(enc0.getSpeed() > enc1.getSpeed()){
      //drive a bit slower
       qik.setMotorSpeed(Qik::Motor::M0, motorPower--);
    }
    encoderCount = (encoderCountenc0 + encoderCountenc1) / 2;
    //print("enc1: %d and enc2: %d\n",  enc0.getSpeed(), enc1.getSpeed());
    pause(50);
  } 
  
  // Turn both left and right motors off
  qik.setMotorSpeed(Qik::Motor::M0, 0);
  qik.setMotorSpeed(Qik::Motor::M1, 0);
  //brake as hard as possible
  qik.setBrakePower(Qik::Motor::M0, 127);
  qik.setBrakePower(Qik::Motor::M1, 127); 
  
  pause(5000);
  
  
  
  // Sleep for 20 seconds to give us some time to measure 
  //pause(20000);
  
  // Reset the start encoder count
  startEncoderCountenc0 = enc0.getEncoderCount();
  startEncoderCountenc1 = enc1.getEncoderCount();
  encoderCount = 0;
  
  // Distance for one circle of Rosbee
  distanceToGo = 1037; // In mm
  
  // Target encoder count
  encoderCountTarget = totalCounts360WheelTurn * ((float)distanceToGo / wheelCircumference);
  
  // Turn left and right motors on with an opposite motorPower so the robot will drive in a circle
  qik.setMotorSpeed(Qik::Motor::M0, motorPower);
  qik.setMotorSpeed(Qik::Motor::M1, -motorPower); 
     
  // While robot hasn't reached the distanceToGo   
  while(encoderCount < encoderCountTarget) {
    int encoderCountenc0 = enc0.getEncoderCount() - startEncoderCountenc0;
    int encoderCountenc1 = enc1.getEncoderCount() - startEncoderCountenc1;
    encoderCount = (encoderCountenc0 + encoderCountenc1) / 2;            
  } 
   
  // Turn both left and right motors off
  qik.setMotorSpeed(Qik::Motor::M0, 0);
  qik.setMotorSpeed(Qik::Motor::M1, 0);
  
  /*
   *
   * END TEST CODE
   *
   */
}   
  
  
  int totalCountsFor360WheelTurn = 3300;
  int countChangeStepSize = 100;
  int wheelPower = 30;
  
  qik.setBrakePower(Qik::Motor::M0, 127);
  qik.setBrakePower(Qik::Motor::M1, 127); 

  
  //Run forever.
  //The rosbee is expected to work as long as it has power.
  //Therefore  this loop never needs to end.
  while(true){
    //Get the command byte.
    //This will block if no byte is available.
    cmd = uart.readChar();
    //Check which command to execute.
    //This is just a epic long switch case.
    //There was honestly no better way to do this that does
    //not require making infinite classes.
    switch(cmd){
      //Debug
      //Commands regarding debugging.
      case 'p':
      print("pulseCount = %u\n", enc1.getEncoderCount());
        break;
      case 'l':
      print("direction = %d\n", enc0.getDirection());
        break;
      case 'r':
      print("direction = %d\n", enc1.getDirection());
        break;
      case 'i':
      print("test\n");
        break;
      case 'j':
        qik.setMotorSpeed(Qik::Motor::M1,100);
        break;
         case 'b':
        qik.setMotorSpeed(Qik::Motor::M1,0);
        break;
        
      case '+':
        totalCountsFor360WheelTurn = totalCountsFor360WheelTurn + countChangeStepSize;
        print("Total: %d\n", totalCountsFor360WheelTurn);
        break;
      case '-':
        totalCountsFor360WheelTurn = totalCountsFor360WheelTurn - countChangeStepSize;
        print("Total: %d\n", totalCountsFor360WheelTurn);
        break;
      case '*':
        countChangeStepSize = countChangeStepSize * 10;
        print("Count change step size: %d\n", countChangeStepSize);          
        break;
      case '/':
        countChangeStepSize = countChangeStepSize / 10;
        if (countChangeStepSize < 1) {
          countChangeStepSize = 1; 
        }
        print("Count change step size: %d\n", countChangeStepSize);          
        break;
      case '[': {
        print("Starting M0 at %d for %d\n", wheelPower, totalCountsFor360WheelTurn);
        int encoderCountEncStart = enc0.getEncoderCount();
        //int currentLoops = enc0.getLoops();
        qik.setMotorSpeed(Qik::Motor::M0, wheelPower);
        while (enc0.getEncoderCount() < (totalCountsFor360WheelTurn + encoderCountEncStart)) {
        }
        //print("Loops: %d\n", enc0.getLoops() - currentLoops);   
        qik.setMotorSpeed(Qik::Motor::M0, 0); 
        qik.setBrakePower(Qik::Motor::M0,100);
        break;
      }        
      case ']': {
        print("Starting M1 at %d for %d\n", wheelPower, totalCountsFor360WheelTurn);
        int encoderCountEncStart = enc1.getEncoderCount();
       // int currentLoops = enc1.getLoops();
        qik.setMotorSpeed(Qik::Motor::M1, wheelPower);
        while (enc1.getEncoderCount() < (totalCountsFor360WheelTurn + encoderCountEncStart)) {
        }   
        //print("Loops: %d\n", enc1.getLoops() - currentLoops);  
        qik.setMotorSpeed(Qik::Motor::M1, 0); 
        qik.setBrakePower(Qik::Motor::M1,100);
        break;
      }
      case ',':
        wheelPower = wheelPower - 1;
        print("Wheelpower: %d\n", wheelPower);
        break;  
      case '.':
        wheelPower = wheelPower + 1;
        print("Wheelpower: %d\n", wheelPower);
        break;      
      
    } // End switch.             
  } // End while.
  //The program should never come here, but it's required by the compiler.
  return 0;
}    