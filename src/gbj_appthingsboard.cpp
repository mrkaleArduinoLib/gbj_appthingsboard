#include "gbj_appthingsboard.h"
const String gbj_appthingsboard::VERSION = "GBJ_APPTHINGSBOARD 1.3.0";

gbj_appthingsboard::ResultCodes gbj_appthingsboard::connect()
{
  // No Wifi
  if (!wifi_->isConnected())
  {
    return setLastResult(ResultCodes::ERROR_CONNECT);
  }
  // Call callback just once since connection lost
  if (status_.flConnGain)
  {
    SERIAL_TITLE("Connection lost")
    status_.flConnGain = false;
    status_.flSubscribed = false;
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
  if (thingsboard_->connect(server_, token_))
  {
    SERIAL_ACTION_END("Success")
    SERIAL_VALUE("stage", status_.stage)
    SERIAL_VALUE("fails", status_.fails)
    SERIAL_VALUE("cycles", status_.cycles)
    SERIAL_VALUE("server", server_)
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
