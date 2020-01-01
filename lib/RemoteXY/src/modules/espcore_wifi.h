#ifndef _REMOTEXY_MOD__ESPCORE_WIFI_H_
#define _REMOTEXY_MOD__ESPCORE_WIFI_H_

/*
for ESP8266 board: need include <ESP8266WiFi.h>
for ESP32 board: need include <WiFi.h>
*/

#include "classes/RemoteXY_API.h"

#define REMOTEXY_SEND_BUFFER_LENGTH 256

class CRemoteXY : public CRemoteXY_API {

  protected:
  uint16_t port;
  char * wifiSsid;
  char * wifiPassword;
  
  WiFiServer *server;
  WiFiClient client; 
  
  uint8_t wifiStatus;
  uint32_t serverTimeOut;
  
  uint8_t sendBuffer[REMOTEXY_SEND_BUFFER_LENGTH];
  uint16_t sendBufferCount; 
  uint16_t sendBytesAvailable;  

  public:
  CRemoteXY (const void * _conf, void * _var, const char * _accessPassword, const char * _wifiSsid, const char * _wifiPassword, uint16_t _port) {
    port = _port;
    wifiSsid = (char *) _wifiSsid;
    wifiPassword = (char *) _wifiPassword;
    serverTimeOut = 0;
    init (_conf, _var, _accessPassword);    
  }
  
  uint8_t initModule () {  
    delay(100);
    WiFi.disconnect();
    WiFi.softAPdisconnect(true);
#if defined(REMOTEXY_WIFI__POINT)
    /* access point */
    WiFi.mode(WIFI_AP);
    WiFi.softAP(wifiSsid, wifiPassword);
#if defined(REMOTEXY__DEBUGLOGS)
    DEBUGLOGS_write ("IP: ");
    REMOTEXY__DEBUGLOGS.print (WiFi.softAPIP());
#endif
#else    
    /* station */
    WiFi.mode(WIFI_STA);
    WiFi.begin(wifiSsid, wifiPassword);
    delay(1000);
    
#if defined(REMOTEXY__DEBUGLOGS)
    DEBUGLOGS_write("Started connecting to access point...");
#endif    
#endif
    client.stop();
    server = new WiFiServer (port);
    server->begin();    
    server->setNoDelay(true);
#if defined(REMOTEXY__DEBUGLOGS)
    DEBUGLOGS_write ("Server started");
#endif
    return 1;
  }

  void handlerModule () {
    uint8_t status = WiFi.status();
    if (status != wifiStatus) {
      wifiStatus = status;
#if defined(REMOTEXY__DEBUGLOGS)
      if (wifiStatus == WL_CONNECTED) {
        DEBUGLOGS_write ("Connected to access point");
        DEBUGLOGS_write ("IP: ");
        REMOTEXY__DEBUGLOGS.print (WiFi.localIP());
      }
#endif  
    }
    
    if (!client) {
      client=server->available();
      if (client) {
        resetServerTimeOut ();  // new client
#if defined(REMOTEXY__DEBUGLOGS)
        DEBUGLOGS_write ("Client connected");
#endif
      }
    }
    if (client) {
      if (client.connected()) {
        if (millis () - serverTimeOut > REMOTEXY_SERVER_TIMEOUT) {
          client.stop ();
#if defined(REMOTEXY__DEBUGLOGS)
          DEBUGLOGS_write ("Client stoped by timeout");
#endif
        }
      }
      else {
        client.stop ();
#if defined(REMOTEXY__DEBUGLOGS)
        DEBUGLOGS_write ("Client stoped");
#endif
      }
    }        
  }


  void sendStart (uint16_t len) {
    sendBytesAvailable = len;
    sendBufferCount = 0;
  }

  void sendByte (uint8_t b) {
    if (client) {
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
          resetServerTimeOut ();
        } 
      }
    }
  }  
  
  uint8_t receiveByte () {
    uint8_t b;
    if (client) {
      if (client.connected()) {
        b = client.read();
#if defined(REMOTEXY__DEBUGLOGS)
        DEBUGLOGS_writeInputHex (b);
#endif
        resetServerTimeOut ();
        return b;
      }
    }
  }
  
  uint8_t availableByte () {   
#if !defined(REMOTEXY_WIFI__POINT)
    if (wifiStatus != WL_CONNECTED) return 0;
#endif
    if (client) {
      if (client.connected()) return client.available();
    }
    return 0;
  }  
  
  void resetServerTimeOut () {
    serverTimeOut = millis (); //REMOTEXY_SERVER_TIMEOUT;
  }

};


#define RemoteXY_Init() remotexy = new CRemoteXY (RemoteXY_CONF_PROGMEM, &RemoteXY, REMOTEXY_ACCESS_PASSWORD, REMOTEXY_WIFI_SSID, REMOTEXY_WIFI_PASSWORD, REMOTEXY_SERVER_PORT)



#endif //_REMOTEXY_MOD__ESPCORE_WIFI_H_