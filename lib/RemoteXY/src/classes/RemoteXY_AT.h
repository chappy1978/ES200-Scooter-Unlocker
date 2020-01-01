#ifndef _REMOTEXY_AT_H_
#define _REMOTEXY_AT_H_

#include <inttypes.h> 
#include <Arduino.h>
#include <string.h>
#include <stdarg.h>
#include "classes/RemoteXY_Serial.h"


#define AT_BUFFER_STR_LENGTH 10

const char * AT_ANSWER_ERROR = "ERROR";
const char * AT_ANSWER_OK = "OK";
const char * AT_ANSWER_SEND_OK = "SEND OK";
const char * AT_MESSAGE_READY = "ready";
const char * AT_ANSWER_GO = ">";
const char * AT_MESSAGE_AT = "AT";
const char * AT_MESSAGE_CONNECT = "?,CONNECT";
const char * AT_MESSAGE_CLOSED = "?,CLOSED";
const char * AT_MESSAGE_CONNECT_FAIL = "?,CONNECT FAIL";
const char * AT_MESSAGE_IPD = "+IPD,?,*:";


class CRemoteXY_AT : public CRemoteXY_Serial {
  
  protected:
  char bufferAT[AT_BUFFER_STR_LENGTH+1];
  uint8_t bufferATPos;
  char * params[3];
  uint8_t paramsLength[3];      
  uint8_t haveEcho;
    
  protected:
  void initAT () {
    bufferATPos=0;
  }

  virtual void closedAT () {};
  virtual void connectAT () {};
  virtual void readyAT () {};
  virtual void inputDataAT () {};
 
  protected:  
  void sendATCommand (const char * command, ...) { 
   
    char *p = (char*) command;
    va_list argptr;
    while (serial->available () > 0) serial->read (); 
    va_start (argptr, command);
    while (p) {
      serial->write (p);
#if defined(REMOTEXY__DEBUGLOGS)
      DEBUGLOGS_writeOutput (p);
#endif
      p=va_arg(argptr,char*);
    }
    va_end(argptr);     
    serial->write ("\r\n");
  }    
  
  protected:  
  uint8_t waitATAnswer (const char * answer, uint16_t delay) {
    uint8_t b;
    uint32_t timeOut = millis();
  
    uint8_t k=0;
    while (millis() - timeOut <= delay) {
    
    
      if (serial->available ()>0) {
        b=serial->read  ();
        if (b==10) continue;
#if defined(REMOTEXY__DEBUGLOGS)
        if (b==13) DEBUGLOGS_writeInputNewString ();
        else DEBUGLOGS_writeInputChar (b);
#endif
        if (b==13) {
          bufferAT[k++]=0; 
          bufferATPos=0;
          k=0;
          if (strcmp (bufferAT,answer)==0) return 1;
          if (strcmp (bufferAT,AT_ANSWER_ERROR)==0) return 0;
          if (cmpBufferAT () ==  AT_MESSAGE_READY) return 0;       
          if (strcmp (bufferAT,AT_MESSAGE_AT)==0) haveEcho=1;
        }
        else {
          if (k<AT_BUFFER_STR_LENGTH) bufferAT[k++]=b;
          if (b=='>') {
            if (answer==AT_ANSWER_GO) return 1;
          }
        }
      }
    } 
    return 0;  
  }

  // echo test, returns
  //  0 - no answer
  //  1 - no echo
  //  2 - have echo
  protected:  
  uint8_t testATecho () { 
    haveEcho = 0;
    sendATCommand ("AT",0);
    if (!waitATAnswer (AT_ANSWER_OK, 1000)) return 0;
    return (haveEcho==0?1:2);
  }


  protected:
  void readATMessage () {
    uint8_t b;
    while (serial->available ()>0) {
      b=serial->read  ();
      if (b==10) continue;
#if defined(REMOTEXY__DEBUGLOGS)
      if (b==13) DEBUGLOGS_writeInputNewString ();
      else DEBUGLOGS_writeInputChar (b);
#endif
      if (b==13) {
        bufferAT[bufferATPos]=0;
        bufferATPos=0;
         if (!cmpBufferAT ()) return;
      }
      else {
        if (bufferATPos<AT_BUFFER_STR_LENGTH) bufferAT[bufferATPos++]=b;  
        if (b==':') {
          bufferAT[bufferATPos]=0;
          if (strcmpAT (bufferAT,AT_MESSAGE_IPD)==0) {
#if defined(REMOTEXY__DEBUGLOGS)
            DEBUGLOGS_writeInputNewString ();
#endif
            bufferATPos=0;
            inputDataAT ();
            return;
          } 
        }
      }    
    }
  }
  
  protected:
  const char * cmpBufferAT () {
    if (strcmpAT (bufferAT,AT_MESSAGE_CONNECT)==0) {connectAT (); return AT_MESSAGE_CONNECT;}
    if (strcmpAT (bufferAT,AT_MESSAGE_CLOSED)==0) {closedAT (); return AT_MESSAGE_CLOSED;}             
    if (strcmpAT (bufferAT,AT_MESSAGE_CONNECT_FAIL)==0) {closedAT (); return AT_MESSAGE_CONNECT_FAIL;}             
    if (strcmpAT (bufferAT,AT_MESSAGE_READY)==0) {readyAT (); return AT_MESSAGE_READY;}    
    return 0;           
  }  
  
  protected:
  uint8_t strcmpAT (char * str, const char * temp) {
    uint8_t k = 0;
    while (*temp) {
      if (!*str) return 1;
      switch (*temp) {
        case '?':
          params[k]=str;
          paramsLength[k]=1;
          temp++; 
          str++;   
          k++; 
          break;
        case '*':
          params[k]=str;
          paramsLength[k]=0; 
          temp++;
          while (*str!=*temp) {
            if (!*str++) return 1; 
            paramsLength[k]++;
          }
          k++;
          break;
        default: 
          if (*(str++)!=*(temp++)) return 1;
          break;
      }
    }
    if (*temp) return 1;
    return 0;
  }  
  
  protected:
  uint16_t getATParamInt (uint8_t k) {
    uint16_t res = 0;
    char * p=params[k];
    uint8_t i=paramsLength[k];
    while (i--) res = res*10+(*p++)-'0';
    return res;
  }  
   
};

#endif //_REMOTEXY_AT_H_