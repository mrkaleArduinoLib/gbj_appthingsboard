/*
  NAME:
  gbj_appthingsboard

  DESCRIPTION:
  Application library for processing connection to the ThingsBoard IoT platform.

  LICENSE:
  This program is free software; you can redistribute it and/or modify
  it under the terms of the license GNU GPL v3
  http://www.gnu.org/licenses/gpl-3.0.html (related to original code) and MIT
  License (MIT) for added code.

  CREDENTIALS:
  Author: Libor Gabaj
 */
#ifndef GBJ_APPTHINGSBOARD_H
#define GBJ_APPTHINGSBOARD_H

#if defined(__AVR__)
  #include <Arduino.h>
  #include <inttypes.h>
#elif defined(ESP8266)
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <Arduino.h>
  #include <WiFi.h>
#elif defined(PARTICLE)
  #include <Particle.h>
#endif
#include "ThingsBoard.h"
#include "gbj_appbase.h"
#include "gbj_appwifi.h"
#include "gbj_serial_debug.h"
#include "gbj_timer.h"

#undef SERIAL_PREFIX
#define SERIAL_PREFIX "gbj_appthingsboard"

class gbj_appthingsboard : public gbj_appbase
{
public:
  static const String VERSION;

  /*
    Constructor.

    DESCRIPTION:
    Constructor creates the class instance object and sets credentials for
    IoT platform.

    PARAMETERS:
    server - IP address of a ThingsBoard server.
      - Data type: constant string
      - Default value: none
      - Limited range: none

    token - Device authorization token.
      - Data type: constant string
      - Default value: none
      - Limited range: none

    RETURN: object
  */
  inline gbj_appthingsboard(const char *server, const char *token)
  {
    _server = server;
    _token = token;
    _thingsboard = new ThingsBoard(_wificlient);
    _timer = new gbj_timer(0);
  }

  /*
    Initialization.

    DESCRIPTION:
    The method should be called in the SETUP section of a sketch.

    PARAMETERS:
    wifi - A pointer to the instance object for processing WiFi connection.
      - Data type: gbj_appwifi
      - Default value: none
      - Limited range: system address space

    RETURN: Result code.
  */
  inline void begin(gbj_appwifi *wifi)
  {
    SERIAL_TITLE("begin");
    _wifi = wifi;
    setAttribsChange(); // For initial attributes publishing
  }

  inline void callbacks(RPC_Callback *callbacks = 0, size_t callbacks_size = 0)
  {
    SERIAL_TITLE("callbacks");
    _callbacks = callbacks;
    _callbacks_size = callbacks_size;
  }

  inline void run()
  {
    if (_timer->run())
    {
      // Check external handlers
      if (!_wifi->isConnected())
      {
        SERIAL_VALUE("run", "No Wifi");
        return;
      }
      setLastResult();
      // Connect
      if (isSuccess())
      {
        connect();
      }
      // Subscribe
      if (isSuccess())
      {
        subscribe();
      }
      // Publish client attributes at change
      if (isSuccess())
      {
        publishAttribs();
      }
      // Publish telemetry
      if (isSuccess())
      {
        publishData();
      }
      // Error
      if (isError())
      {
        SERIAL_VALUE("error", getLastResult());
      }
    }
    // General loop delay. If zero, connecting to WiFi AP will timeout.
    delay(20);
    _thingsboard->loop();
  }

  template<class T>
  ResultCodes publishDataItem(const char *key, T value)
  {
    SERIAL_ACTION("publishDataItem...")
    if (_thingsboard->sendTelemetryData(key, value))
    {
      SERIAL_ACTION_END("OK");
      return setLastResult();
    }
    else
    {
      SERIAL_ACTION_END("Error");
      return setLastResult(ResultCodes::ERROR_PUBLISH);
    }
  }

  template<class T>
  ResultCodes publishAttrib(const char *attrName, T value)
  {
    SERIAL_ACTION("publishAttrib...");
    if (_thingsboard->sendAttributeData(attrName, value))
    {
      SERIAL_ACTION_END("OK");
      return setLastResult();
    }
    else
    {
      SERIAL_ACTION_END("Error");
      return setLastResult(ResultCodes::ERROR_PUBLISH);
    }
  }

  // Abstract methods
  virtual ResultCodes publishData() = 0;
  virtual ResultCodes publishAttribs() = 0;

  // Setters
  inline void setPeriod(unsigned long period) { _timer->setPeriod(period); };

  // Getters
  inline unsigned long getPeriod() { return _timer->getPeriod(); };
  inline bool isConnected() { return _thingsboard->connected(); }
  inline bool isSubscribed() { return _subscribed; }

private:
  enum Timing : unsigned int
  {
    PERIOD_ATTEMPS = 10,
    PERIOD_CONNECT = 500,
  };
  size_t _callbacks_size;
  WiFiClient _wificlient;
  const char *_server;
  const char *_token;
  bool _subscribed;
  bool _attribsChange;
  // Handlers
  ThingsBoard *_thingsboard;
  RPC_Callback *_callbacks;
  // Methods
  ResultCodes connect();
  ResultCodes subscribe();

protected:
  gbj_timer *_timer;
  gbj_appwifi *_wifi;
  inline void startTimer(unsigned long period)
  {
    setPeriod(period);
    _timer->resume();
  }
  inline void setAttribsChange() { _attribsChange = true; }
  inline void resetAttribsChange() { _attribsChange = false; }
  inline bool isAttribsChange() { return _attribsChange; }
};

#endif