#include "gbj_appthingsboard.h"
const String gbj_appthingsboard::VERSION = "GBJ_APPTHINGSBOARD 1.0.0";

gbj_appthingsboard::ResultCodes gbj_appthingsboard::connect()
{
  if (isConnected())
  {
    SERIAL_TITLE("Connected");
    return setLastResult();
  }
  uint8_t counter = Params::PARAM_ATTEMPS;
  SERIAL_ACTION("Connecting to TB...");
  _subscribed = false;
  if (_fails)
  {
    while (!_thingsboard->connect(_server, _token))
    {
      if (counter--)
      {
        delay(Timing::PERIOD_CONNECT);
      }
      else
      {
        SERIAL_ACTION_END("Timeout");
        _fails--;
        _tsRetry = millis();
        SERIAL_VALUE("fails", Params::PARAM_FAILS - _fails);
         return setLastResult(ResultCodes::ERROR_CONNECT);
      }
      SERIAL_DOT
    }
    SERIAL_ACTION_END("Connected");
    SERIAL_VALUE("server", _server);
    SERIAL_VALUE("token", _token);
    SERIAL_VALUE("fails", Params::PARAM_FAILS - _fails);
    _fails = Params::PARAM_FAILS;
    _tsRetry = millis();
    setLastResult();
  }
  else
  {
    SERIAL_ACTION_END("Ignored");
    setLastResult(ResultCodes::ERROR_CONNECT);
    // Retry connection after a while since recent connection or failure
    if (millis() - _tsRetry > Timing::PERIOD_RETRY)
    {
      SERIAL_TITLE("Reset retry");
      _fails = Params::PARAM_FAILS;
      _tsRetry = millis();
    }
  }
  return getLastResult();
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