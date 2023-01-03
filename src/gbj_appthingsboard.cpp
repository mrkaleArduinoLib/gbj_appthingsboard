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
  // Wait for connection
  if (status_.tsRetry && millis() - status_.tsRetry < Timing::PERIOD_CONN)
  {
    return setLastResult(ResultCodes::ERROR_NOINIT);
  }
  // Start connection
  if (handlers_.onConnectStart)
  {
    handlers_.onConnectStart();
  }
  SERIAL_VALUE("Connecting to", "ThingsBoard")
  // Successful connection
  if (thingsboard_->connect(server_, token_))
  {
    SERIAL_VALUE("Connection", "Success")
    SERIAL_VALUE("Server", server_)
    SERIAL_VALUE("Token", token_)
    SERIAL_VALUE("fails", status_.fails)
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
    SERIAL_VALUE("Connection", "Fail")
    SERIAL_VALUE("Server", server_)
    SERIAL_VALUE("fails", status_.fails)
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
