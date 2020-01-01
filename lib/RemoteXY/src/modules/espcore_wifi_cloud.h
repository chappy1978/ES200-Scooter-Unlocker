#ifndef _REMOTEXY_MOD__ESPCORE_WIFI_CLOUD_H_
#define _REMOTEXY_MOD__ESPCORE_WIFI_CLOUD_H_

/*
for ESP8266 board: need include <ESP8266WiFi.h>
for ESP32 board: need include <WiFi.h>
*/

#include "classes/RemoteXY_API.h"

#define REMOTEXY_SEND_BUFFER_LENGTH 256

class CRemoteXY : public CRemoteXY_API {

  protected:
  char * wifiSsid;
  char * wifiPassword;
  
  WiFiClient client;
  uint8_t clientState;
  uint8_t wifiStatus;
  
  uint8_t sendBuffer[REMOTEXY_SEND_BUFFER_LENGTH];
  uint16_t sendBufferCount; 
  uint16_t sendBytesAvailable;  

  public:
  CRemoteXY (const void * _conf, void * _var, const char * _accessPassword, const char * _wifiSsid, const char * _wifiPassword, const char * _cloudServer, uint16_t _cloudPort, const char * _cloudToken) {
    wifiSsid = (char *) _wifiSsid;
    wifiPassword = (char *) _wifiPassword;
    clientState=0;
    
    init (_conf, _var, _accessPassword);  
    initCloud (_cloudServer, _cloudPort, _cloudToken);  
  }
  
  uint8_t initModule () {  
    delay(100);
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
       
    /* station */
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSsid, wifiPassword);
    delay(1000);
#if defined(REMOTEXY__DEBUGLOGS)
    DEBUGLOGS_write("Started connecting to access point");
#endif    

    return 1;
  }
  
  void handlerModule () {
    uint8_t status = WiFi.status();
    if (status != wifiStatus) {
      wifiStatus = status;
      if (wifiStatus == WL_CONNECTED) {
#if defined(REMOTEXY__DEBUGLOGS)
        DEBUGLOGS_write("Connected to access point");
        DEBUGLOGS_write("IP: ");
        REMOTEXY__DEBUGLOGS.print(WiFi.localIP());
#endif  
        startCloudConnection ();
      }
      if (wifiStatus != WL_CONNECTED) { 
        clientState = 0;
        stopCloud ();
      }
    }    
  }
  
  int8_t connectServerCloud (char * _cloudServer, uint16_t _cloudPort) {
    if (wifiStatus != WL_CONNECTED) return -1; 
    if (client.connect(_cloudServer, _cloudPort)) clientState = 1;
    return clientState;
  }
  

  //override API
  void closeConnection () {
    if (clientState) {
      client.stop ();
      clientState = 0;
    }
  }

  void sendStart (uint16_t len) {
    sendBytesAvailable = len;
    sendBufferCount = 0;
  }

  void sendByte (uint8_t b) {
    if (clientState) {
      if (client.connected()) {
#if defined(REMOTEXY__DEBUGLOGS)
        DEBUGLOGS_writeOutputHex (b);
#endif
        sendBuffer[sendBufferCount++] = b;
        sendBytesAvailable--;       
        if ((sendBufferCount==REMOTEXY_SEND_BUFFER_LENGTH) || (sendBytesAvailable==0)) {
          uint8_t buf[sendBufferCount];
          for (uint16_t i=0; i<sendBufferCount; i++) buf[i]=sendBuffer[i];
          client.write(buf, sendBufferCount);
          sendBufferCount=0;
        } 
      }
      else stopCloud ();
    }
  }  
  
  uint8_t receiveByte () {
    uint8_t b;
    if (clientState) {
      if (client.connected()) {
        b = client.read();
#if defined(REMOTEXY__DEBUGLOGS)
        DEBUGLOGS_writeInputHex (b);
#endif
        return b;
      }
      else stopCloud ();
    }
  }
  
  uint8_t availableByte () {
    if (clientState) {
      if (client.connected()) return client.available();
      else stopCloud ();
    }
    return 0;
  }  

};


#define RemoteXY_Init() remotexy = new CRemoteXY (RemoteXY_CONF_PROGMEM, &RemoteXY, REMOTEXY_ACCESS_PASSWORD, REMOTEXY_WIFI_SSID, REMOTEXY_WIFI_PASSWORD, REMOTEXY_CLOUD_SERVER, REMOTEXY_CLOUD_PORT, REMOTEXY_CLOUD_TOKEN)



#endif //_REMOTEXY_MOD__ESPCORE_WIFI_CLOUD_H_