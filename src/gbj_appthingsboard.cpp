#include "gbj_appthingsboard.h"
const String gbj_appthingsboard::VERSION = "GBJ_APPTHINGSBOARD 1.0.0";

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
    if (handlers_.onDisconnect)
    {
      handlers_.onDisconnect();
    }
  }
  // Wait for recovery period after failed connection
  if (status_.tsRetry && millis() - status_.tsRetry <
                           (status_.fails > PARAM_FAILS ? Timing::PERIOD_PROLONG
                                                        : Timing::PERIOD_CYCLE))
  {
    return setLastResult(ResultCodes::ERROR_NOINIT);
  }
  if (handlers_.onConnectStart)
  {
    handlers_.onConnectStart();
  }
  SERIAL_ACTION("Connection to TB...")
  byte counter = Params::PARAM_TRIES;
  while (!thingsboard_->connect(server_, token_) && counter--)
  {
    SERIAL_DOT
    delay(Timing::PERIOD_FAIL);
  }
  // Successful connection
  if (thingsboard_->connected())
  {
    SERIAL_ACTION_END("Success")
    SERIAL_VALUE("tries", Params::PARAM_TRIES - counter + 1)
    SERIAL_VALUE("fails", status_.fails)
    SERIAL_VALUE("server", server_)
    SERIAL_VALUE("token", token_)
    status_.reset();
    status_.flConnGain = true;
    if (handlers_.onConnectSuccess)
    {
      handlers_.onConnectSuccess();
    }
    setLastResult(ResultCodes::SUCCESS);
  }
  // Failed connection
  else
  {
    status_.tsRetry = millis();
    status_.fails++;
    SERIAL_ACTION_END("Fail")
    SERIAL_VALUE("fails", status_.fails)
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
