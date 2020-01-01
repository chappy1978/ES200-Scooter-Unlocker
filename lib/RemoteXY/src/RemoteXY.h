/* RemoteXY.h 
   A RemoteXY Library - Remote device control
   version 2.4.3   
   ===========================================================
   For use RemoteXY library visit website http://remotexy.com
   This website will help you use the library for configuring 
   a remote control from a smartphone or tablet.
   
   This library is free software; you can redistribute it and/or
   modify it under the terms of the GNU Lesser General Public
   License as published by the Free Software Foundation; either
   version 2.1 of the License, or (at your option) any later version.
        
   Supported modes: 
   All boards:
    #define REMOTEXY_MODE__HARDSERIAL                 - direct data transfer via HARDSERIAL
    #define REMOTEXY_MODE__SOFTSERIAL                 - direct data transfer via SOFTSERIAL
    #define REMOTEXY_MODE__ETHERNET                   - data transfer using <ethernet.h> library and open server
    #define REMOTEXY_MODE__ETHERNET_CLOUD             - data transfer using <ethernet.h> library and cloud connection
    #define REMOTEXY_MODE__HARDSERIAL_ESP8266         - data transfer via HARDSERIAL using AT commands of ESP8266 and open server
    #define REMOTEXY_MODE__HARDSERIAL_ESP8266_POINT   - data transfer via HARDSERIAL using AT commands of ESP8266 and open access point with a server
    #define REMOTEXY_MODE__HARDSERIAL_ESP8266_CLOUD   - data transfer via HARDSERIAL using AT commands of ESP8266 and cloud connection
    #define REMOTEXY_MODE__SOFTSERIAL_ESP8266         - data transfer via SOFTSERIAL using AT commands of ESP8266 and open server
    #define REMOTEXY_MODE__SOFTSERIAL_ESP8266_POINT   - data transfer via SOFTSERIAL using AT commands of ESP8266 and open access point with a server
    #define REMOTEXY_MODE__SOFTSERIAL_ESP8266_CLOUD   - data transfer via SOFTSERIAL using AT commands of ESP8266 and cloud connection
    #define REMOTEXY_MODE__WIFI                       - data transfer using wifi.h library and open server
    
   Only ESP8266 boards:
    #define REMOTEXY_MODE__ESP8266CORE_ESP8266WIFI           - data transfer using <esp8266wifi.h> library and open server
    #define REMOTEXY_MODE__ESP8266CORE_ESP8266WIFI_POINT     - data transfer using <esp8266wifi.h> library and open access point with a server
    #define REMOTEXY_MODE__ESP8266CORE_ESP8266WIFI_CLOUD     - data transfer using <esp8266wifi.h> library and cloud connection

   Only ESP32 boards:
    #define REMOTEXY_MODE__ESP32CORE_WIFI                    - data transfer using <wifi.h> library and open server
    #define REMOTEXY_MODE__ESP32CORE_WIFI_POINT              - data transfer using <wifi.h> library and open access point with a server
    #define REMOTEXY_MODE__ESP32CORE_WIFI_CLOUD              - data transfer using <wifi.h> library and cloud connection
    #define REMOTEXY_MODE__ESP32CORE_BLE                     - data transfer using <BLEdevice.h> library

   Parameters depending on the selected mode (for example):
    #define REMOTEXY_SERIAL Serial  // for Hardware Serial
    #define REMOTEXY_SERIAL_SPEED 115200 
    #define REMOTEXY_SERIAL_RX 2   // for Software Serial
    #define REMOTEXY_SERIAL_TX 3   // for Software Serial
    #define REMOTEXY_WIFI_SSID "RemoteXY"  
    #define REMOTEXY_WIFI_PASSWORD "1234567890" 
    #define REMOTEXY_ETHERNET_MAC "DE:AD:BE:EF:EF:ED"  // for Ethernet modules
    #define REMOTEXY_SERVER_PORT 6377 
    #define REMOTEXY_CLOUD_TOKEN "xxxx" // for Cloud
    #define REMOTEXY_CLOUD_SERVER "cloud.remotexy.com" // for Cloud
    #define REMOTEXY_CLOUD_PORT 6376  // for Cloud
    #define REMOTEXY_ACCESS_PASSWORD "1" 

   Debug log info on 115200 (define before include this library):
    #define REMOTEXY__DEBUGLOGS Serial

   = Version history ========================================

   version 2.2.5   
     - support MPIDE;
   version 2.3.1    
     - Support the device access password;
     - Support the cloud server as beta test;
     - Fixed a bug where the length of variables more than 255;
     - Fixed a bug where ESP module reboot and device did not see it;
     - Fixed a bug where the connection was filed and the device 
         did not see it and reconnection is impossible 
   version 2.3.4  
     - Fixed a bug where the length of all input variables more than 256;
     - Fixed a bug where millis() overflow in 50 days;
     - Fixed some bugs;
   version 2.3.5  
     - Fixed some bugs;
   version 2.4.1
     - support ESP32 WiFi and Bluetooth   
   version 2.4.2
     - Fixed some bugs;
   version 2.4.3
     - Fixed some bugs;
          
*/

#ifndef _REMOTEXY_H_
#define _REMOTEXY_H_

//#define REMOTEXY__DEBUGLOGS Serial
#define REMOTEXY__DEBUGLOGS_SPEED 115200


#if defined(REMOTEXY_MODE__HARDSERIAL) || defined(REMOTEXY_MODE__SERIAL) || defined(REMOTEXY_MODE__HC05_HARDSERIAL) 
  #define REMOTEXY_MOD__SERIAL
  #define REMOTEXY_PORT__HARDSERIAL
#elif defined(REMOTEXY_MODE__SOFTSERIAL) || defined(REMOTEXY_MODE__SOFTWARESERIAL) || defined(REMOTEXY_MODE__HC05_SOFTSERIAL)
  #define REMOTEXY_MOD__SERIAL
  #define REMOTEXY_PORT__SOFTSERIAL
#elif defined(REMOTEXY_MODE__HARDSERIAL_ESP8266_POINT) || defined(REMOTEXY_MODE__ESP8266_HARDSERIAL_POINT) || defined(REMOTEXY_MODE__ESP8266POINT_HARDSERIAL)
  #define REMOTEXY_MOD__ESP8266
  #define REMOTEXY_WIFI__POINT
  #define REMOTEXY_PORT__HARDSERIAL
#elif defined(REMOTEXY_MODE__SOFTSERIAL_ESP8266_POINT) || defined(REMOTEXY_MODE__ESP8266_SOFTSERIAL_POINT) || defined(REMOTEXY_MODE__ESP8266POINT_SOFTSERIAL)
  #define REMOTEXY_MOD__ESP8266
  #define REMOTEXY_WIFI__POINT
  #define REMOTEXY_PORT__SOFTSERIAL
#elif defined(REMOTEXY_MODE__HARDSERIAL_ESP8266) || defined(REMOTEXY_MODE__ESP8266_HARDSERIAL)
  #define REMOTEXY_MOD__ESP8266
  #define REMOTEXY_PORT__HARDSERIAL
#elif defined(REMOTEXY_MODE__SOFTSERIAL_ESP8266) || defined(REMOTEXY_MODE__ESP8266_SOFTSERIAL)
  #define REMOTEXY_MOD__ESP8266
  #define REMOTEXY_PORT__SOFTSERIAL
#elif defined(REMOTEXY_MODE__HARDSERIAL_ESP8266_CLOUD) || defined(REMOTEXY_MODE__ESP8266_HARDSERIAL_CLOUD)
  #define REMOTEXY_MOD__ESP8266_CLOUD
  #define REMOTEXY_PORT__HARDSERIAL
  #define REMOTEXY_CLOUD
#elif defined(REMOTEXY_MODE__SOFTSERIAL_ESP8266_CLOUD) || defined(REMOTEXY_MODE__ESP8266_SOFTSERIAL_CLOUD)
  #define REMOTEXY_MOD__ESP8266_CLOUD
  #define REMOTEXY_PORT__SOFTSERIAL
  #define REMOTEXY_CLOUD
#elif defined(REMOTEXY_MODE__ETHERNET) || defined(REMOTEXY_MODE__ETHERNET_LIB) || defined(REMOTEXY_MODE__W5100_SPI)
  #define REMOTEXY_MOD__ETHERNET
#elif defined(REMOTEXY_MODE__ETHERNET_CLOUD) || defined(REMOTEXY_MODE__ETHERNET_LIB_CLOUD)
  #define REMOTEXY_MOD__ETHERNET_CLOUD
  #define REMOTEXY_CLOUD
#elif defined(REMOTEXY_MODE__WIFI) || defined(REMOTEXY_MODE__WIFI_LIB)
  #define REMOTEXY_MOD__WIFI
#elif defined(REMOTEXY_MODE__ESP8266CORE_ESP8266WIFI_POINT) || defined(REMOTEXY_MODE__ESP8266WIFI_LIB_POINT) || defined(REMOTEXY_MODE__ESP8266WIFIPOINT_LIB) 
  #define REMOTEXY_MOD__ESPCORE_WIFI
  #define REMOTEXY_WIFI__POINT
#elif defined(REMOTEXY_MODE__ESP8266CORE_ESP8266WIFI) || defined(REMOTEXY_MODE__ESP8266WIFI_LIB) 
  #define REMOTEXY_MOD__ESPCORE_WIFI
#elif defined(REMOTEXY_MODE__ESP8266CORE_ESP8266WIFI_CLOUD) || defined(REMOTEXY_MODE__ESP8266WIFI_LIB_CLOUD)                
  #define REMOTEXY_MOD__ESPCORE_WIFI_CLOUD 
  #define REMOTEXY_CLOUD
#elif defined(REMOTEXY_MODE__ESP32CORE_WIFI_POINT)  
  #define REMOTEXY_MOD__ESPCORE_WIFI
  #define REMOTEXY_WIFI__POINT
#elif defined(REMOTEXY_MODE__ESP32CORE_WIFI) 
  #define REMOTEXY_MOD__ESPCORE_WIFI
#elif defined(REMOTEXY_MODE__ESP32CORE_WIFI_CLOUD)
  #define REMOTEXY_MOD__ESPCORE_WIFI_CLOUD
  #define REMOTEXY_CLOUD
#elif defined(REMOTEXY_MODE__ESP32CORE_BLE)
  #define REMOTEXY_MOD__ESP32CORE_BLE
#else
  #error RemoteXY mode does not defined or defined error: REMOTEXY_MODE__XXXXXXX 
#endif


#include <inttypes.h> 
#include "classes/RemoteXY_Lib.h"

/*
#if defined(REMOTEXY_PORT__HARDSERIAL) || defined(REMOTEXY__DEBUGLOGS)
  #include <HardwareSerial.h>
#endif 
*/


#if defined(REMOTEXY_MOD__SERIAL) 
  #include "modules/serial.h" 
#elif defined(REMOTEXY_MOD__ESP8266)
  #include "modules/esp8266.h" 
#elif defined(REMOTEXY_MOD__ESP8266_CLOUD)
  #include "modules/esp8266_cloud.h" 
#elif defined(REMOTEXY_MOD__ETHERNET)
  #include "modules/ethernet.h" 
#elif defined(REMOTEXY_MOD__ETHERNET_CLOUD)
  #include "modules/ethernet_cloud.h" 
#elif defined(REMOTEXY_MOD__WIFI)
  #include "modules/wifi.h" 
#elif defined(REMOTEXY_MOD__ESPCORE_WIFI)
  #include "modules/espcore_wifi.h" 
#elif defined(REMOTEXY_MOD__ESPCORE_WIFI_CLOUD)
  #include "modules/espcore_wifi_cloud.h" 
#elif defined(REMOTEXY_MOD__ESP32CORE_BLE)
  #include "modules/esp32core_ble.h" 
#endif 

#ifndef REMOTEXY_ACCESS_PASSWORD 
#define REMOTEXY_ACCESS_PASSWORD ""
#endif 

CRemoteXY *remotexy;   

#define RemoteXY_Handler() remotexy->handler ()
#define RemoteXY_CONF const PROGMEM RemoteXY_CONF_PROGMEM


#endif //_REMOTEXY_H_

