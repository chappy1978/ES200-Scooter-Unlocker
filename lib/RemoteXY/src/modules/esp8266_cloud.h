#ifndef _REMOTEXY_MOD_ESP8266_H_
#define _REMOTEXY_MOD_ESP8266_H_

#include "classes/RemoteXY_AT.h"

#define REMOTEXY_ESP8266_MAX_SEND_BYTES 2048

class CRemoteXY : public CRemoteXY_AT {

protected:
  char * wifiSsid;
  char * wifiPassword;
  uint16_t connectAvailable;  // need receive data
  uint8_t clientState;


  uint16_t sendBytesAvailable;  
  uint16_t sendBytesLater;

public:

#if defined(REMOTEXY_PORT__HARDSERIAL)
  CRemoteXY (const void * _conf, void * _var, const char * _accessPassword, HardwareSerial * _serial, long _serialSpeed, const char * _wifiSsid, const char * _wifiPassword, const char * _cloudServer, uint16_t _cloudPort, const char * _cloudToken) {
    initSerial (_serial, _serialSpeed);
#elif defined(REMOTEXY_PORT__SOFTSERIAL)
  CRemoteXY (const void * _conf, void * _var, const char * _accessPassword, uint8_t _serialRx, uint8_t _serialTx, long _serialSpeed, const char * _wifiSsid, const char * _wifiPassword, const char * _cloudServer, uint16_t _cloudPort, const char * _cloudToken) {
    initSerial (_serialRx, _serialTx, _serialSpeed);
#endif
    initAT ();
    wifiSsid = (char *) _wifiSsid;
    wifiPassword = (char *) _wifiPassword;

    connectAvailable=0;
    sendBytesAvailable=0;
    sendBytesLater=0;
    
    initCloud (_cloudServer, _cloudPort, _cloudToken);  
    init (_conf, _var, _accessPassword);
    startCloudConnection ();
  }


  protected:
  uint8_t initModule () {
    
#if defined(REMOTEXY__DEBUGLOGS)
    DEBUGLOGS_write ("Find ESP module...");
#endif     

    uint8_t tryCount=20;
    while (--tryCount>0) {
      sendATCommand ("AT",0);
      if (waitATAnswer (AT_ANSWER_OK, 1000)) break;
    }
    if (tryCount==0) {
#if defined(REMOTEXY__DEBUGLOGS)
      DEBUGLOGS_write ("ESP module not found");
#endif     
      return 0;
    }
    sendATCommand ("AT+RST",0);
    if (!waitATAnswer (AT_ANSWER_OK, 1000)) return 0; 
    if (!waitATAnswer (AT_MESSAGE_READY, 5000)) return 0;
     
    return setModule (); 
  }
  
  uint8_t setModule () {    
    stopCloud ();
    clientState = 0;
    connectAvailable=0;
    sendATCommand ("ATE0",0);
    if (!waitATAnswer (AT_ANSWER_OK, 1000)) return 0;       
    sendATCommand ("AT+CWMODE=1",0);
    if (!waitATAnswer (AT_ANSWER_OK, 2000)) return 0;   
    sendATCommand ("AT+CWQAP",0);
    if (!waitATAnswer (AT_ANSWER_OK, 2000)) return 0;   
    sendATCommand ("AT+CWDHCP=1,1",0);
    if (!waitATAnswer (AT_ANSWER_OK, 2000)) return 0;    
    sendATCommand ("AT+CWJAP=\"",wifiSsid,"\",\"",wifiPassword,"\"",0);
    if (!waitATAnswer (AT_ANSWER_OK, 30000)) return 0;  
#if defined(REMOTEXY__DEBUGLOGS)
    sendATCommand ("AT+CIPSTA?",0);
    if (!waitATAnswer (AT_ANSWER_OK, 2000)) return 0;  
#endif     
    sendATCommand ("AT+CIPMODE=0",0);
    if (!waitATAnswer (AT_ANSWER_OK, 2000)) return 0;   
    sendATCommand ("AT+CIPMUX=1",0);
    if (!waitATAnswer (AT_ANSWER_OK, 1000)) return 0;
    delay(100);
    
    return 1;
  }

  int8_t connectServerCloud (char * _cloudServer, uint16_t _cloudPort) {
    char sport[6];    
    rxy_itos (_cloudPort, sport);
    if (testATecho ()==2) setModule ();
    sendATCommand ("AT+CIPSTART=0,\"TCP\",\"", _cloudServer,"\",", sport,0);
    if (waitATAnswer (AT_ANSWER_OK, 10000)) clientState = 1;
    else clientState = 0;
    return clientState;
  }
  
  //override API Cloud
  void closeConnection () {
    if (clientState) {
      sendATCommand ("AT+CIPCLOSE=0",0);
      waitATAnswer (AT_ANSWER_OK, 1000);    
      clientState = 0;
    }
  }
  
  //override API  
  void handlerModule () {
    while (serial->available ()>0) {      
      if (connectAvailable) return;
      readATMessage ();
    }
  }
  
  //override AT
  void readyAT () {
    if (moduleRunning) {
      setModule ();
      startCloudConnection ();
    }
  }  
  
  //override AT
  void connectAT () {
    connectAvailable=0;
  };
 
  //override AT
  void closedAT () {
    stopCloud ();
    clientState = 0;
  }
  
  //override AT
  void inputDataAT () {
    uint16_t size;
    size=getATParamInt (1);
    connectAvailable=size;
  }  
  
  
  void sendStart (uint16_t len) {
    char s[6];
    if (clientState) {
      sendBytesLater=0;
      if (len>REMOTEXY_ESP8266_MAX_SEND_BYTES) {
        sendBytesLater=len-REMOTEXY_ESP8266_MAX_SEND_BYTES;
        len=REMOTEXY_ESP8266_MAX_SEND_BYTES;
      }
      sendBytesAvailable=len;
      rxy_itos (len, s);     
      sendATCommand ("AT+CIPSEND=0,",s,0);
      if (!waitATAnswer (AT_ANSWER_GO, 1000)) sendBytesAvailable=0;
    }
  }
  
  void sendByte (uint8_t b) {
    if (sendBytesAvailable) {
      serial->write (b); 
#if defined(REMOTEXY__DEBUGLOGS)
        DEBUGLOGS_writeOutputHex (b);
#endif
      sendBytesAvailable--;
      if (!sendBytesAvailable) {
        waitATAnswer (AT_ANSWER_SEND_OK, 1000);      
        if (sendBytesLater) sendStart (sendBytesLater); 
      }
    }
  }
  
  
  uint8_t receiveByte () {
    uint8_t b;
    if (connectAvailable) {
      if (serial->available ()>0) {
        connectAvailable--;
        b = serial->read  ();
#if defined(REMOTEXY__DEBUGLOGS)
        DEBUGLOGS_writeInputHex (b);
#endif
        return b;
      }
    }  
    return 0;
  }
  
  uint8_t availableByte () {
    if (connectAvailable) {
      return serial->available ()>0;
    }
    return 0;
  }  


};




#if defined(REMOTEXY_PORT__HARDSERIAL)
  #define RemoteXY_Init() remotexy = new CRemoteXY (RemoteXY_CONF_PROGMEM, &RemoteXY, REMOTEXY_ACCESS_PASSWORD, &REMOTEXY_SERIAL, REMOTEXY_SERIAL_SPEED, REMOTEXY_WIFI_SSID, REMOTEXY_WIFI_PASSWORD, REMOTEXY_CLOUD_SERVER, REMOTEXY_CLOUD_PORT, REMOTEXY_CLOUD_TOKEN)
#elif defined(REMOTEXY_PORT__SOFTSERIAL)
  #define RemoteXY_Init() remotexy = new CRemoteXY (RemoteXY_CONF_PROGMEM, &RemoteXY, REMOTEXY_ACCESS_PASSWORD, REMOTEXY_SERIAL_RX, REMOTEXY_SERIAL_TX, REMOTEXY_SERIAL_SPEED, REMOTEXY_WIFI_SSID, REMOTEXY_WIFI_PASSWORD, REMOTEXY_CLOUD_SERVER, REMOTEXY_CLOUD_PORT, REMOTEXY_CLOUD_TOKEN)
#endif


#endif //_REMOTEXY_MOD_ESP8266_H_