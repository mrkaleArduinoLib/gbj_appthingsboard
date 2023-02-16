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
  const char *VERSION = "GBJ_APPTHINGSBOARD 1.14.0";

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
                            const char *token,
                            Handlers handlers = Handlers())
  {
    token_ = token;
    server_ = server;
    handlers_ = handlers;
    timer_ = new gbj_timer(Timing::PERIOD_PUBLISH);
  }

  inline void callbacks(const std::vector<RPC_Callback> &callbacks)
  {
    callbacks_ = callbacks;
    SERIAL_VALUE("callbacks", callbacks_.size())
  }

  inline void run()
  {
    if (isConnected())
    {
      ResultCodes result = setLastResult();
      if (!status_.flStatics)
      {
        publishAttribsStatic();
        // Only at very beginning
        status_.flStatics = isSuccess();
        // Catch recent error code
        result = getLastResult() ? getLastResult() : result;
      }
      publishAttribsDynamic();
      result = getLastResult() ? getLastResult() : result;
      publishEvents();
      result = getLastResult() ? getLastResult() : result;
      if (timer_->run())
      {
        if (handlers_.onPublish)
        {
          handlers_.onPublish();
        }
        publishMeasures();
        result = getLastResult() ? getLastResult() : result;
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
  virtual void publishEvents() = 0;
  virtual void publishMeasures() = 0;
  virtual void publishAttribsStatic() = 0;
  virtual void publishAttribsDynamic() = 0;

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
  inline byte getFails() { return status_.fails; }
  inline bool isConnected() { return thingsboard_->connected(); }
  inline bool isSubscribed() { return status_.flSubscribed; }
  inline const char *getServer() { return server_; }

protected:
  gbj_timer *timer_;

private:
  enum Timing : unsigned long
  {
    PERIOD_PUBLISH = 12 * 1000,
    PERIOD_CONN = 1 * 60 * 1000,
  };
  struct Status
  {
    byte fails;
    unsigned long tsRetry;
    bool flConnGain, flSubscribed, flStatics;
    void init()
    {
      fails = tsRetry = 0;
      flConnGain = true;
    }
  } status_;
  WiFiClient wificlient_;
  ThingsBoardSized<256, 16> *thingsboard_ =
    new ThingsBoardSized<256, 16>(wificlient_);
  const char *server_, *token_;
  // Handlers
  Handlers handlers_;
  std::vector<RPC_Callback> callbacks_;
  // Methods
  ResultCodes connect();
  ResultCodes subscribe();
};

#endif