/*
  DeviceManager.h - Library for manage arduino ports through web page.
  Created by Jonathas Barbosa, December 2, 2019.
  Released into the public domain.
*/
#ifndef DeviceManager_h
#define DeviceManager_h

#include "Arduino.h"
#include "ESP8266WebServer.h"


struct Device
{
    String name;
    uint8_t port;
    bool state;
};

class DeviceManager
{
  public:
    DeviceManager(ESP8266WebServer* server, Device* devices);
    void initialize();
  private:
    ESP8266WebServer* _server;
    Device* _devices;
    void initializePorts();
    void createRoutes();
    void showAllDevices();
    void controlPort(int port, bool state);
    void handleRoot();
    String getState(bool state);
    void handleTurnDevices(bool state);
    uint8_t findDeviceIndexByPort(int port);
    void handleNotFound();
};

#endif