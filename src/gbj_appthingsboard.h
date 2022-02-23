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
#include "config_params_gen.h"
#include "gbj_appbase.h"
#include "gbj_appwifi.h"
#include "gbj_serial_debug.h"
#include "gbj_timer.h"

#undef SERIAL_PREFIX
#define SERIAL_PREFIX "gbj_appthingsboard"

class gbj_appthingsboard : public gbj_appbase
{
public:
  const String VERSION = "GBJ_APPTHINGSBOARD 1.5.0";

  typedef void Handler();

  struct Handlers
  {
    Handler *onConnectStart;
    Handler *onConnectTry;
    Handler *onConnectSuccess;
    Handler *onConnectFail;
    Handler *onDisconnect;
    Handler *onSubscribeSuccess;
    Handler *onSubscribeFail;
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
    server_ = server;
    token_ = token;
    handlers_ = handlers;
    timer_ = new gbj_timer(Timing::PERIOD_PUBLISH);
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
    SERIAL_TITLE("begin")
    wifi_ = wifi;
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
      if (!flStaticsPublished_)
      {
        publishAttribsStatic();
        flStaticsPublished_ = isSuccess(); // Only at very beginning
      }
      publishAttribsDynamic();
      publishEvents();
      if (timer_->run())
      {
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

  // Setters
  inline void setPeriod(unsigned long period) { timer_->setPeriod(period); }

  // Getters
  inline unsigned long getPeriod() { return timer_->getPeriod(); }
  inline byte getStage() { return status_.stage; }
  inline byte getFails() { return status_.fails; }
  inline byte getCycles() { return status_.cycles; }
  inline const char *getServer() { return server_; }
  inline bool isConnected() { return thingsboard_->connected(); }
  inline bool isSubscribed() { return status_.flSubscribed; }

protected:
  enum Datatype
  {
    TYPE_NONE,
    TYPE_BOOL,
    TYPE_INT,
    TYPE_UINT,
    TYPE_REAL,
    TYPE_STR,
  };
  union Data
  {
    const char *str;
    bool boolean;
    long integer;
    unsigned long big;
    float real;
  };
  struct Parameter
  {
    const char *name;
    Datatype type;
    Data val;
    bool flAlways;
    bool flInit;
    bool flShow;
    Parameter(const char *key, bool always = false)
      : name(key)
      , type(Datatype::TYPE_NONE)
      , flAlways(always)
      , flInit(true)
    {
    }
    bool isReady() { return flShow; }
    void init() { flInit = true; }
    void hide() { flShow = false; }
    const char *getName() { return name; }
    void set(const char *value)
    {
      type = Datatype::TYPE_STR;
      flShow = flAlways || flInit || !(val.str == value);
      val.str = value;
    }
    void set(String value)
    {
      type = Datatype::TYPE_STR;
      flShow = flAlways || flInit || !(val.str == value.c_str());
      val.str = value.c_str();
    }
    void set(bool value)
    {
      type = Datatype::TYPE_BOOL;
      flShow = flAlways || flInit || !(val.boolean == value);
      val.boolean = value;
    }
    void set(int value)
    {
      type = Datatype::TYPE_INT;
      flShow = flAlways || flInit || !(val.integer == value);
      val.integer = value;
    }
    void set(long value)
    {
      type = Datatype::TYPE_INT;
      flShow = flAlways || flInit || !(val.integer == value);
      val.integer = value;
    }
    void set(unsigned int value)
    {
      type = Datatype::TYPE_UINT;
      flShow = flAlways || flInit || !(val.big == value);
      val.integer = value;
    }
    void set(unsigned long value)
    {
      type = Datatype::TYPE_UINT;
      flShow = flAlways || flInit || !(val.big == value);
      val.integer = value;
    }
    void set(float value)
    {
      type = Datatype::TYPE_REAL;
      flShow = flAlways || flInit || !(val.real == value);
      val.real = value;
    }
    String get()
    {
      String result;
      switch (type)
      {
        case Datatype::TYPE_STR:
          result = String(val.str);
          break;

        case Datatype::TYPE_BOOL:
          result = val.boolean ? SERIAL_F("true") : SERIAL_F("false");
          break;

        case Datatype::TYPE_INT:
          result = String(val.integer);
          break;

        case Datatype::TYPE_UINT:
          result = String(val.big);
          break;

        case Datatype::TYPE_REAL:
          result = String(val.real, 4);
          break;

        case Datatype::TYPE_NONE:
          result = "n/a";
          break;
      }
      flInit = false;
      return result;
    }
  };
  //****************************************************************************
  // Parameters definition
  //****************************************************************************
  // Static attributes initiated at boot once
  Parameter version = Parameter(versionStatic);
  Parameter broker = Parameter(brokerStatic);
  Parameter portOTA = Parameter(portOTAStatic);
  Parameter hostname = Parameter(hostnameStatic);
  Parameter mcuBoot = Parameter(mcuBootStatic);
  Parameter addressIP = Parameter(addressIpStatic);
  Parameter addressMAC = Parameter(addressMacStatic);

  // Dynamic attributes updated immediatelly (EEPROM)
  Parameter mcuRestarts = Parameter(mcuRestartsPrm);
  Parameter periodPublish = Parameter(periodPublishPrm);

  // Measures updated immediatelly (events)

  // Measures updated periodically (telemetry)
  Parameter rssi = Parameter(rssiTelem, true); // Publish always
  //****************************************************************************
  gbj_timer *timer_;
  gbj_appwifi *wifi_;
  char progmemBuffer_[progmemBufferLen];
  inline const char *getPrmName(const char *progmemPrmName)
  {
    strcpy_P(progmemBuffer_, progmemPrmName);
    return progmemBuffer_;
  }

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
    PARAM_CONN_1 = 6,
    PARAM_CONN_2 = 11,
    PARAM_CONN_3 = 23,
  };
  struct Status
  {
    byte fails, cycles, stage;
    unsigned long tsRetry;
    bool flConnGain, flSubscribed;
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
  const char *server_;
  const char *token_;
  bool flStaticsPublished_;
  // Handlers
  Handlers handlers_;
  RPC_Callback *callbacks_;
  // Methods
  ResultCodes connect();
  ResultCodes subscribe();
};

#endif