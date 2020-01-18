/*
   -- Scooter Freedom --

   This source code of graphical user interface
   has been generated automatically by RemoteXY editor.
   To compile this code using RemoteXY library 2.4.3 or later version
   download by link http://remotexy.com/en/library/
   To connect using RemoteXY mobile app by link http://remotexy.com/en/download/
     - for ANDROID 4.3.1 or later version;
     - for iOS 1.3.5 or later version;

   This source code is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
*/

//////////////////////////////////////////////
//        include libraries                 //
//////////////////////////////////////////////

// RemoteXY select connection mode and include library
#define REMOTEXY_MODE__ESP32CORE_BLE

#include "RemoteXY.h"
#include "FastCRC.h"
#include <Arduino.h>

// RemoteXY connection settings
#define REMOTEXY_BLUETOOTH_NAME "Scooter Rebellion"

// RemoteXY configurate
#pragma pack(push, 1)

uint8_t RemoteXY_CONF[] =
  { 255,2,0,3,0,65,0,8,216,1,
  2,1,30,19,22,11,233,26,31,31,
  79,78,0,79,70,70,0,65,4,10,
  20,9,9,65,3,10,44,9,9,3,
  131,30,44,22,8,232,26,129,0,6,
  73,52,6,205,83,99,111,111,116,101,
  114,32,82,101,98,101,108,108,105,111,
  110,0 };

// this structure defines all the variables of your control interface
struct {

    // input variable
  uint8_t switch_1; // =1 if switch ON and =0 if OFF
  uint8_t select_1; // =0 if select position A, =1 if position B, =2 if position C, ...

    // output variable
  uint8_t led_1_r; // =0..255 LED Red brightness
  uint8_t led_2_g; // =0..255 LED Green brightness
  uint8_t led_2_b; // =0..255 LED Blue brightness

    // other variable
  uint8_t connect_flag;  // =1 if Bluetooth is connected, else =0

} RemoteXY;
#pragma pack(pop)

/////////////////////////////////////////////
//           END RemoteXY include          //
/////////////////////////////////////////////

#define PIN_SWITCH_1 13

////////////////////////////////////////////
//         Scooter Starter include        //
///////////////////////////////////////////
#define INTERVAL 1000
unsigned long start_time = 0;
int run_once = 0;

byte messageOff = 0x10;
byte messageStart = 0x15;
byte lightOn = 0xE5;
byte lightFlashing = 0xE3;
byte lightOff = 0xE1;

////////////////////////////////////////////
//         Scooter Command include        //
///////////////////////////////////////////
FastCRC8 CRC8; //Checksum Library
uint32_t crc;

//byte commandByte = 0; // for incoming BLE data
byte commandbyteOld = 0; // last incoming BLE data

uint8_t buf[] = {0xA6, 0x12, 0x02, 0x00, 0xE4, 0x00};

#define BUFSIZE 5

void commandSent(byte commandByte)
{
    if (commandByte != commandbyteOld)
    {
     buf[3] = commandByte;
     crc = CRC8.maxim(buf, BUFSIZE); //CRC-8 MAXIM Check Sum Calculator
     buf[5] = crc;
     commandbyteOld = commandByte;
     Serial.println("CRC calculated");
   }
}


void setup()
{
  RemoteXY_Init ();

  Serial.begin(115200);
  Serial1.begin(9600);

  pinMode (PIN_SWITCH_1, OUTPUT);

  commandSent(messageOff);
  delay(500);
  commandSent(messageStart);
  delay(500);
  digitalWrite(PIN_SWITCH_1, (RemoteXY.switch_1==0)?LOW:HIGH); //This line had been in the loop() moved it here for noise.

}

void loop()
{
  RemoteXY_Handler ();



   if(RemoteXY.switch_1 == 1){

     switch(RemoteXY.select_1){ //Headlight switch and indicator
      case 0:
       RemoteXY.led_2_b = 0;
       RemoteXY.led_2_g = 0;
       commandSent(lightOff);
       break;
      case 1:
        RemoteXY.led_2_b = 255;
        RemoteXY.led_2_g = 0;
        commandSent(lightOn);
        break;
      case 2:
        RemoteXY.led_2_b = 0;
        RemoteXY.led_2_g = 255;
        commandSent(lightFlashing);
        break;
      }

    if(millis() > start_time + INTERVAL){
        start_time = millis();
        Serial1.write(buf, sizeof(buf));
        run_once = 0;
      }
    RemoteXY.led_1_r = 255;
    }
    else{
      RemoteXY.led_1_r = 0;
      if (run_once == 0){
      commandSent(messageOff);
      Serial1.write(buf, sizeof(buf));
      run_once = 1;
      }
    }
   /*Serial.print(RemoteXY.switch_1);
   Serial.print(" ");
   Serial.print(RemoteXY.select_1);
   Serial.print(" ");*/
   Serial.print(buf[3], HEX);
   Serial.print(" ");
   Serial.println(RemoteXY.connect_flag);
}
