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

#include "../include/Encoder.h"
#include <simpletools.h>

Encoder::Encoder(int newPin, int side):
pin1{newPin}, 
side{side}
{   
  //Start execution of code on a new cog.
  //We can not directly call a member function.
  //So we pass the pointer of this object as the parameter to achieve the same.
  cogId = cogstart(&Encoder::run, this, stack,  sizeof(Encoder)); 
}

Encoder::~Encoder(){
  //Stop the cog execution and free it for other purposes.
  cogstop(cogId); 
}   
 
unsigned int Encoder::getEncoderCount(){
  //Return the amount of counted pulses.
  return pulseCount; 
}

int Encoder::getDirection(){
  //Return the direction which the encoder is turning. 
  return direction; 
}
 
int Encoder::getSpeed(){
  //Return the amount of counted pulses per second.
  return speed;  
}

void Encoder::run(void* obj){
  //Since we are sure the object is of type Encoder we can safely cast it back to a Encoder pointer. 
  Encoder* enc = reinterpret_cast<Encoder*>(obj);
  enc->direction = -1;
  int oldDirection = -1;
  //algoritme to set the pins to that for both wheels forwards and backwards is the same way. 
  if(enc->side == 0){
    enc->pin2 = enc->pin1 + 1;
  } 
  else if(enc->side == 1){
     enc->pin2 = enc->pin1 - 1;
  }
  
  //Var contains the data what was read from the pin.     
  int readpin1, readpin2;
  
  //Var contains the data from last reading.
  //We use this to make a comparison to the new reading.
  //To make sure we have the correct start value we already make a reading here. 
  int readlast1 = input(enc->pin1);  
  int readlast2 = input(enc->pin2);  
  
  //Var used to count the time for speed.
  int count = 0;
  
  //Var used to the calculate the speed.
  int lastSpeed = 0;
  
  //Var used to count the time for if the speed is ZERO. 
  int timer = 0;
  
  //Start the reading, this will never end.
  while(true){
    //Read the state of the pin.
    //1 = endocer blocked
    //0 = encoder can see the led
    readpin1 = input(enc->pin1);
    readpin2 = input(enc->pin2);
    
    //Used quadrature encoder.
    //reads two different pins. 
    //if the state of one of the pins changes increase PulseCount.  
    if(readlast1 != readpin1 || readlast2 != readpin2){
      enc->pulseCount++; 
    }
    
    //Used quadrature encoder.
    //reads two different pins. 
    //if both last reads were low[0,0] or both were high[1,1].
    if((readlast1 && readlast2) || (!readlast1 && !readlast2)){
      //if last read was low AND pin is high [0,x] -> [1,x]
      //direction is backwards.  
      if(!readlast1 && readpin1){
        enc->direction = 0;
      }
      //if last read was low AND pin is high [x,0] -> [x,1] 
      //direction is forwards.
      else if(!readlast2 && readpin2){
        enc->direction = 1;
      }               
    }
    
    //if the direction changes pulseCount resets. 
    else if(enc->direction != oldDirection){
      enc->pulseCount = 0;
      oldDirection = enc->direction;
    }
    
    //Update the last pin state with the new.      
    readlast1 = readpin1;
    readlast2 = readpin2;
    
    //Add 1 tick the the time counter.
    //This is equal to 2 ms.
    count++;
    //Check if we are 500 tick further.
    //This is equal to 1 sec.
    if(count >= 500){
       //Restet the counter to 0.
       //So we can begin counting a new second.
       count = 0;
       //Update the speed.
       //The speed is equal to the current pulse count minus the pulse count a second ago.
       enc->speed = (enc->pulseCount - lastSpeed);
       //Update the pulse count from a second ago with the current pulse count.
       lastSpeed = enc->pulseCount; 
    }
    
    //count times how long the encoder has speed ZERO
    if (enc->speed == 0){
      timer ++;   
    } 
    else {
      timer = 0; 
    }  
    //if encoder speed is ZERO for 250 milliseconds.
    //pulseCount to ZERO. 
    if (timer >= 125){
      enc->pulseCount = 0;  
    }         
    //Wait 2000 microseconds aka 2 milliseconds.
    waitcnt(CNT + 20 * us);      
  }   
} 