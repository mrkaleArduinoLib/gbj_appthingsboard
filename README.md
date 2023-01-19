<a id="library"></a>

# gbj\_appthingsboard
This is an application library, which is used usually as a project library for particular PlatformIO project. It encapsulates the functionality of `connection to ThingsBoard IoT platform`. The encapsulation provides following advantages:

* Functionality is hidden from the main sketch.
* The library follows the principle `separation of concerns`.
* The library is reusable for various projects without need to code connection management.
* Update in library is valid for all involved projects.
* It specifies (inherits from) the parent application library `gbj_appbase`.
* It utilizes funcionality and error handling from the parent class.


## Fundamental functionality
* The library utilizes internal timer for periodical data publishing to <abbr title="Internet of Things">IoT</abbr> platform.
* The connection to Wifi and to ThingsBoard IoT platform is checked at every loop of a main sketch.
* If no connection to IoT platform is detected, the library repeats connecting after [internal Connection waiting period](#internals) again. It is up to an application and/or an external watchdog timer to restart the microcontroller after some failed connection attempts.
* The library subscribes to externally defined (in a main sketch) <abbr title="Remote Procedure Call">RPC</abbr> functions to interact with the IoT platform.
* The class from the library is not intended to be used directly in a sketch, just as a parent class for project specific device libraries communicating with IoT platform, e.g., `apptb_device`.


<a id="internals"></a>

## Internal parameters
Internal parameters are hard-coded in the library as enumerations and none of them have setters or getters associated. The waiting for next connection attempt is in non-blocking mode. However, waiting on IoT platform connection `timeout is cca 5 seconds` in either case.

* **Publishing period** (`12 seconds`): It is a default time period for publishing data to IoT platform. Real publishing period is associated with corresponding setter and getter.
* **Connection waiting period** (`1 minute`): It is a time period during which the system is waiting for next attempt to connect to IoT platform. The real time period between failed connection attempts is increased by the IoT platform timeout cca `5 seconds`, so that the real time period among attempts is cca `65 seconds`.


<a id="dependency"></a>

## Dependency
* **gbj\_appbase**: Parent library for all application libraries loaded from the file `gbj_appbase.h`.
* **gbj\_serial\_debug**: Auxilliary library for debug serial output loaded from the file `gbj_serial_debug.h`. It enables to exclude serial outputs from final compilation.
* **gbj\_timer**: Library for executing internal timer within an instance object loaded from the file `gbj_timer.h`.
* **ThingsBoard**: Library for managing connection to IoT platform loaded from the file `ThingsBoard.h` in library `ThingsBoard-Arduino-MQTT-SDK`.

#### Espressif ESP8266 platform
* **Arduino.h**: Main include file for the Arduino platform.
* **ESP8266WiFi.h**: Main include file for the wifi connection needed for an external wifi object injection.

#### Espressif ESP32 platform
* **Arduino.h**: Main include file for the Arduino platform.
* **WiFi.h**: Main include file for the wifi connection needed for an external wifi object injection.

#### Particle platform
* **Particle.h**: Includes alternative (C++) data type definitions.

> Library is not intended to be utilized on platforms without WiFi capabality.


<a id="constants"></a>

## Constants
* **VERSION**: Name and semantic version of the library.

Other constants, enumerations, result codes, and error codes are inherited from the parent library.


<a id="interface"></a>

## Custom data types
* [Handler](#handler)
* [Handlers](#handlers)

## Interface
The methods in bold are virtual methods and should be implemented in a project specific libraries.

* [gbj_appthingsboard()](#gbj_appthingsboard)
* [callbacks()](#callbacks)
* [**run()**](#run)
* [**publishEvents()**](#publish)
* [publishMeasure()](#publishMeasure)
* [**publishMeasures()**](#publish)
* [publishMeasuresBatch()](#publishMeasuresBatch)
* [publishAttrib()](#publishAttrib)
* [publishAttribsBatch()](#publishAttribsBatch)
* [**publishAttribsStatic()**](#publish)
* [**publishAttribsDynamic()**](#publish)
* [**setPeriod()**](#setPeriod)
* [**getPeriod()**](#getPeriod)
* [getPrmName()](#getPrmName)
* [getFails()](#getFails)
* [getServer()](#getServer)
* [isConnected()](#isConnected)
* [isSubscribed()](#isSubscribed)


<a id="handler"></a>

## Handler

#### Description
The template or the signature of a callback function, which is called at particular event in the processing. It can be utilized for instant communicating with other modules of the application (project).
* A handler is just a bare function without any input parameters and returning nothing.
* A handler can be declared just as `void` type.

#### Syntax
    typedef void Handler()

#### Parameters
None

#### Returns
None

#### See also
[Handlers](#handlers)

[Back to interface](#interface)


<a id="handlers"></a>

## Handlers

#### Description
Structure of pointers to handlers each for particular event in processing.
* Individual or all handlers do not need to be defined in the main sketch, just those that are useful.

#### Syntax
    struct Handlers
    {
      Handler *onPublish;
      Handler *onConnectStart;
      Handler *onConnectTry;
      Handler *onConnectSuccess;
      Handler *onConnectFail;
      Handler *onDisconnect;
      Handler *onSubscribeSuccess;
      Handler *onSubscribeFail;
      Handler *onRestart;
    }

#### Parameters
* **onPublish**: Pointer to a callback function, which is called right before publishing telemetry measures. This handler is useful for gaining values of measures, for which there is no specific or individual process, e.g., <abbr title="Received Signal Strength Indicator">RSSI</abbr> value of WiFi connection, or which can be fetched from the system directly.
* **onConnectStart**: Pointer to a callback function, which is called right before a new connection set.
* **onConnectTry**: Pointer to a callback function, which is called after every failed connection attempt. It allows to observe the pending connection set.
* **onConnectSuccess**: Pointer to a callback function, which is called right after successful connection to IoT platform.
* **onConnectFail**: Pointer to a callback function, which is called right after failed connection set.
* **onDisconnect**: Pointer to a callback function, which is called at lost of connection to IoT platform. It allows to create an alarm or a signal about it.
* **onSubscribeSuccess**: Pointer to a callback function, which is called right after successful subscribing <abbr title="Remote Procedure Call">RPC</abbr> functions.
* **onSubscribeFail**: Pointer to a callback function, which is called right after failed subscribing <abbr title="Remote Procedure Call">RPC</abbr> functions.
* **onRestart**: Pointer to a callback function, which is called right before microcontroller restarts. It allows to do some actions related to it, e.g., increment and save number of restarts to the EEPROM.

#### Example
Instantiation of the library is only for illustration here. Use the appropriate child class of a project specific library instead.
```cpp
void onDeviceSuccess()
{
  ...
}
gbj_appthingsboard::Handlers handlersDevice = { .onConnectSuccess = onDeviceSuccess };
gbj_appthingsboard device = gbj_appthingsboard(..., handlersDevice);
```

#### See also
[Handler](#handler)

[gbj_appthingsboard](#gbj_appthingsboard)

[Back to interface](#interface)


<a id="gbj_appthingsboard"></a>

## gbj_appthingsboard()

#### Description
Constructor creates the class instance object and initiates internal resources.
* It inputs credentials for identifying a IoT Platform device.
* It creates an internal timer for periodical data publishing.

#### Syntax
    gbj_appthingsboard(const char *server, const char *token, Handlers handlers)

#### Parameters
* **server**: Pointer to an address of ThingsBoard server. It should be either IP address or web address.
  * *Valid values*: constant pointer to string
  * *Default value*: none


* **token**: Pointer to the authorization token of a ThingsBoard device.
  * *Valid values*: constant pointer to string
  * *Default value*: none


* **handlers**: Pointer to a structure of callback functions. This structure as well as handlers should be defined in the main sketch.
  * *Data type*: Handlers
  * *Default value*: empty structure

#### Returns
Object performing connection and reconnection to the IoT platform.

#### See also
[Handlers()](#handlers)

[Back to interface](#interface)


<a id="callbacks"></a>

## callbacks()

#### Description
The registration method for subscribing external functions as <abbr title="Remote Procedure Call">RPC</abbr> callbacks to the IoT platform.
* The method should be called in the setup section of a sketch.
* The method subscribes all external function in the input list.

#### Syntax
	void callbacks(RPC_Callback *callbacks = 0, size_t callbacks_size = 0)

#### Parameters
* **callbacks**: Array (list) of external function to be subscribed.
  * *Valid values*: pointer to an array of type `RPC_Callback`
  * *Default value*: 0


* **callbacks_size**: Number of callback functions in the list.
  * *Valid values*: positive integer
  * *Default value*: 0

#### Returns
None

#### Example
Initialization instance object in the sketch loop. Instantiation of the library is only for illustration here. Use the appropriate child class of a project specific library instead.
```cpp
RPC_Response processSetDelay(const RPC_Data &data){...}
RPC_Response processGetDelay(const RPC_Data &data){...}
RPC_Response processCommand(const RPC_Data &data){...}
const size_t callbacks_size = 3;
RPC_Callback callbacks[callbacks_size] = {
  { "setValue", processSetDelay },
  { "getValue", processGetDelay },
  { "rpcCommand", processCommand },
};
gbj_appthingsboard device = gbj_appthingsboard(...);
void setup()
{
  device.callbacks(callbacks, callbacks_size);
}
```

[Back to interface](#interface)


<a id="run"></a>

## run()

#### Description
The execution method as the implementation of the virtual method from parent class, which should be called frequently, usually in the loop function of a sketch.
* The method connects to the IoT platform at the very first calling it.
* At the start of each timer period the method checks the connection to the wifi network as well as to the IoT platform. If wifi network is available, it reconnects to the IoT platform if neccesary.
* If the serial connection is active, the library outputs flow of the connection and at success lists input credentials of the connection to the IoT platform.

#### Syntax
    void run()

#### Parameters
None

#### Returns
None

[Back to interface](#interface)


<a id="publishMeasure"></a>

## publishMeasure()

#### Description
The method publishes input key-value pair as the telemetry data item of the device to the IoT platform.
* The method is templated by input value data type.
* The method does not need to be called by templating syntax, because it is able to identify proper data type by data type of the just value argument.

#### Syntax
    template<class T>
    ResultCodes publishMeasure(const char *key, T value)

#### Parameters
* **key**: Pointer to a telemetry data item key (name). It should be in lower pascal case (camel case).
  * *Valid values*: pointer to a constant string
  * *Default value*: none


* **value**: Value of corresponding data type for a telemetry data item.
  * *Valid values*: any value of type `int`, `bool`, `float`, `const char*`
  * *Default value*: none

#### Returns
Some of [result or error codes](#constants) from the parent class.

#### See also
[publishMeasures()](#publish)

[publishMeasuresBatch()](#publishMeasuresBatch)

[Back to interface](#interface)


<a id="publishMeasuresBatch"></a>

## publishMeasuresBatch()

#### Description
The method publishes input array of key-value pairs as a batch of the telemetry data items of the device to the IoT platform.
* The key-value pair itself is an array with two items, where the first one is the key and second one is the value.

#### Syntax
    ResultCodes publishMeasuresBatch(const Telemetry *data, size_t data_count)

#### Parameters
* **data**: Pointer to an array of arrays with telemetry data item key-value pairs.
  * *Valid values*: pointer to a constant array of arrays
  * *Default value*: none


* **data_count**: Number of key-value pairs in input array.
  * *Valid values*: non-negative integer
  * *Default value*: none

#### Returns
Some of [result or error codes](#constants) from the parent class.

#### Example
```cpp
byte data_items = 2;
Telemetry data[data_items] = {
  { "temperature", 20.3 },
  { "humidity", 40 },
};
publishDataBatch(data, data_items);
```

#### See also
[publishMeasure()](#publishMeasure)

[publishMeasures()](#publish)

[Back to interface](#interface)


<a id="publishAttrib"></a>

## publishAttrib()

#### Description
The method publishes input key-value pair as the client attribute of the device to the IoT platform.
* The method is templated by input value data type.
* The method does not need to be called by templating syntax, because it is able to identify proper data type by data type of the just value argument.

#### Syntax
    template<class T>
    ResultCodes publishAttrib(const char *attrName, T value)

#### Parameters
* **attrName**: Pointer to a client attribute name. It should be in lower pascal case (camel case).
  * *Valid values*: pointer to a constant string
  * *Default value*: none


* **value**: Value of corresponding data type for an attribute.
  * *Valid values*: Any value of type `int`, `bool`, `float`, `const char*`
  * *Default value*: none

#### Returns
Some of [result or error codes](#constants) from the parent class.

#### See also
[publishAttribsStatic()](#publish)

[publishAttribsDynamic()](#publish)

[publishAttribsBatch()](#publishAttribsBatch)

[Back to interface](#interface)


<a id="publishAttribsBatch"></a>

## publishAttribsBatch()

#### Description
The method publishes input array of key-value pairs as a batch of the client attributes of the device to the IoT platform.
* The key-value pair itself is an array with two items, where the first one is the key and second one is the value.

#### Syntax
    ResultCodes publishAttribsBatch(const Attribute *data, size_t data_count)

#### Parameters
* **data**: Pointer to an array of arrays with client attributes key-value pairs.
  * *Valid values*: pointer to a constant array of arrays
  * *Default value*: none


* **data_count**: Number of key-value pairs in input array
  * *Valid values*: non-negative integer
  * *Default value*: none

#### Returns
Some of [result or error codes](#constants) from the parent class.

#### Example
```cpp
byte data_items = 2;
Attribute data[data_items] = {
  { "period", 5000 },
  { "factor", 0.2 },
};
publishAttribsBatch(data, data_items);
```

#### See also
[publishAttrib()](#publishAttrib)

[publishAttribsStatic()](#publish)

[publishAttribsDynamic()](#publish)

[Back to interface](#interface)


<a id="publish"></a>

## publishEvents(), publishMeasures(), publishAttribsStatic(), publishAttribsDynamic()

#### Description
The virtual methods that every child class derived from this library class should implement.

* The method `publishEvents()` is dedicated for telemetry data items of the device, which change occassionally and rarely. So that they are published individually at every change immediatelly by the multiple calls of the method [publishAttrib()](#publishAttrib).
* The method `publishMeasures()` is dedicated for telemetry data items of the device, which change frequently, so that they have to by published periodically by either multiple calls of the method [publishMeasure()](#publishMeasure) or the single call of the method [publishMeasuresBatch()](#publishMeasuresBatch).
* The method `publishAttribsStatic()` is dedicated for static client attributes of the device, which cannot be changed from IoT platform by <abbr title="Remote Procedure Call">RPC</abbr>, but they are set at compile or boot time of the device. So that they are published just once by either multiple calls of the method [publishAttrib()](#publishAttrib) or the single call of the method [publishAttribsBatch()](#p.ublishAttribsBatch ).
* The method `publishAttribsDynamic()` is dedicated for dynamic client attributes of the device, which change occassionally and rarely. So that they are published individually at every change immediatelly by the multiple calls of the method [publishAttrib()](#publishAttrib).

#### Syntax
    ResultCodes publishEvents()
    ResultCodes publishMeasures()
    ResultCodes publishAttribsStatic()
    ResultCodes publishAttribsDynamic()

#### Parameters
None

#### Returns
Some of [result or error codes](#constants) from the parent class.

#### See also
[publishMeasure()](#publishMeasure)

[publishMeasuresBatch()](#publishMeasuresBatch)

[publishAttrib()](#publishAttrib)

[publishAttribsBatch()](#publishAttribsBatch)

[Back to interface](#interface)


<a id="setPeriod"></a>

## setPeriod()

#### Description
The overloaded method sets a new period of the internal timer in milliseconds input either in milliseconds or in seconds.
* The method with numerical input argument is the implementation of the virtual methods from the parent class.
* The method with textual input argument is useful with conjunction with a project data hub, which data has always string data type.
* If input period is zero or not numerical (leading to zero), the library sets the [internal default Publishing period](#internals).

#### Syntax
    void setPeriod(unsigned long period)
    void setPeriod(String periodSec)

#### Parameters
* **period**: Duration of a repeating interval of the internal timers in milliseconds.
  * *Valid values*: 0 ~ 2^32 - 1
  * *Default value*: none


* **periodSec**: Duration of a repeating interval of the internal timers in seconds declared as string.
  * *Valid values*: String
  * *Default value*: none

#### Returns
None

#### See also
[getPeriod()](#getPeriod)

[Back to interface](#interface)


<a id="getPeriod"></a>

## getPeriod()

#### Description
The method returns current period of the internal timer.
* The method is the implementation of the virtual methods from the parent class.

#### Syntax
    unsigned long getPeriod()

#### Parameters
None

#### Returns
Current timer period in milliseconds.

#### See also
[setPeriod()](#setPeriod)

[Back to interface](#interface)


<a id="getPrmName"></a>

## getPrmName()

#### Description
The method returns a pointer to a parameter name.
* The method is protected and accessible only for derived child classes.

#### Syntax
    const char *getPrmName(const char *progmemPrmName)

#### Parameters
* **progmemPrmName**: Pointer to an internal buffer with copy of a parameter name stored in the flash memory of the microcontroller.
  * *Valid values*: pointer to a constant array of characters
  * *Default value*: none

#### Returns
Pointer to a parameter name.

[Back to interface](#interface)


<a id="getFails"></a>

## getFails()

#### Description
The method returns number of failed connection attempts since booting or recent successful connection.
* This published value can be used for disabling a heartbeating in an application and to restart the microcontroller by an external watchdog timer.

#### Syntax
    byte getFails()

#### Parameters
None

#### Returns
Returns current number of failed connection attempts.

[Back to interface](#interface)


<a id="getServer"></a>

## getServer()

#### Description
The method returns a pointer to an IP address of the ThingsBoard server, which is used for connection to IoT platform.

#### Syntax
    const char *getServer()

#### Parameters
None

#### Returns
Pointer to connected IP address of IoT platform.

[Back to interface](#interface)


<a id="isConnected"></a>

## isConnected()

#### Description
The method returns a flag whether the microcontroller is connected to the IoT platform.

#### Syntax
    bool isConnected()

#### Parameters
None

#### Returns
Flag about connecting status to IoT platform.

[Back to interface](#interface)


<a id="isSubscribed"></a>

## isSubscribed()

#### Description
The method returns a flag whether the <abbr title="Remote Procedure Call">RPC</abbr> functions have been successfully subscribed to the IoT platform.

#### Syntax
    bool isSubscribed()

#### Parameters
None

#### Returns
Flag about subscribe status to IoT platform.

[Back to interface](#interface)
