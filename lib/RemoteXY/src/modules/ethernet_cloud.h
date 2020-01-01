#ifndef _REMOTEXY_MOD_ETHERNET_CLOUD_H_
#define _REMOTEXY_MOD_ETHERNET_CLOUD_H_

#include "classes/RemoteXY_API.h"

class CRemoteXY : public CRemoteXY_API {

  protected:
  char * macAddress;
  uint16_t port;
  EthernetClient client;
  uint8_t clientState;
  
  public:
  CRemoteXY (const void * _conf, void * _var, const char * _accessPassword, const char * _macAddress, const char * _cloudServer, uint16_t _cloudPort, const char * _cloudToken) {
    macAddress = (char *) _macAddress;
    init (_conf, _var, _accessPassword);
    clientState = 0;
    initCloud (_cloudServer, _cloudPort, _cloudToken);
    startCloudConnection ();  
  }
  
  uint8_t initModule () {  
    uint8_t mac[6];
    delay(100);    
    rxy_getMacAddr (macAddress, mac);  
#if defined(REMOTEXY__DEBUGLOGS)
    DEBUGLOGS_write("Ethernet connecting...");
#endif
    if (Ethernet.begin(mac) == 0) {
#if defined(REMOTEXY__DEBUGLOGS)
      DEBUGLOGS_write("Ethernet DHCP configuration failed");
#endif
      return 0;
    }  
#if defined(REMOTEXY__DEBUGLOGS)
    DEBUGLOGS_write("Ethernet DHCP connected");
    DEBUGLOGS_write("IP: ");
    REMOTEXY__DEBUGLOGS.print(Ethernet.localIP());
#endif      
    delay(100);
    
    return 1;
  }
 
  
  int8_t connectServerCloud (char * _cloudServer, uint16_t _cloudPort) {
    if (client.connect(_cloudServer, _cloudPort)) clientState = 1;
    else clientState = 0;
    return clientState;
  }
  

  //override API
  void closeConnection () {
    if (clientState) {
      client.stop ();
      clientState = 0;
    }
  }  
      
  void sendByte (uint8_t b) {
    if (clientState) {
      if (client.connected()) {
        client.write(b);  
#if defined(REMOTEXY__DEBUGLOGS)
        DEBUGLOGS_writeOutputHex (b);
#endif
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

#define RemoteXY_Init() remotexy = new CRemoteXY (RemoteXY_CONF_PROGMEM, &RemoteXY, REMOTEXY_ACCESS_PASSWORD, REMOTEXY_ETHERNET_MAC, REMOTEXY_CLOUD_SERVER, REMOTEXY_CLOUD_PORT, REMOTEXY_CLOUD_TOKEN)


#endif //_REMOTEXY_MOD_ETHERNET_CLOUD_H_