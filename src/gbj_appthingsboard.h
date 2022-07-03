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

#if defined(ESP8266)
  #include <Arduino.h>
  #include <ESP8266WiFi.h>
#elif defined(ESP32)
  #include <Arduino.h>
  #include <WiFi.h>
#elif defined(PARTICLE)
  #include <Particle.h>
#else
  #error !!! Only platforms with WiFi are suppored !!!
#endif
#include "ThingsBoard.h"
#include "gbj_appbase.h"
#include "gbj_serial_debug.h"
#include "gbj_timer.h"

#undef SERIAL_PREFIX
#define SERIAL_PREFIX "gbj_appthingsboard"

class gbj_appthingsboard : public gbj_appbase
{
public:
  const char *VERSION = "GBJ_APPTHINGSBOARD 1.10.0";

  typedef void Handler();

  struct Handlers
  {
    Handler *onPublish;
    Handler *onConnectStart;
    Handler *onConnectTry;
    Handler *onConnectSuccess;
    Handler *onConnectFail;
    Handler *onDisconnect;
    Handler *onSubscribeSuccess;
    Handler *onSubscribeFail;
    Handler *onRestart;
  };

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
    server_fallback - Alternative IP address of a ThingsBoard server.
      - Data type: constant string
      - Default value: none
      - Limited range: none
    token - Device authorization token.
      - Data type: constant string
      - Default value: none
      - Limited range: none
    handlers - A structure with pointers to various callback handler functions.
      - Data type: Handlers
      - Default value: structure with zeroed all handlers
      - Limited range: system address range

    RETURN: object
  */
  inline gbj_appthingsboard(const char *server,
                            const char *server_fallback,
                            const char *token,
                            Handlers handlers = Handlers())
  {
    token_ = token;
    servers_[0] = server;
    servers_[1] = server_fallback;
    status_.ips = strlen(server_fallback) ? 2 : 1;
    handlers_ = handlers;
    timer_ = new gbj_timer(Timing::PERIOD_PUBLISH);
  }

  inline void callbacks(RPC_Callback *callbacks = 0, size_t callbacks_size = 0)
  {
    callbacks_ = callbacks;
    callbacks_size_ = callbacks_size;
    SERIAL_VALUE("callbacks", callbacks_size_)
  }

  inline void run()
  {
    if (isConnected())
    {
      if (!status_.flStatics)
      {
        publishAttribsStatic();
        status_.flStatics = isSuccess(); // Only at very beginning
      }
      publishAttribsDynamic();
      publishEvents();
      if (timer_->run())
      {
        if (handlers_.onPublish)
        {
          handlers_.onPublish();
        }
        publishMeasures();
      }
    }
    else
    {
      connect();
    }
    subscribe();
    // General loop delay. If zero, connecting to WiFi AP will timeout.
    delay(20);
    thingsboard_->loop();
  }

  /*
    Publish telemetry data item.

    DESCRIPTION:
    Method publishes one data item as a telemetry measure.
    - Method is templated by the data type of a measure's value.

    PARAMETERS:
    key - A pointer to the constant string denoting the name of a measure.
      - Data type: const char
      - Default value: none
      - Limited range: system address space

    value - Value of the measure.
      - Data type: Any of int, bool, float, const char*
      - Default value: none
      - Limited range: none

    RETURN: object
  */
  template<class T>
  inline ResultCodes publishMeasure(const char *key, T value)
  {
    SERIAL_ACTION("publishMeasure: ")
    SERIAL_CHAIN2(key, "...")
    if (thingsboard_->sendTelemetryData(key, value))
    {
      SERIAL_ACTION_END("OK")
      return setLastResult();
    }
    else
    {
      // ThingsBoard outputs error message with EOL
      SERIAL_VALUE("publishMeasure", "Error")
      return setLastResult(ResultCodes::ERROR_PUBLISH);
    }
  }

  inline ResultCodes publishMeasuresBatch(const Telemetry *data,
                                          size_t data_count)
  {
    SERIAL_ACTION("publishMeasuresBatch")
    SERIAL_CHAIN3(" (", data_count, ")...")
    if (thingsboard_->sendTelemetry(data, data_count))
    {
      SERIAL_ACTION_END("OK")
      return setLastResult();
    }
    else
    {
      // ThingsBoard outputs error message with EOL
      SERIAL_VALUE("publishMeasuresBatch", "Error")
      return setLastResult(ResultCodes::ERROR_PUBLISH);
    }
  }

  /*
    Publish client attribute.

    DESCRIPTION:
    Method publishes one client attribute of a device.
    - Method is templated by the data type of an attribute's value.

    PARAMETERS:
    key - A pointer to the constant string denoting the name of an attribute.
      - Data type: const char
      - Default value: none
      - Limited range: system address space

    value - Value of the attribute.
      - Data type: Any of int, bool, float, const char*
      - Default value: none
      - Limited range: none

    RETURN: object
  */
  template<class T>
  inline ResultCodes publishAttrib(const char *attrName, T value)
  {
    SERIAL_ACTION("publishAttrib: ")
    SERIAL_CHAIN2(attrName, "...")
    if (thingsboard_->sendAttributeData(attrName, value))
    {
      SERIAL_ACTION_END("OK")
      return setLastResult();
    }
    else
    {
      // ThingsBoard outputs error message with EOL
      SERIAL_VALUE("publishAttrib", "Error")
      return setLastResult(ResultCodes::ERROR_PUBLISH);
    }
  }

  inline ResultCodes publishAttribsBatch(const Attribute *data,
                                         size_t data_count)
  {
    SERIAL_ACTION("publishAttribsBatch")
    SERIAL_CHAIN3(" (", data_count, ")...")
    if (thingsboard_->sendAttributes(data, data_count))
    {
      SERIAL_ACTION_END("OK")
      return setLastResult();
    }
    else
    {
      // ThingsBoard outputs error message with EOL
      SERIAL_VALUE("publishAttribsBatch", "Error")
      return setLastResult(ResultCodes::ERROR_PUBLISH);
    }
  }

  // Abstract methods
  virtual ResultCodes publishEvents() = 0;
  virtual ResultCodes publishMeasures() = 0;
  virtual ResultCodes publishAttribsStatic() = 0;
  virtual ResultCodes publishAttribsDynamic() = 0;

  // Set timer period inputed as unsigned long in milliseconds
  inline void setPeriod(unsigned long period)
  {
    timer_->setPeriod(period == 0 ? Timing::PERIOD_PUBLISH : period);
  }
  // Set timer period inputed as String in seconds
  inline void setPeriod(String period)
  {
    setPeriod(1000 * (unsigned long)period.toInt());
  }

  // Getters
  inline unsigned long getPeriod() { return timer_->getPeriod(); }
  inline byte getStage() { return status_.stage; }
  inline byte getFails() { return status_.fails; }
  inline byte getCycles() { return status_.cycles; }
  inline bool isConnected() { return thingsboard_->connected(); }
  inline bool isSubscribed() { return status_.flSubscribed; }
  inline const char *getServer() { return servers_[status_.server]; }

protected:
  gbj_timer *timer_;

private:
  enum Timing : unsigned long
  {
    PERIOD_PUBLISH = 12 * 1000,
    PERIOD_CONN_1 = 5 * 1000, // It is ThingsBoard timeout as well
    PERIOD_CONN_2 = 1 * 60 * 1000,
    PERIOD_CONN_3 = 5 * 60 * 1000,
  };
  enum Params : byte
  {
    PARAM_CONN_1 = 6, // 6 attemps with PERIOD_CONN1 delay
    PARAM_CONN_2 = 11, // 5 attemps with PERIOD_CONN2 delay
    PARAM_CONN_3 = 23, // 12 attemps with PERIOD_CONN3 delay
    PARAM_CYCLES = 3, // Failed cycles for fallback IP or MCU restart
  };
  struct Status
  {
    byte fails, cycles, stage;
    byte server, ips;
    unsigned long tsRetry;
    bool flConnGain, flSubscribed, flStatics;
    void init()
    {
      fails = cycles = stage = tsRetry = 0;
      flConnGain = true;
    }
  } status_;
  size_t callbacks_size_;
  WiFiClient wificlient_;
  ThingsBoardSized<256, 16> *thingsboard_ =
    new ThingsBoardSized<256, 16>(wificlient_);
  const char *servers_[2];
  const char *token_;
  // Handlers
  Handlers handlers_;
  RPC_Callback *callbacks_;
  // Methods
  ResultCodes connect();
  ResultCodes subscribe();
};

#endif