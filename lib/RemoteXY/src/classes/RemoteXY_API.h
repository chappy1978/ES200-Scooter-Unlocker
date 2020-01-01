#ifndef _REMOTEXY_API_H_
#define _REMOTEXY_API_H_

#include <inttypes.h> 
#include <stdlib.h>
#include <Arduino.h>


#define REMOTEXY_PACKAGE_START_BYTE 0x55
#define REMOTEXY_PASSWORD_LENGTH_MAX 26
#define REMOTEXY_TIMEOUT 5000
#define REMOTEXY_SERVER_TIMEOUT 7000
#define REMOTEXY_CLOUD_RETRY_TIMEOUT 500
#define REMOTEXY_CLOUD_CONNECT_TIMEOUT 10000
#define REMOTEXY_CLOUD_RECONNECT_TIMEOUT 30000
#define REMOTEXY_CLOUD_ECHO_TIMEOUT 60000

// cloud states
#define REMOTEXY_CLOUD_STATE_STOP 0 
#define REMOTEXY_CLOUD_STATE_WAIT_RECONNECT 1 
#define REMOTEXY_CLOUD_STATE_WAIT_NEXT_TRY 2 
#define REMOTEXY_CLOUD_STATE_CONNECTION 3
#define REMOTEXY_CLOUD_STATE_REGISTRATION 6 
#define REMOTEXY_CLOUD_STATE_WORKING 7


class CRemoteXY_API {
  protected:
  uint8_t confVersion;
  uint8_t *conf;
  uint8_t *var;
  uint8_t *accessPassword;
  uint16_t outputLength;
  uint16_t inputLength;
  uint16_t confLength;
  uint8_t *connect_flag;

  uint8_t *receiveBuffer;
  uint16_t receiveBufferLength;
  uint16_t receiveIndex;
  uint16_t receiveCRC;

  uint32_t wireTimeOut;
  
  uint8_t moduleRunning;
    
  protected:
  virtual uint8_t initModule () {return 1;};
  virtual void handlerModule () {};
  virtual void closeConnection () {};
  virtual void sendStart (uint16_t len) {};
  virtual void sendByte (uint8_t b) {};
  virtual uint8_t receiveByte () {};
  virtual uint8_t availableByte () {};  
  
  public:
  void init (const void * _conf, void * _var, const char * _accessPassword) {
    uint32_t ms;
    uint8_t i;
    uint8_t* p = (uint8_t*)_conf;
    uint8_t b = getConfByte (p++);
    if (b==0xff) {
      inputLength = getConfByte (p++);
      inputLength |= getConfByte (p++)<<8;
      outputLength = getConfByte (p++);
      outputLength |= getConfByte (p++)<<8;
    }
    else {
      inputLength = b;
      outputLength = getConfByte (p++);    
    }
    confLength = getConfByte (p++);
    confLength |= getConfByte (p++)<<8;
    conf = p;
    confVersion = getConfByte (p);
    var = (uint8_t*)_var;
    uint16_t varLength = outputLength+inputLength;
    connect_flag = var+varLength;
    *connect_flag = 0;   
        
    accessPassword = (uint8_t*)_accessPassword;

    receiveBufferLength=inputLength;
    if ((*accessPassword!=0)&&(receiveBufferLength<REMOTEXY_PASSWORD_LENGTH_MAX))
      receiveBufferLength = REMOTEXY_PASSWORD_LENGTH_MAX;
    receiveBufferLength +=6;  
    
    receiveBuffer = (uint8_t*)malloc (receiveBufferLength);             
    
    p = var;
    i = varLength;
    while (i--) *p++=0;   
    
    resetWire ();
 
#if defined(REMOTEXY__DEBUGLOGS)
    DEBUGLOGS_init ();
    DEBUGLOGS_write("RemoteXY started");
#endif
  
    moduleRunning = initModule ();
#if defined(REMOTEXY__DEBUGLOGS)
    if (!moduleRunning) {
      DEBUGLOGS_write ("Wire module not available, RemoteXY stoped");
    }    
#endif     
    
  }

  
  private:
  inline uint8_t getConfByte (uint8_t* p) {
    return pgm_read_byte_near (p);                                     
  }
  
  private:
  void resetWire () {
    receiveIndex=0; 
    receiveCRC=initCRC ();
    *connect_flag = 0;    
    wireTimeOut= millis();
  }
  
      
  public:
  void handler () {
    uint8_t *p, *kp;
    uint16_t i;  
    uint8_t b;
    uint16_t packageLength;
    
    if (!moduleRunning) return;
    
    handlerModule ();
    
#if defined(REMOTEXY_CLOUD)  
    handlerCloud ();
#endif
    
    while (availableByte () > 0) {  
      b = receiveByte ();  
      
      if ((receiveIndex==0) && (b!=REMOTEXY_PACKAGE_START_BYTE)) continue;   
      receiveBuffer[receiveIndex++]=b;
      updateCRC (&receiveCRC, b);
      if (receiveIndex>receiveBufferLength) {       
        searchStartByte (1); //receiveBuffer overflow
      }
      while (true) {
        if (receiveIndex<6) break;
        packageLength = receiveBuffer[1]|(receiveBuffer[2]<<8);
        if (packageLength>receiveBufferLength) searchStartByte (1); // error
        else if (packageLength<6) searchStartByte (1); // error
        else if (packageLength==receiveIndex) {      
          if (receiveCRC==0) {
            if (handleReceivePackage ()) {
              receiveIndex=0;
              receiveCRC=initCRC ();
              break;
            }
          }
          searchStartByte (1); // error 
        }
        else if (packageLength<receiveIndex) {
          uint16_t crc = initCRC ();
          p = receiveBuffer;
          i=packageLength;
          while (i--) updateCRC (&crc, *(p++)); 
          if (crc==0) {
            if (handleReceivePackage ()) {
              searchStartByte (packageLength);
              continue;
            }
          }
          searchStartByte (1);        
        }
        else break;
      }
    }  
    
    if (millis() - wireTimeOut > REMOTEXY_TIMEOUT) {
      resetWire ();
    }      
  }
  
  private:
  uint16_t initCRC () {
    return 0xffff;
  }          
    
  private:
  void updateCRC (uint16_t *crc, uint8_t b) {
    *crc ^= b;
    for (uint8_t i=0; i<8; ++i) {
      if ((*crc) & 1) *crc = ((*crc) >> 1) ^ 0xA001;
      else *crc >>= 1;
    }
  } 
  
  private:  
  void sendByteUpdateCRC (uint8_t b, uint16_t *crc) {
    sendByte (b); 
    updateCRC (crc, b);
  }  

  private:  
  void sendPackage (uint8_t command, uint8_t *p, uint16_t length, uint8_t itConf) {
    uint16_t crc = initCRC ();
    uint16_t packageLength = length+6;
    sendStart (packageLength);
    sendByteUpdateCRC (REMOTEXY_PACKAGE_START_BYTE, &crc);
    sendByteUpdateCRC (packageLength, &crc);
    sendByteUpdateCRC (packageLength>>8, &crc);
    sendByteUpdateCRC (command, &crc);  
    uint8_t b;
    while (length--) {
      if (itConf) b=getConfByte (p++);
      else b=*p++;
      sendByteUpdateCRC (b, &crc);
    }
    sendByte (crc);  
    sendByte (crc>>8);
  }
  
  private:  
  void searchStartByte (uint16_t pos) {
    uint8_t *p, *kp;
    uint16_t i;
    uint16_t ri = receiveIndex;
    p=receiveBuffer+pos;
    receiveCRC=initCRC ();
    for (i=pos; i<ri; i++) {
      if (*p==REMOTEXY_PACKAGE_START_BYTE) {      
        kp=receiveBuffer;
        receiveIndex=receiveIndex-i;
        while (i++<ri) {
          updateCRC (&receiveCRC, *p);              
          *(kp++)=*(p++);
        }
        return;
      }
      p++;
    }        
    receiveIndex=0;
  }  
  
  uint8_t handleReceivePackage () {
    uint8_t command;
    uint16_t i;
    uint16_t length;
    uint8_t *p, *kp;
       
    length = receiveBuffer[1]|(receiveBuffer[2]>>8); 
    length-=6;
    command = receiveBuffer[3];
    switch (command) {  
      case 0x00:  
        uint8_t available;
        if (length==0) {
          if (*accessPassword==0) available=1;
          else available=0;
        }
        else {
          uint8_t ch;
          available = 1;
          p = receiveBuffer+4;
          kp = accessPassword; 
          while (true) {
            ch=*kp++;
            if (ch!=*p++) available=0;
            if (!ch) break;
          }                               
        } 
        if (available!=0) {
          sendPackage (command, conf, confLength,  1);
          *connect_flag = 1;
        }
        else {
          uint8_t buf[4];
          p = buf;
          kp = conf;         
          i=confVersion>=5?3:2;
          length = i+1;
          while (i--) *p++ = getConfByte(kp++);
          *p++ = 0xf0;
          sendPackage (command, buf, length,  0);
        }          
        break;   
      case 0x40:  
        sendPackage (command, var, inputLength+outputLength, 0); 
        break;   
      case 0x80:  
        if (length==inputLength) {
          p=receiveBuffer+4;
          kp=var;
          i= inputLength;
          while (i--) *kp++=*p++;
        }
        sendPackage (command, 0, 0, 0);
        break;   
      case 0xC0:  
        sendPackage (command, var+inputLength, outputLength, 0);
        break;   
#if defined(REMOTEXY_CLOUD)  
      case 0x10: // echo
        sendPackage (command, 0, 0, 0);
        break;   
      case 0x11:
        if (cloudState==REMOTEXY_CLOUD_STATE_REGISTRATION) {
          setCloudState (REMOTEXY_CLOUD_STATE_WORKING);
        }
        break;   
#endif //REMOTEXY_CLOUD       
      default:
        return 0; 
    }  
    
    wireTimeOut=millis();    
#if defined(REMOTEXY_CLOUD)  
    if (cloudState==REMOTEXY_CLOUD_STATE_WORKING) {
      cloudTimeOut=millis();
    }
#endif //REMOTEXY_CLOUD       
    return 1;
  }
  
  
///////////////////////////////////////////////////////////////////////////////
// PUBLIC DOCUMENTED FUNCTIONS
  
  public:
  uint8_t isConnected () {
    return *connect_flag;
  }


///////////////////////////////////////////////////////////////////////////////
// CLOUD SUPPORT 


#if defined(REMOTEXY_CLOUD)  
  protected:
  char *cloudServer;
  uint16_t cloudPort;
  uint8_t cloudRegistPackage[38];
  uint8_t cloudState;   
  uint32_t cloudTimeOut;


  virtual int8_t connectServerCloud (char * _cloudServer, uint16_t _cloudPort) {return 0;};

  public:
  void initCloud (const char * _cloudServer, uint16_t _cloudPort, const char * _cloudToken) {
    cloudServer = (char *) _cloudServer;
    cloudPort = _cloudPort;
    
    uint8_t i;
    uint8_t *p = cloudRegistPackage;
    *p++ = getConfByte(conf+0);
    *p++ = 0;    
    for (i=0; i<32; i++) {
      if (*_cloudToken==0) *(p++)=0;
      else *(p++)=*(_cloudToken++);
    }
    uint16_t *len = (uint16_t*)p;
    *len = outputLength + inputLength;
    if (confLength>*len) *len = confLength;   
    *len += 6+1;    
    len = (uint16_t*)(p+2);     
    *len = receiveBufferLength;  
    
    cloudState = REMOTEXY_CLOUD_STATE_STOP;
  }
  
  public:
  void startCloudConnection () {
    if (cloudState<REMOTEXY_CLOUD_STATE_CONNECTION) {
      setCloudState (REMOTEXY_CLOUD_STATE_CONNECTION);
    }
  }  
  
  public:
  void stopCloud () {
    closeConnection ();
    resetWire ();
    if (cloudState<REMOTEXY_CLOUD_STATE_CONNECTION) return;
#if defined(REMOTEXY__DEBUGLOGS)
    DEBUGLOGS_write("Cloud server disconnected");
#endif
    setCloudState (REMOTEXY_CLOUD_STATE_WAIT_RECONNECT);
  }
  
  private:
  void setCloudState (uint8_t state) {
    cloudState = state;
    cloudTimeOut = millis(); 
#if defined(REMOTEXY__DEBUGLOGS)
    switch (state) {
      case REMOTEXY_CLOUD_STATE_WAIT_RECONNECT: 
        DEBUGLOGS_write("Waiting to reconnect to the cloud server");
        break;
      case REMOTEXY_CLOUD_STATE_WAIT_NEXT_TRY: 
        DEBUGLOGS_write("Waiting to next try to connect to the cloud server");
        break;
      case REMOTEXY_CLOUD_STATE_CONNECTION: 
        DEBUGLOGS_write("Started connecting to cloud server");
        break;
      case REMOTEXY_CLOUD_STATE_REGISTRATION: 
        DEBUGLOGS_write("Waiting for registration on cloud server");
        break;
      case REMOTEXY_CLOUD_STATE_WORKING: 
        DEBUGLOGS_write("Connect to the cloud server successfully");
        break;
      default:
        DEBUGLOGS_write("Unknown cloud state ");
        REMOTEXY__DEBUGLOGS.print(cloudState);
        break;
    }
#endif
  }
  
  private:
  void handlerCloud () {
    int8_t res;
    if (!moduleRunning) return;
    switch (cloudState) {
      
      case REMOTEXY_CLOUD_STATE_WAIT_RECONNECT:  
        if (millis() - cloudTimeOut > REMOTEXY_CLOUD_RECONNECT_TIMEOUT)
          setCloudState (REMOTEXY_CLOUD_STATE_CONNECTION);          
        break;
        
      case REMOTEXY_CLOUD_STATE_WAIT_NEXT_TRY:  
        if (millis() - cloudTimeOut > REMOTEXY_CLOUD_RETRY_TIMEOUT) 
          setCloudState (REMOTEXY_CLOUD_STATE_CONNECTION);            
        break;
        
      case REMOTEXY_CLOUD_STATE_CONNECTION:  
        res = connectServerCloud (cloudServer, cloudPort);
        if (res == 1) {          
          setCloudState (REMOTEXY_CLOUD_STATE_REGISTRATION);   
          sendPackage (0x11, cloudRegistPackage, 38, 0);
        }
        else if (res == 0) {
#if defined(REMOTEXY__DEBUGLOGS)
          DEBUGLOGS_write("Cloud server connection error");          
#endif         
          setCloudState (REMOTEXY_CLOUD_STATE_WAIT_RECONNECT); 
        }
        else {
          setCloudState (REMOTEXY_CLOUD_STATE_WAIT_NEXT_TRY); 
        }
        break;
        
      case REMOTEXY_CLOUD_STATE_REGISTRATION:  
        if (millis() - cloudTimeOut > REMOTEXY_CLOUD_CONNECT_TIMEOUT) 
          stopCloud ();
        break;
        
      case REMOTEXY_CLOUD_STATE_WORKING:  
        if (millis() - cloudTimeOut > REMOTEXY_CLOUD_ECHO_TIMEOUT) 
          stopCloud ();
        break;
    }
  } 
  
///////////////////////////////////////////////////////////////////////////////
// PUBLIC DOCUMENTED FUNCTIONS OF CLOUD
  
  public:
  uint8_t isCloudConnected () {
    return (cloudState==REMOTEXY_CLOUD_STATE_WORKING ? 1:0);
  }
  
  
#endif //REMOTEXY_CLOUD

///////////////////////////////////////////////////////////////////////////////
// DEBUG FUNCTIONS

#if defined(REMOTEXY__DEBUGLOGS)
  uint8_t debug_flags;
  uint8_t debug_hexcounter;
  
  public:
  void DEBUGLOGS_init () {
    debug_flags=0;
    REMOTEXY__DEBUGLOGS.begin (REMOTEXY__DEBUGLOGS_SPEED);
    REMOTEXY__DEBUGLOGS.println();
  }

  public:
  void DEBUGLOGS_writeTime () {
    uint32_t d = millis();
    char s[15];
    sprintf (s, "[%5ld.%03ld] ",d/1000, d%1000);     
    REMOTEXY__DEBUGLOGS.println ();    
    REMOTEXY__DEBUGLOGS.print (s);
  }
  
  public:
  void DEBUGLOGS_write (char *s) {
    debug_flags = 0;
    DEBUGLOGS_writeTime (); 
    REMOTEXY__DEBUGLOGS.print(s);
  }
  
  public:
  void DEBUGLOGS_writeInput (char *s) {
    if ((debug_flags & 0x01)==0) {
      DEBUGLOGS_writeTime ();
      REMOTEXY__DEBUGLOGS.print("<- ");
    }
    debug_flags = 0x01;   
    REMOTEXY__DEBUGLOGS.print(s);
  }

  public:
  void DEBUGLOGS_writeOutput (char *s) {
    if ((debug_flags & 0x02)==0) {
      DEBUGLOGS_writeTime ();
      REMOTEXY__DEBUGLOGS.print("-> ");
    }
    debug_flags = 0x02;   
    REMOTEXY__DEBUGLOGS.print(s);
  }

  public:
  void DEBUGLOGS_writeInputHex (uint8_t b) {
    if ((debug_flags & 0x01)==0) {
      DEBUGLOGS_writeTime ();
      REMOTEXY__DEBUGLOGS.print("<-");
      debug_hexcounter=0;
    }
    debug_flags = 0x01;   
    DEBUGLOGS_writeHex (b);
  }

  public:
  void DEBUGLOGS_writeOutputHex (uint8_t b) {
    if ((debug_flags & 0x02)==0) {
      DEBUGLOGS_writeTime ();
      REMOTEXY__DEBUGLOGS.print("->");
      debug_hexcounter=0;
    }
    debug_flags = 0x02;
    DEBUGLOGS_writeHex (b);
  }
  
  public:
  void DEBUGLOGS_writeInputChar (char s) {
    if ((debug_flags & 0x01)==0) {
      DEBUGLOGS_writeTime ();
      REMOTEXY__DEBUGLOGS.print("<- ");
    }
    debug_flags = 0x01;   
    REMOTEXY__DEBUGLOGS.print(s);
  }

  public:
  void DEBUGLOGS_writeInputNewString () {
    debug_flags = 0;
  }
  
  public:
  void DEBUGLOGS_writeHex (uint8_t b) {
    debug_hexcounter++;
    if (debug_hexcounter>16) {
      REMOTEXY__DEBUGLOGS.println();
      REMOTEXY__DEBUGLOGS.print("              ");
      debug_hexcounter=1;
    }
    REMOTEXY__DEBUGLOGS.print(' ');
    REMOTEXY__DEBUGLOGS.print(b>>4, HEX); 
    REMOTEXY__DEBUGLOGS.print(b&0x0f, HEX); 
    
    
    
  }  
  
#endif //REMOTEXY__DEBUGLOGS 
   
};

#endif //_REMOTEXY_API_H_

