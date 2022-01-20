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
#include "config_params.h"
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
    callbacks_size_ = callbacks_size;
    SERIAL_VALUE("callbacks", callbacks_size_);
  }

  inline void run()
  {
    connect();
    if (isConnected())
    {
      if (!flStaticsPublished_)
      {
        publishAttribsStatic();
        flStaticsPublished_ = isSuccess(); // Only at very beginning
      }
      publishAttribsDynamic();
      publishEvents();
    }
    // Publish telemetry
    if (timer_->run())
    {
      if (isConnected())
      {
        publishMeasures();
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
  virtual ResultCodes publishEvents() = 0;
  virtual ResultCodes publishMeasures() = 0;
  virtual ResultCodes publishAttribsStatic() = 0;
  virtual ResultCodes publishAttribsDynamic() = 0;

  // Setters
  inline void setPeriod(unsigned long period) { timer_->setPeriod(period); };
  inline void resetConnFail() { tbConnTime.isFail = false; }

  // Getters
  inline unsigned long getPeriod() { return timer_->getPeriod(); };
  inline bool isConnected() { return thingsboard_->connected(); }
  inline bool isSubscribed() { return flSubscribed_; }
  inline unsigned int getConnFailRetries() { return tbConnTime.rts; };
  inline unsigned int getConnFailErrors() { return tbConnTime.err; };
  inline unsigned int getConnFailCnt() { return tbConnTime.cnt; };
  inline unsigned long getConnFailCur() { return tbConnTime.cur; };
  inline unsigned long getConnFailMin() { return tbConnTime.min; };
  inline unsigned long getConnFailMax() { return tbConnTime.max; };
  inline bool getConnFail() { return tbConnTime.isFail; };

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
    bool ignore;
    bool used;
    Parameter()
      : name(NULL)
      , type(Datatype::TYPE_NONE)
      , val()
    {
    }
    Parameter(const char *key)
      : name(key)
      , type(Datatype::TYPE_NONE)
      , val()
    {
    }
    Parameter(const char *key, const char *value)
      : name(key)
      , type(Datatype::TYPE_STR)
    {
      val.str = value;
    }
    Parameter(const char *key, String value)
      : name(key)
      , type(Datatype::TYPE_STR)
    {
      val.str = value.c_str();
    }
    Parameter(const char *key, bool value)
      : name(key)
      , type(Datatype::TYPE_BOOL)
    {
      val.boolean = value;
    }
    Parameter(const char *key, int value)
      : name(key)
      , type(Datatype::TYPE_INT)
    {
      val.integer = value;
    }
    Parameter(const char *key, long value)
      : name(key)
      , type(Datatype::TYPE_INT)
    {
      val.integer = value;
    }
    Parameter(const char *key, unsigned int value)
      : name(key)
      , type(Datatype::TYPE_UINT)
    {
      val.big = value;
    }
    Parameter(const char *key, unsigned long value)
      : name(key)
      , type(Datatype::TYPE_UINT)
    {
      val.big = value;
    }
    Parameter(const char *key, float value)
      : name(key)
      , type(Datatype::TYPE_REAL)
    {
      val.real = value;
    }
    bool getIgnore() { return ignore; }
    void setIgnore() { ignore = true; }
    void resetIgnore() { ignore = false; }
    void set()
    {
      type = Datatype::TYPE_NONE;
      ignore = false;
    }
    void set(const char *value)
    {
      type = Datatype::TYPE_STR;
      ignore = used && (val.str == value);
      val.str = value;
    }
    void set(String value)
    {
      type = Datatype::TYPE_STR;
      ignore = used && (val.str == value.c_str());
      val.str = value.c_str();
    }
    void set(bool value)
    {
      type = Datatype::TYPE_BOOL;
      ignore = used && (val.boolean == value);
      val.boolean = value;
    }
    void set(int value)
    {
      type = Datatype::TYPE_INT;
      ignore = used && (val.integer == value);
      val.integer = value;
    }
    void set(long value)
    {
      type = Datatype::TYPE_INT;
      ignore = used && (val.integer == value);
      val.integer = value;
    }
    void set(unsigned int value)
    {
      type = Datatype::TYPE_UINT;
      ignore = used && (val.big == value);
      val.integer = value;
    }
    void set(unsigned long value)
    {
      type = Datatype::TYPE_UINT;
      ignore = used && (val.big == value);
      val.integer = value;
    }
    void set(float value)
    {
      type = Datatype::TYPE_REAL;
      ignore = used && (val.real == value);
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
          result = NA;
          break;
      }
      used = true;
      return result;
    }
  };
  //****************************************************************************
  // Parameters definition
  //****************************************************************************
  // Generic parameters initiated at compile time.
  // Compiler build macros or included static data.
  Parameter version = Parameter(versionStatic, MAIN_VERSION);
  Parameter broker = Parameter(brokerStatic, BROKER);
  Parameter portOTA = Parameter(portOTAStatic, OTA_PORT);

  // Generic parameters initiated at boot once
  Parameter hostname = Parameter(hostnameStatic);
  Parameter addressIP = Parameter(addressIPStatic);
  Parameter addressMAC = Parameter(addressMACStatic);

  // Generic EEPROM parameters updated at run time immediatelly
  Parameter periodPublish = Parameter(periodPublishPrm);
  Parameter mcuRestarts = Parameter(mcuRestartsPrm);

  // Telemetry parameters updated periodically
  Parameter rssi = Parameter(rssiTelem);
  Parameter connWifi = Parameter(connWifiTelem);
  Parameter connThingsboard = Parameter(connThingsboardTelem);
  // Telemetry -- ThingsBoard connection
  Parameter connRetries = Parameter(connRetriesTelem);
  Parameter connErrors = Parameter(connErrorsTelem);
  Parameter connCnt = Parameter(connCntTelem);
  Parameter connCur = Parameter(connCurTelem);
  Parameter connMin = Parameter(connMinTelem);
  Parameter connMax = Parameter(connMaxTelem);
  //****************************************************************************
  gbj_timer *timer_;
  gbj_appwifi *wifi_;
  inline void startTimer(unsigned long period)
  {
    SERIAL_VALUE("startTimer", period);
    setPeriod(period);
    timer_->resume();
  }
  char progmemBuffer_[progmemBufferLen];
  inline const char *getPrmName(const char *progmemPrmName)
  {
    strcpy_P(progmemBuffer_, progmemPrmName);
    return progmemBuffer_;
  }

private:
  enum Timing : unsigned long
  {
    PERIOD_CONNECT = 500,
    PERIOD_RETRY = 5 * 60 * 1000,
  };
  enum Params : byte
  {
    PARAM_ATTEMPS = 3,
    PARAM_FAILS = 2,
  };
  struct Connection
  {
    unsigned int rts; // Retries (waits)
    unsigned int err; // Fails
    unsigned int cnt;
    unsigned long cur;
    unsigned long max;
    unsigned long min = Timing::PERIOD_RETRY;
    bool isFail;
    void reset()
    {
      rts = err = cnt = cur = max = 0;
      min = Timing::PERIOD_RETRY;
      isFail = false;
    }
  } tbConnTime;
  size_t callbacks_size_;
  WiFiClient wificlient_;
  ThingsBoardSized<256, 16> *thingsboard_ =
    new ThingsBoardSized<256, 16>(wificlient_);
  const char *server_;
  const char *token_;
  bool flSubscribed_;
  bool flStaticsPublished_;
  byte fails_ = Params::PARAM_FAILS;
  unsigned long tsRetry_;
  // Handlers
  RPC_Callback *callbacks_;
  // Methods
  ResultCodes connect();
  ResultCodes subscribe();
};

#endif