/*
  for ESP32 bluetooth connection
  thanks FedericoBusero (github.com/FedericoBusero/)
*/

#ifndef _REMOTEXY_MOD__ESP32CORE_BLE_H_
#define _REMOTEXY_MOD__ESP32CORE_BLE_H_

#include "classes/RemoteXY_API.h"

#define REMOTEXY_SEND_BUFFER_LENGTH 20
#define REMOTEXY_RECEIVE_BUFFER_LENGTH 1024

#include <BLEDevice.h>
#include <BLEServer.h>
#include <BLEUtils.h>
#include <BLE2902.h>


#define SERVICE_UUID             "0000FFE0-0000-1000-8000-00805F9B34FB" // UART service UUID
#define CHARACTERISTIC_UUID_RXTX "0000FFE1-0000-1000-8000-00805F9B34FB"

class CRemoteXY : public CRemoteXY_API, BLEServerCallbacks, BLECharacteristicCallbacks   {
  protected:
    BLEServer *pServer;
    BLECharacteristic * pRxTxCharacteristic;

    uint8_t sendBuffer[REMOTEXY_SEND_BUFFER_LENGTH];
    uint16_t sendBufferCount;
    uint16_t sendBytesAvailable;
    
    uint8_t receiveBuffer[REMOTEXY_RECEIVE_BUFFER_LENGTH];
    uint16_t receiveBufferStart;
    uint16_t receiveBufferPos;
    uint16_t receiveBufferCount;
    

  public:
    CRemoteXY (const void * _conf, void * _var, const char * _accessPassword, const char * _bleDeviceName) {
      init (_conf, _var, _accessPassword);
#if defined(REMOTEXY__DEBUGLOGS)
      DEBUGLOGS_write("Init BLE");
#endif

      receiveBufferCount = 0;
	    
      // Create the BLE Device
      BLEDevice::init(_bleDeviceName);

      // Create the BLE Server
      pServer = BLEDevice::createServer();
      pServer->setCallbacks(this);

      // Create the BLE Service
      BLEService *pService = pServer->createService(SERVICE_UUID);

      // Create a BLE Characteristic
      pRxTxCharacteristic = pService->createCharacteristic(
                              CHARACTERISTIC_UUID_RXTX,
                              BLECharacteristic::PROPERTY_READ |
                              BLECharacteristic::PROPERTY_NOTIFY |
                              BLECharacteristic::PROPERTY_WRITE_NR 
                            );

      BLE2902 *ble2902 = new BLE2902();
      ble2902->setNotifications(true);
      pRxTxCharacteristic->addDescriptor(ble2902);
      pRxTxCharacteristic->setCallbacks(this);

      // Start the service
      pService->start();

      // Start advertising
      pServer->getAdvertising()->addServiceUUID(pService->getUUID());
      pServer->getAdvertising()->start();

#if defined(REMOTEXY__DEBUGLOGS)
      DEBUGLOGS_write("BLE started");
#endif
    }

    void onConnect(BLEServer* pServer) {
#if defined(REMOTEXY__DEBUGLOGS)
      DEBUGLOGS_write("Client connected");
#endif
      receiveBufferStart = 0;
      receiveBufferPos = 0;
      receiveBufferCount = 0;
    };

    void onDisconnect(BLEServer* pServer) {
#if defined(REMOTEXY__DEBUGLOGS)
      DEBUGLOGS_write("Client disconnected");
#endif
      receiveBufferCount = 0;
    }

    void onWrite(BLECharacteristic *pCharacteristic) {
      std::string rxValue = pCharacteristic->getValue();

      if (rxValue.length() > 0) {
        for (uint16_t i = 0; i < rxValue.length(); i++) {        
          uint8_t b = (uint8_t)rxValue[i];  
          receiveBuffer[receiveBufferPos++] =  b;
          if (receiveBufferPos>=REMOTEXY_RECEIVE_BUFFER_LENGTH) receiveBufferPos=0; 
          if (receiveBufferCount<REMOTEXY_RECEIVE_BUFFER_LENGTH) receiveBufferCount++;
          else {
            receiveBufferStart++;
            if (receiveBufferStart>=REMOTEXY_RECEIVE_BUFFER_LENGTH) receiveBufferStart=0;
          } 
#if defined(REMOTEXY__DEBUGLOGS)
          DEBUGLOGS_writeInputHex (b);
#endif          
        }
      }
    }


    void sendStart (uint16_t len) {
      sendBytesAvailable = len;
      sendBufferCount = 0;
    }

    void sendByte (uint8_t b) {
      sendBuffer[sendBufferCount++] = b;
      sendBytesAvailable--;
      if ((sendBufferCount == REMOTEXY_SEND_BUFFER_LENGTH) || (sendBytesAvailable == 0)) {
#if defined(REMOTEXY__DEBUGLOGS)
        for (uint16_t i = 0; i < sendBufferCount; i++) {
          DEBUGLOGS_writeOutputHex (sendBuffer[i]);
        }
#endif
        pRxTxCharacteristic->setValue((uint8_t *)sendBuffer, sendBufferCount);
        pRxTxCharacteristic->notify();
        sendBufferCount = 0;
      }

    }

    uint8_t receiveByte () {
      uint8_t b =0;
      if (receiveBufferCount>0) {      
        b = receiveBuffer[receiveBufferStart++];
        if (receiveBufferStart>=REMOTEXY_RECEIVE_BUFFER_LENGTH) receiveBufferStart=0;
        receiveBufferCount--;
      }
      return b;
    }


    uint8_t availableByte () {
      return receiveBufferCount;
    };

};


#define RemoteXY_Init() remotexy = new CRemoteXY (RemoteXY_CONF_PROGMEM, &RemoteXY, REMOTEXY_ACCESS_PASSWORD, REMOTEXY_BLUETOOTH_NAME)

#endif //_REMOTEXY_MOD__ESP32CORE_BLE_H_