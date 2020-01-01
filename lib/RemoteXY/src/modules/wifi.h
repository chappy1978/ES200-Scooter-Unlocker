#ifndef _REMOTEXY_MOD_WIFI_H_
#define _REMOTEXY_MOD_WIFI_H_

/*
need include <SPI.h>, <WiFi.h>;
*/

#include "classes/RemoteXY_API.h"

class CRemoteXY : public CRemoteXY_API {

  protected:
  uint16_t port;
  char * wifiSsid;
  char * wifiPassword;
  
  WiFiServer *server;
  WiFiClient client;

  public:
  CRemoteXY (const void * _conf, void * _var, const char * _accessPassword, const char * _wifiSsid, const char * _wifiPassword, uint16_t _port) {
    port = _port;
    wifiSsid = (char *) _wifiSsid;
    wifiPassword = (char *) _wifiPassword;
    init (_conf, _var, _accessPassword);    
  }
  
  uint8_t initModule () {  
    delay(100);    
    /* station only*/
    WiFi.begin(wifiSsid, wifiPassword);
    uint8_t i = 40;
    while (WiFi.status() != WL_CONNECTED && i--) delay(500);
    
#if defined(REMOTEXY__DEBUGLOGS)
    if (i) {
      DEBUGLOGS_write("IP: ");
      REMOTEXY__DEBUGLOGS.print(WiFi.localIP());
    }
    else {
      DEBUGLOGS_write("Could not connect to ");
      REMOTEXY__DEBUGLOGS.print (wifiSsid);
    }
#endif
    if (!i) return 0;

    client.stop();
    server = new WiFiServer (port);
    server->begin();    
    return 1;
  }

  void sendByte (uint8_t b) {
    if (client) {
      if (client.connected()) {
        client.write(b);
#if defined(REMOTEXY__DEBUGLOGS)
        DEBUGLOGS_writeOutputHex (b);
#endif
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
        return b;
      }
    }
  }
  
  uint8_t availableByte () {
    if (!client) {
      client=server->available();
    }
    if (client) {
      if (client.connected()) return client.available();
      else client.stop();
    }
    return 0;
  }  

};


#define RemoteXY_Init() remotexy = new CRemoteXY (RemoteXY_CONF_PROGMEM, &RemoteXY, REMOTEXY_ACCESS_PASSWORD, REMOTEXY_WIFI_SSID, REMOTEXY_WIFI_PASSWORD, REMOTEXY_SERVER_PORT)



#endif //_REMOTEXY_MOD_WIFI_H_