////
//  ██████╗  ██████╗ ██████╗  ██████╗ ██████╗ ███████╗███████╗ ██████╗██╗   ██╗███████╗
//  ██╔══██╗██╔═══██╗██╔══██╗██╔═══██╗██╔══██╗██╔════╝██╔════╝██╔════╝██║   ██║██╔════╝
//  ██████╔╝██║   ██║██████╔╝██║   ██║██████╔╝█████╗  ███████╗██║     ██║   ██║█████╗  
//  ██╔══██╗██║   ██║██╔══██╗██║   ██║██╔══██╗██╔══╝  ╚════██║██║     ██║   ██║██╔══╝  
//  ██║  ██║╚██████╔╝██████╔╝╚██████╔╝██║  ██║███████╗███████║╚██████╗╚██████╔╝███████╗
//  ╚═╝  ╚═╝ ╚═════╝ ╚═════╝  ╚═════╝ ╚═╝  ╚═╝╚══════╝╚══════╝ ╚═════╝ ╚═════╝ ╚══════╝
//                                                                                                                                          
//
// @file Rosbee.cpp
// @date Created: 16-03-2015
// @version 3.0
//
// @author Nathan Schaaphuizen, Robert Zegers, Kevin Damen
//
// @section LICENSE
// License: newBSD
//
// Copyright © 2016, HU University of Applied Sciences Utrecht.
// All rights reserved.
//
// Redistribution and use in source and binary forms, with or without modification, are permitted provided that the following conditions are met:
// - Redistributions of source code must retain the above copyright notice, this list of conditions and the following disclaimer.
// - Redistributions in binary form must reproduce the above copyright notice, this list of conditions and the following disclaimer in the documentation and/or other materials provided with the distribution.
// - Neither the name of the HU University of Applied Sciences Utrecht nor the names of its contributors may be used to endorse or promote products derived from this software without specific prior written permission.
//
// THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
// "AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO,
// THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
// ARE DISCLAIMED. IN NO EVENT SHALL THE HU UNIVERSITY OF APPLIED SCIENCES UTRECHT
// BE LIABLE FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR
// CONSEQUENTIAL DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE
// GOODS OR SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION)
// HOWEVER CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
// LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT
// OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
////

#include <simpletools.h>
#include "../include/Qik.h"
#include "../include/Encoder.h"
#include "../include/Uart.h"

/// Entry point of application.
int main(){
	//Pin connected to the rx pin of the Qik motor controller.
	int qTxpin = 25;
	//Pin connected to the tx pin of the Qik motor controller.
	int qRxpin = 26;
	//Baud rate used to communicate with the Qik motor controller.
	int qBaud = 115200;
  
	//Pin connected to the photointerrupter of the encoder on the left motor.
	int leftWheel = 0;
	//Pin connected to the photointerrupter of the encoder on the right motor.
	int rightWheel = 3;
  
	//side of the encoder. 0 is left, 1 is right.
	int side;
  
	//value to change the value of the motor. 
	int countChangeStepSize = 100;
  
	//Uart object for communication.
	Uart uart;
	//Qik object for motor control.
	Qik qik{qTxpin,qRxpin,qBaud};
	//Stop  motor 0.
	//This is done so the rosbee won't drive away and/or stop while the 
	//program is rebooted.
	qik.setMotorSpeed(Qik::Motor::M0,0);
 
	//Stop  motor 1.
	//This is done so the rosbee won't drive away and/or stop while the 
	//program is rebooted.
	qik.setMotorSpeed(Qik::Motor::M1,0);
  
	//Encoder object for the left motor.
	side = 0;
	Encoder enc0{leftWheel, side};
	//Encoder object fot the right motor.
	side = 1;
	Encoder enc1{rightWheel, side};

	//Variables used for communcation.
	//cmd = command byte received.
	//value = follow byte received.
	//rtn = byte to be send.
	//intRtn = int(4 bytes) to be send.
	//speed = motor speed (can be negative).
	char cmd, value, rtn;           //value not used!!! rtn not used!!! 
	int intRtn;                     //can't find where this is used!!!!
	signed char speed;              //can't find where this is used!!!!
  
  /*\
   *
   *  TEST CODE
   *
  \*/
	//Encoder counts for 1 wheel turn for the rosbee. 
	//for another wheel needs other value. 
	int totalCounts360WheelTurn = 3000; // In encoder counts
	//circumference of the wheel of the rosbee in mm. 
	int wheelCircumference = 386; // In mm
	//Motor power
	int motorPower = 30;
	if (1) {
		//Sleep 5 seconds before starting test
		pause(5000);
  
 
  
		//Keep track of amount of encoder ticks
		int startEncoderCountenc0 = enc0.getEncoderCount();
		int startEncoderCountenc1 = enc1.getEncoderCount();
		int encoderCount = 0;

  
		//4 meters forward
		int distanceToGo = 4000; // In mm
  
		//Target encoder count
		float encoderCountTarget = totalCounts360WheelTurn * 
				((float)distanceToGo / wheelCircumference);
  
		//Turn both motors (left and right) on at the given power
		qik.setMotorSpeed(Qik::Motor::M0, motorPower);
		qik.setMotorSpeed(Qik::Motor::M1, motorPower);
  
		//Drive in a straight line. 
		//While robot hasn't reached the distanceToGo
		while(encoderCount < encoderCountTarget) {
			int encoderCountenc0 = enc0.getEncoderCount() - startEncoderCountenc0;
			int encoderCountenc1 = enc1.getEncoderCount() - startEncoderCountenc1;

			//if the speed of enc0 is less then enc1,
			//the motor power needs to be a bit faster
			if(enc0.getSpeed() < enc1.getSpeed()){
				//drive a bit faster
				qik.setMotorSpeed(Qik::Motor::M0, motorPower++);
			}      
    
			//else if the speed of enc0 is more then enc1 
			//the motor power needs to be a bit slower
			else if(enc0.getSpeed() > enc1.getSpeed()){
				//drive a bit slower
				qik.setMotorSpeed(Qik::Motor::M0, motorPower--);
			}
    
			//Average of the values of both encoders. 
			encoderCount = (encoderCountenc0 + encoderCountenc1) / 2;
			//Sleep for 50 milliseconds. 
			pause(50);
		} 
  
		//Turn both (left and right) motors off
		qik.setMotorSpeed(Qik::Motor::M0, 0);
		qik.setMotorSpeed(Qik::Motor::M1, 0);
  
		//brake as hard as possible
		qik.setBrakePower(Qik::Motor::M0, 127);
		qik.setBrakePower(Qik::Motor::M1, 127); 
  
		//Sleep for 5 seconds to do the next step of the test code. 
		pause(5000);
  
		//reset encoderCount
		encoderCount = 0;
  
		//Distance for the rosbee to turn 360 degree.
		distanceToGo = 1037; // In mm
  
		//Target encoder count
		encoderCountTarget = totalCounts360WheelTurn * 
					((float)distanceToGo / wheelCircumference);
  
		//Turn left and right motors on with an opposite 
		//motorPower so the robot will drive in a circle.
		qik.setMotorSpeed(Qik::Motor::M0, motorPower);
		qik.setMotorSpeed(Qik::Motor::M1, -motorPower); 
  
		//Reset the start encoder count
		startEncoderCountenc0 = enc0.getEncoderCount();
		startEncoderCountenc1 = enc1.getEncoderCount();
  
		//Drive around it's axis. 
		//While robot hasn't reached the distanceToGo   
		while(encoderCount < encoderCountTarget) {
			int encoderCountenc0 = enc0.getEncoderCount()- startEncoderCountenc0;
			int encoderCountenc1 = enc1.getEncoderCount() - startEncoderCountenc1;
			//Average of the values of both encoders.
			encoderCount = (encoderCountenc0 + encoderCountenc1) / 2;         
		} 
   
		//Turn both left and right motors off
		qik.setMotorSpeed(Qik::Motor::M0, 0);
		qik.setMotorSpeed(Qik::Motor::M1, 0);

		//Brake as hard as possible  
		qik.setBrakePower(Qik::Motor::M0, 127);
		qik.setBrakePower(Qik::Motor::M1, 127); 
	}   

  /*\
   *
   *  END TEST CODE
   *
  \*/ 


  /*\
   *
   *  EASY INPUT CODE
   *  this code is to give commands to the rosbee.
   *  To set speed or set motor on or off.
   *  easy code to debug the values. 
   *
  \*/


  
	//Run forever.
  //The rosbee is expected to work as long as it has power.
  //Therefore this loop never needs to end.
	while(true){
		//Get the command byte.
		//This will block if no byte is available.
		cmd = uart.readChar();
    
		//Check which command to execute.
		//This is just a epic long switch case.
		//There was honestly no better way to do this that does
		//not require making infinite classes.
		switch(cmd){
			//Commands regarding debugging.
			case 'p': //print pulseCount of encoder 1. 
				print("pulseCount = %u\n", enc1.getEncoderCount());
				break;
			case 's': //print speed of encoder 1.
				print("Speed = %u\n", enc1.getSpeed());
				break;
			case 'l': //print direction of encoder 0 (LEFT).
				print("direction = %d\n", enc0.getDirection());
				break;
			case 'r': //print direction of encoder 1 (RIGHT).
				print("direction = %d\n", enc1.getDirection());
				break;
			case 'i': //print "TEST". check if read command works. 
				print("TEST\n");
				break;
			case 'j': //rotate motor 1 with power 30. 
				qik.setMotorSpeed(Qik::Motor::M1, 30);
				break;
			case 'b': //rotate motor 0 with power 0. so the motor doesn't rotate. 
				qik.setMotorSpeed(Qik::Motor::M1, 0);
				break;
        
			case '+': //Increase totalCountFor360WheelTurn.
				totalCounts360WheelTurn = totalCounts360WheelTurn + 
									countChangeStepSize;
				print("Total: %d\n", totalCounts360WheelTurn);
				break;
			case '-': //Decrease totalCountFor360WheelTurn.
				totalCounts360WheelTurn = totalCounts360WheelTurn - 
									countChangeStepSize;
				print("Total: %d\n", totalCounts360WheelTurn);
				break;
			case '*': //Multiply CountChangeStepSize by 10.
				countChangeStepSize = countChangeStepSize * 10;
				print("Count change step size: %d\n", countChangeStepSize);          
				break;
			case '/': //Devide CountChangeStepSize by 10.
				countChangeStepSize = countChangeStepSize / 10;
				if (countChangeStepSize < 1) {
					countChangeStepSize = 1; 
				}
				print("Count change step size: %d\n", countChangeStepSize);          
				break;
			case '[': { //Start motor 0
				print("Starting M0 at %d for %d\n", motorPower, 
										totalCounts360WheelTurn);
				int encoderCountEncStart = enc0.getEncoderCount();
				qik.setMotorSpeed(Qik::Motor::M0, motorPower);
				while (enc0.getEncoderCount() < (totalCounts360WheelTurn + 
												encoderCountEncStart)) {
				}  
				qik.setMotorSpeed(Qik::Motor::M0, 0); 
				qik.setBrakePower(Qik::Motor::M0,100);
				break;
			}        
			case ']': { //Start motor 1
				print("Starting M1 at %d for %d\n", motorPower, 
				totalCounts360WheelTurn);
				
				int encoderCountEncStart = enc1.getEncoderCount();
				qik.setMotorSpeed(Qik::Motor::M1, motorPower);
				while (enc1.getEncoderCount() < (totalCounts360WheelTurn + 
												encoderCountEncStart)) {
				}     
				qik.setMotorSpeed(Qik::Motor::M1, 0); 
				qik.setBrakePower(Qik::Motor::M1,100);
				break;
			}
			case ',': //Increase motor Power with 1 
				motorPower = motorPower + 1;
				print("motorPower: %d\n", motorPower);
				break;  
			case '.': //Decrease motor Power with 1
				motorPower = motorPower - 1;
				print("motorPower: %d\n", motorPower);
				break;      
      
		} // End switch.             
	} // End while.
  //The program should never come here, but it's required by the compiler.
  
  /*\
   *
   * END EASY INPUT CODE
   *
  \*/

	return 0;
}    