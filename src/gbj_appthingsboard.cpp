#include "gbj_appthingsboard.h"

gbj_appthingsboard::ResultCodes gbj_appthingsboard::connect()
{
  // No Wifi
  if (!WiFi.isConnected())
  {
    return setLastResult(ResultCodes::ERROR_CONNECT);
  }
  // Call callback just once since connection lost
  if (status_.flConnGain)
  {
    SERIAL_TITLE("Connection lost")
    status_.flConnGain = false;
    status_.flSubscribed = false;
    status_.flStatics = false;
    if (handlers_.onDisconnect)
    {
      handlers_.onDisconnect();
    }
  }
  // Calculate waiting period
  unsigned long period;
  if (status_.fails < Params::PARAM_CONN_1)
  {
    period = Timing::PERIOD_CONN_1;
    status_.stage = 1;
  }
  else if (status_.fails < Params::PARAM_CONN_2)
  {
    period = Timing::PERIOD_CONN_2;
    status_.stage = 2;
  }
  else if (status_.fails < Params::PARAM_CONN_3)
  {
    period = Timing::PERIOD_CONN_3;
    status_.stage = 3;
  }
  // Repeate connection cycle
  else
  {
    period = Timing::PERIOD_CONN_1;
    status_.cycles++;
    status_.stage = 1;
    status_.fails = 0;
    // Fallback
    if (status_.cycles >= Params::PARAM_CYCLES)
    {
      status_.server++;
      if (status_.server < status_.ips)
      {
        // Next server address
        status_.cycles = 0;
      }
      else
      {
        // Restart MCU
        SERIAL_TITLE("MCU Restart")
        if (handlers_.onRestart)
        {
          handlers_.onRestart();
        }
        ESP.restart();
      }
    }
    setLastResult(ResultCodes::ERROR_CONNECT);
  }
  // Wait for next connection
  if (status_.tsRetry && millis() - status_.tsRetry < period)
  {
    return setLastResult(ResultCodes::ERROR_NOINIT);
  }
  // Start connection
  if (handlers_.onConnectStart)
  {
    handlers_.onConnectStart();
  }
  SERIAL_ACTION("Connection to TB...")
  // Successful connection
  if (thingsboard_->connect(servers_[status_.server], token_))
  {
    SERIAL_ACTION_END("Success")
    SERIAL_VALUE("stage", status_.stage)
    SERIAL_VALUE("fails", status_.fails)
    SERIAL_VALUE("cycles", status_.cycles)
    SERIAL_VALUE("server", servers_[status_.server])
    SERIAL_VALUE("token", token_)
    if (handlers_.onConnectSuccess)
    {
      handlers_.onConnectSuccess();
    }
    status_.init();
    setLastResult(ResultCodes::SUCCESS);
  }
  // Failed connection
  else
  {
    status_.tsRetry = millis();
    status_.fails++;
    SERIAL_ACTION_END("Fail")
    SERIAL_VALUE("stage", status_.stage)
    SERIAL_VALUE("fails", status_.fails)
    SERIAL_VALUE("cycles", status_.cycles)
    SERIAL_VALUE("server", status_.server)
    status_.flSubscribed = false;
    if (handlers_.onConnectFail)
    {
      handlers_.onConnectFail();
    }
    setLastResult(ResultCodes::ERROR_CONNECT);
  }
  return getLastResult();
}

gbj_appthingsboard::ResultCodes gbj_appthingsboard::subscribe()
{
  if (callbacks_size_ == 0 || !isConnected() || isSubscribed())
  {
    return setLastResult();
  }
  // All consequent data processing will happen in callbacks as denoted by
  // callbacks[] array.
  if (thingsboard_->RPC_Subscribe(callbacks_, callbacks_size_))
  {
    SERIAL_TITLE("RPC subscribed")
    status_.flSubscribed = true;
    if (handlers_.onSubscribeSuccess)
    {
      handlers_.onSubscribeSuccess();
    }
    return setLastResult();
  }
  else
  {
    SERIAL_TITLE("RPC subscription failed")
    if (handlers_.onSubscribeFail)
    {
      handlers_.onSubscribeFail();
    }
    return setLastResult(ResultCodes::ERROR_SUBSCRIBE);
  }
}
