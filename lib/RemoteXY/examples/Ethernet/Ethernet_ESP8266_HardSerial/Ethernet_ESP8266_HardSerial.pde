/*
  RemoteXY example. 
  Smartphone Ethernet connect over ESP8266 Wi-Fi module 
  (hardware serial connected).

  This shows an example of using the library RemoteXY.
  In the example you can control the LED pin 13 using the button on the 
  smartphone. You need to connect Wi-Fi module ESP8266 
  specified contacts: 
  Pin 0(RX) ---  TX ESP8266 module           
  Pin 1(TX) ---  RX ESP8266 module    
  ESP8266 must be 115200 baud rate
  
  Download the mobile app from the 
  website: http://remotexy.com/download/ for connect this sketch.
  
  Use the website http://remotexy.com/ to create your own management 
  interface your arduino with your smartphone or tablet.
  You can create different management interfaces. Use buttons, 
  switches, sliders, joysticks (g-sensor) all colors and sizes 
  in its interface. Next, you will be able to get the sample 
  code for arduino to use your interface for control from a 
  smartphone or tablet. You will not need to re-install the 
  android app, as it will determine which interface you have 
  downloaded the arduino.
  
*/

///////////////////////////////////////////// 
//        RemoteXY include library         // 
///////////////////////////////////////////// 

/* RemoteXY select connection mode and include library */ 
#define REMOTEXY_MODE__ESP8266_HARDSERIAL 
#include <RemoteXY.h> 

/* RemoteXY connection settings */ 
#define REMOTEXY_SERIAL Serial 
#define REMOTEXY_SERIAL_SPEED 115200 
#define REMOTEXY_WIFI_SSID "YOUR_SSID" 
#define REMOTEXY_WIFI_PASSWORD "YOUR_PASS"
#define REMOTEXY_SERVER_PORT 6377

/* RemoteXY configurate  */ 
unsigned char RemoteXY_CONF[] = 
  { 1,0,11,0,1,5,1,0,21,2
  ,59,59,2,88,0 }; 
   
/* this structure defines all the variables of your control interface */ 
struct { 

    /* input variable */
  unsigned char button_1; /* =1 if button pressed, else =0 */

    /* other variable */
  unsigned char connect_flag;  /* =1 if wire connected, else =0 */

} RemoteXY; 

///////////////////////////////////////////// 
//           END RemoteXY include          // 
///////////////////////////////////////////// 

#define PIN_BUTTON_1 13


void setup()  
{ 
  RemoteXY_Init ();  
   
  pinMode (PIN_BUTTON_1, OUTPUT);
   

  // TODO you setup code 
   
} 

void loop()  
{  
  RemoteXY_Handler (); 
   
  digitalWrite(PIN_BUTTON_1, (RemoteXY.button_1==0)?LOW:HIGH);
   

  // TODO you loop code 
  // use the RemoteXY structure for data transfer 


}