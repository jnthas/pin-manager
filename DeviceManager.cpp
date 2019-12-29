/*
  DeviceManager.h - Library for manage arduino ports through web page.
  Created by Jonathas Barbosa, December 2, 2019.
  Released into the public domain.
*/

#include "Arduino.h"
#include "ESP8266WebServer.h"
#include "DeviceManager.h"


String MAIN_PAGE = "<!doctype html>"\
                    "<html lang='en'>"\
                    "<head>"\
                        "<meta charset='utf-8'>"\
                        "<title>Device Manager</title>"\
                        "<base href='/'>"\
                        "<meta name='viewport' content='width=device-width, initial-scale=1'>"\
                        "<script>"\
                            "function onToggleChange(port, toggleElem) {"\
                                "const isChecked = toggleElem.checked ? 'on' : 'off';"\
                                "const request = new XMLHttpRequest();"\
                                "const url = `/turn/${isChecked}?port=${port}`;"\
                                "request.open('POST', url);"\
                                "request.send();"\
                            "}"\
                        "</script>"\
                        "<style>"\
                            "*{margin:0;box-sizing:border-box}body,html{background-color:#e5e5e5;width:100%;height:100%;font-family:sans-serif;font-size:16px}main{min-height:100%;display:flex;flex-direction:column;align-items:center;justify-content:center}section{display:flex;flex-direction:column;justify-content:space-evenly;align-items:center;text-align:center;height:100%}section div{width:100%;margin:1rem 0;display:flex;justify-content:space-evenly;align-items:center}section div>span{display:block;text-align:left;font-weight:700;width:16rem}.switch{position:relative;display:inline-block;width:60px;height:34px}.switch input{opacity:0;width:0;height:0}.slider{position:absolute;cursor:pointer;top:0;left:0;right:0;bottom:0;background-color:#ccc;-webkit-transition:.4s;transition:.4s}.slider:before{position:absolute;content:\"\";height:26px;width:26px;left:4px;bottom:4px;background-color:#fff;-webkit-transition:.4s;transition:.4s}input:checked+.slider{background-color:#2196f3}input:focus+.slider{box-shadow:0 0 1px #2196f3}input:checked+.slider:before{-webkit-transform:translateX(26px);-ms-transform:translateX(26px);transform:translateX(26px)}.slider.round{border-radius:34px}.slider.round:before{border-radius:50%}"\
                        "</style>"\
                    "</head>"\
                    "<body>"\
                        "<main>"\
                            "<h1>Devices</h1>"\
                            "<section>"\
                            "{DEVICE_ELEM}"\
                            "</section>"\
                        "</main>"\
                    "</body>"\
                    "</html>";


String DEVICE_ELEM = "<div><span>{DEVICE_NAME}</span><label class='switch'><input type='checkbox' checked onchange=\"onToggleChange('{DEV_PORT}', this)\"><span class='slider round'></span></label></div>";


DeviceManager::DeviceManager(ESP8266WebServer* server, Device* devices)
{
    _server = server;
    _devices = devices;
}

void DeviceManager::initialize()
{
    initializePorts();
    createRoutes();

}

void DeviceManager::initializePorts() {
  for (uint8_t i = 0; i < sizeof(_devices)/4; i++) {
    pinMode(_devices[i].port, OUTPUT);
    digitalWrite(_devices[i].port, _devices[i].state);
  }
}

void DeviceManager::createRoutes() 
{
  _server->on("/", [this]() {
      handleRoot();
  });

  _server->on("/turn/on", [this]() {
    handleTurnDevices(true);
  });

  _server->on("/turn/off", [this]() {
    handleTurnDevices(false);
  });

  _server->on("/devices", [this]() {
      showAllDevices();
  });

  _server->onNotFound([this]() {
      handleNotFound();
  });
}

void DeviceManager::showAllDevices() {

  String message = "Devices:\n\n";
  for (uint8_t i = 0; i < sizeof(_devices)/4; i++) {
    message += " " + _devices[i].name + " Port: " + _devices[i].port + " State: " + _devices[i].state + "\n";
  }

  _server->send(200, "text/plain", message);
}

void DeviceManager::controlPort(int port, bool state) {
  digitalWrite(port, state);
}

void DeviceManager::handleRoot() {
  String all_dev_elems = "";
  for (uint8_t i = 0; i < sizeof(_devices)/4; i++) {
    String dev = DEVICE_ELEM;
    String state = getState(_devices[i].state);
    String stateLabel = getState(!_devices[i].state);
    dev.replace("{DEVICE_NAME}", _devices[i].name);
    dev.replace("{DEV_STATE}", state);
    dev.replace("{DEV_PORT}", String(_devices[i].port, DEC));
    stateLabel.toUpperCase();
    dev.replace("{DEV_STATE_LABEL}", stateLabel);
    
    all_dev_elems += dev;
  }
  String page = MAIN_PAGE;
  page.replace("{DEVICE_ELEM}", all_dev_elems);
  _server->send(200, "text/html", MAIN_PAGE);

  //Serial.println("Handle root");
  
}

String DeviceManager::getState(bool state) {
  if (state) {
    return String("on");
  }

  return String("off");
}

void DeviceManager::handleTurnDevices(bool state) {
  
  if (_server->hasArg("port")) {
    uint8_t index = findDeviceIndexByPort(_server->arg("port").toInt());
    _devices[index].state = !_devices[index].state;
    controlPort(_devices[index].port, _devices[index].state);
    handleRoot();
  } else {
    _server->send(200, "text/plain", "no port found");
  }
}

uint8_t DeviceManager::findDeviceIndexByPort(int port) {
  for (uint8_t i = 0; i < sizeof(_devices)/4; i++) {
    if (_devices[i].port == port) {
      return i;
    }
  }
}

void DeviceManager::handleNotFound() {
  String message = "File Not Found\n\n";
  message += "URI: ";
  message += _server->uri();
  message += "\nMethod: ";
  message += (_server->method() == HTTP_GET) ? "GET" : "POST";
  message += "\nArguments: ";
  message += _server->args();
  message += "\n";
  for (uint8_t i = 0; i < _server->args(); i++) {
    message += " " + _server->argName(i) + ": " + _server->arg(i) + "\n";
  }
  _server->send(404, "text/plain", message);
}

