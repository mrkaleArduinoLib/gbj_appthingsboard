#include "gbj_appthingsboard.h"
const String gbj_appthingsboard::VERSION = "GBJ_APPTHINGSBOARD 1.0.0";

gbj_appthingsboard::ResultCodes gbj_appthingsboard::connect()
{
  if (!wifi_->isConnected())
  {
    return setLastResult(ResultCodes::ERROR_CONNECT);
  }
  if (isConnected())
  {
    return setLastResult();
  }
  flSubscribed_ = false;
  if (fails_)
  {
    SERIAL_ACTION("Connecting to TB...");
    byte counter = Params::PARAM_ATTEMPS;
    unsigned long tsConnStart = millis();
    while (!thingsboard_->connect(server_, token_))
    {
      // Calculate connection statistics
      tbConnTime.cur = millis() - tsConnStart;
      tbConnTime.min = min(tbConnTime.min, tbConnTime.cur);
      tbConnTime.max = max(tbConnTime.max, tbConnTime.cur);
      tbConnTime.cnt++;
      tbConnTime.isFail = true;
      // Evaluate connection failure
      if (--counter == 0)
      {
        SERIAL_ACTION_END("Timeout");
        tsRetry_ = millis();
        fails_--;
        tbConnTime.err++;
        SERIAL_VALUE("fails", Params::PARAM_FAILS - fails_);
        return setLastResult(ResultCodes::ERROR_CONNECT);
      }
      SERIAL_DOT;
      tsConnStart = millis();
    }
    SERIAL_ACTION_END("Connected");
    SERIAL_VALUE("server", server_);
    SERIAL_VALUE("token", token_);
    SERIAL_VALUE("fails", Params::PARAM_FAILS - fails_);
    fails_ = Params::PARAM_FAILS;
    tsRetry_ = millis();
    return subscribe();
  }
  else
  {
    // Retry connection after a while since recent connection or failure
    if (millis() - tsRetry_ > Timing::PERIOD_RETRY)
    {
      SERIAL_TITLE("Reset retry");
      tbConnTime.rts++;
      fails_ = Params::PARAM_FAILS;
      tsRetry_ = millis();
    }
    return setLastResult(ResultCodes::ERROR_CONNECT);
  }
}

gbj_appthingsboard::ResultCodes gbj_appthingsboard::subscribe()
{
  if (callbacks_size_ == 0 || !isConnected() || isSubscribed())
  {
    return setLastResult();
  }
  // All consequent data processing will happen in callbacks as denoted by
  // callbacks[] array.
  SERIAL_ACTION("Subscribing for RPC...");
  if (thingsboard_->RPC_Subscribe(callbacks_, callbacks_size_))
  {
    SERIAL_ACTION_END("OK");
    flSubscribed_ = true;
    return setLastResult();
  }
  else
  {
    SERIAL_ACTION_END("Failed");
    return setLastResult(ResultCodes::ERROR_SUBSCRIBE);
  }
}
