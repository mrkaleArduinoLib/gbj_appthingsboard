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
    server_ = server;
    token_ = token;
    timer_ = new gbj_timer(0);
    attribsChangeStatic_ = true; // Init publishing of static attributes
    attribsChangeDynamic_ = true; // Init publishing of dynamic attributes
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
    wifi_ = wifi;
  }

  inline void callbacks(RPC_Callback *callbacks = 0, size_t callbacks_size = 0)
  {
    callbacks_ = callbacks;
    _callbacks_size = callbacks_size;
    SERIAL_VALUE("callbacks", _callbacks_size);
  }

  inline void run()
  {
    if (timer_->run())
    {
      // Check external handlers
      if (!wifi_->isConnected())
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
      // Publish static client attributes at change
      if (isSuccess() && attribsChangeStatic_)
      {
        publishAttribsStatic();
        if (isSuccess())
        {
         attribsChangeStatic_ = false;
        }
      }
      // Publish dynamic client attributes at change
      if (isSuccess() && attribsChangeDynamic_)
      {
        publishAttribsDynamic();
        if (isSuccess())
        {
          attribsChangeDynamic_ = false;
        }
      }
      // Publish telemetry
      if (isSuccess())
      {
        publishMeasures();
      }
      // Error
      if (isError())
      {
        SERIAL_VALUE("error", getLastResult());
      }
    }
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
    SERIAL_ACTION("publishMeasure: ");
    SERIAL_CHAIN2(key, "...");
    if (thingsboard_->sendTelemetryData(key, value))
    {
      SERIAL_ACTION_END("OK");
      return setLastResult();
    }
    else
    {
      // ThingsBoard outputs error message with EOL
      SERIAL_VALUE("publishMeasure", "Error");
      return setLastResult(ResultCodes::ERROR_PUBLISH);
    }
  }

  inline ResultCodes publishMeasuresBatch(const Telemetry *data,
                                         size_t data_count)
  {
    SERIAL_ACTION("publishMeasuresBatch");
    SERIAL_CHAIN3(" (", data_count, ")...");
    if (thingsboard_->sendTelemetry(data, data_count))
    {
      SERIAL_ACTION_END("OK");
      return setLastResult();
    }
    else
    {
      // ThingsBoard outputs error message with EOL
      SERIAL_VALUE("publishMeasuresBatch", "Error");
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
    SERIAL_ACTION("publishAttrib: ");
    SERIAL_CHAIN2(attrName, "...");
    if (thingsboard_->sendAttributeData(attrName, value))
    {
      SERIAL_ACTION_END("OK");
      return setLastResult();
    }
    else
    {
      // ThingsBoard outputs error message with EOL
      SERIAL_VALUE("publishAttrib", "Error");
      return setLastResult(ResultCodes::ERROR_PUBLISH);
    }
  }

  inline ResultCodes publishAttribsBatch(const Attribute *data,
                                         size_t data_count)
  {
    SERIAL_ACTION("publishAttribsBatch");
    SERIAL_CHAIN3(" (", data_count, ")...");
    if (thingsboard_->sendAttributes(data, data_count))
    {
      SERIAL_ACTION_END("OK");
      return setLastResult();
    }
    else
    {
      // ThingsBoard outputs error message with EOL
      SERIAL_VALUE("publishAttribsBatch", "Error");
      return setLastResult(ResultCodes::ERROR_PUBLISH);
    }
  }

  // Abstract methods
  virtual ResultCodes publishMeasures() = 0;
  virtual ResultCodes publishAttribsStatic() = 0;
  virtual ResultCodes publishAttribsDynamic() = 0;
  virtual void setAttribChange(byte) = 0;

  // Setters
  inline void setPeriod(unsigned long period) { timer_->setPeriod(period); };
  inline void resetConnFail() { tbConnTime.isFail = false; }

  // Getters
  inline unsigned long getPeriod() { return timer_->getPeriod(); };
  inline bool isConnected() { return thingsboard_->connected(); }
  inline bool isSubscribed() { return subscribed_; }
  inline unsigned int getConnFailRetries() { return tbConnTime.rts; };
  inline unsigned int getConnFailErrors() { return tbConnTime.err; };
  inline unsigned int getConnFailCnt() { return tbConnTime.cnt; };
  inline unsigned long getConnFailCur() { return tbConnTime.cur; };
  inline unsigned long getConnFailMin() { return tbConnTime.min; };
  inline unsigned long getConnFailMax() { return tbConnTime.max; };
  inline bool getConnFail() { return tbConnTime.isFail; };

private:
  enum Timing : unsigned long
  {
    PERIOD_CONNECT = 500,
    PERIOD_RETRY = 300000,
  };
  enum Params : byte
  {
    PARAM_ATTEMPS = 5,
    PARAM_FAILS = 3,
  };
  struct Connection
  {
    unsigned int rts; // Retries (waits)
    unsigned int err; // Fails
    unsigned int cnt;
    unsigned long cur;
    unsigned long min = Timing::PERIOD_RETRY;
    unsigned long max = 0;
    bool isFail;
  } tbConnTime;
  size_t _callbacks_size;
  WiFiClient wificlient_;
  ThingsBoardSized<256, 16> *thingsboard_ =
    new ThingsBoardSized<256, 16>(wificlient_);
  const char *server_;
  const char *token_;
  bool subscribed_;
  bool attribsChangeDynamic_;
  bool attribsChangeStatic_;
  byte fails_ = Params::PARAM_FAILS;
  unsigned long tsRetry_ = millis();
  // Handlers
  RPC_Callback *callbacks_;
  // Methods
  ResultCodes connect();
  ResultCodes subscribe();

protected:
  gbj_timer *timer_;
  gbj_appwifi *wifi_;
  inline void setAttribChangeDynamic() { attribsChangeDynamic_ = true; }
  inline void startTimer(unsigned long period)
  {
    SERIAL_VALUE("startTimer", period);
    setPeriod(period);
    timer_->resume();
  }
};

#endif