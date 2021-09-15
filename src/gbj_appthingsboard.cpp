#include "gbj_appthingsboard.h"
const String gbj_appthingsboard::VERSION = "GBJ_APPTHINGSBOARD 1.0.0";

gbj_appthingsboard::ResultCodes gbj_appthingsboard::connect()
{
  if (isConnected())
  {
    return setLastResult();
  }
  uint8_t counter = Timing::PERIOD_ATTEMPS;
  SERIAL_ACTION("Connecting to TB...");
  _subscribed = false;
  while (!_thingsboard->connect(_server, _token))
  {
    if (counter--)
    {
      delay(Timing::PERIOD_CONNECT);
    }
    else
    {
      SERIAL_ACTION_END("Timeout");
      return setLastResult(ResultCodes::ERROR_CONNECT);
    }
    SERIAL_DOT
  }
  SERIAL_ACTION_END("Connected");
  SERIAL_VALUE("server", _server);
  SERIAL_VALUE("token", _token);
  return setLastResult();
}

gbj_appthingsboard::ResultCodes gbj_appthingsboard::subscribe()
{
  if (_callbacks_size == 0 || !isConnected() || isSubscribed())
  {
    return setLastResult();
  }
  // All consequent data processing will happen in callbacks as denoted by
  // callbacks[] array.
  SERIAL_ACTION("Subscribing for RPC...");
  _subscribed = false;
  if (_thingsboard->RPC_Subscribe(_callbacks, _callbacks_size))
  {
    SERIAL_ACTION_END("OK");
    _subscribed = true;
    return setLastResult();
  }
  else
  {
    SERIAL_ACTION_END("Failed");
    return setLastResult(ResultCodes::ERROR_SUBSCRIBE);
  }
}