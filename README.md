<a id="library"></a>

# gbj\_appthingsboard
This is an application library, which is used usually as a project library for particular PlatformIO project. It encapsulates the functionality of `connection to ThingsBoard IoT platform`. The encapsulation provides following advantages:

* Functionality is hidden from the main sketch.
* The library follows the principle `separation of concern`.
* The library is reusable for various projects without need to code connection management.
* Update in library is valid for all involved projects.
* It specifies (inherits from) the parent application library `gbj_appbase`.
* It utilizes funcionality and error handling from the parent class.


## Fundamental functionality
* The library utilizes internal timer for periodical data publishing to IoT platform.
* The connection to Wifi and to ThingsBoard IoT platform is checked at every loop of a main sketch.
* If no connection to IoT platform is detercted, the library starts the [connection process](#connection).
* The library subscibes externally defined (in a main sketch)  _Remote Procedure Call_ (RPC) functions to the IoT platform.
* The class from the library is not intended to be used directly in a sketch, just as a parent class for project specific device libraries communicating with IoT platform, e.g., `apptb_device`.
* The library provides a couple of generic parameters with names stored in flash of the microcontroller and defined in the shared (common) include file `config_params_gen.h`.


<a id="internals"></a>

## Internal parameters
Internal parameters are hard-coded in the library as enumerations and none of them have setters or getters associated.

* **Publishing period** (12 s): It is a default time period for publishing data to IoT platform. Real publishing period is associated with corresponding setter and getter.
* **Period of waiting for next connection attempt** (500 ms): It is a time period, during which the system is waiting in blocking mode for next attempt to connect to IoT platform. The real time period between failed connection attempts can be much longer due to IoT platform timeout.
* **Number of failed connection attempts in the connection set** (5): It is a countdown for failed connections to IoT platform at blocking waiting. After reaching this number of connection fails, which represents a connection set, the library starts waiting for next set, but without blocking the system.
* **Period of waiting for next connection set** (1 min.): It is a time period since recent failed connection attempt of recent connection set, during which the system is waiting in non-blocking mode for next connection set of attempts.
* **Number of failed connection sets** (3): It is a countdown for failed connection sets to IoT platform at non-blocking waiting. After reaching this number of connection sets, which represents a connection cycle, the library starts waiting for reapeting another connection cycle.
* **Period of waiting for next connection cycle** (5 min.): It is a time period since recent failed connection attempt of recent connection set of recent connection cycle, during which the system is waiting in non-blocking mode for next cycle of connections.


<a id="connection"></a>

## Connection process
The connection process is composed of 3 level aiming to be robust. It gives the chance either the microcontroller itself or the IoT platform to recover from failure and when connect automatically. The connection process is controlled by [internal parameters](#internals).

1. **Set of connection attemps**. It is a number of subsequent failed connection attemps. The library tries to connect to IoT platform. If it fails, it starts waiting in blocking mode for next attempt. If predefined number of connection attemps fails, the library starts waiting for next connection set. The connection set with waiting periods among connection attempts allow the microcontroller to consolidate its internals to establish connection. If a connection attemps is successful, the library breaks entire connection process and goes to connection checking mode.

2. **Cycle of connection sets**. It is a number of subsequent failed connection sets. After failed connection set the library is waiting in non-blocking mode for next connection set. If predefined number of connection sets fails, the library starts waiting for next connection cycle. The connection cycle with waiting periods among connection sets allow the microcontroller to wait for a network WiFi router or access point to consolidate, restart, or so.

3. **Reccurent connection cycles**. It is a repeating processing of previous two levels of connection process. If a connection cycle fails, the library starts waiting for repeating connection process described before. The waiting period among connection cycles allow to manually resolve potential problems with a WiFi router or access point, it configuration, restarting, or so.


<a id="generics"></a>

## Generic publishing parameters
Library provides definition of following generic parameter names aimed for publishing to IoT platform that are utilized almost in every project.

* They all are protected, so accessible only for derived child classes.
* The definition name of a parameters defines the name of a parameter (attribute or telemetry measure), which is visibile in IoT platform.
* The variable of definition name takes name of corresponding parameter with appropriate suffix denoting its role.
* The variable name of a parameter is usually same the parameter's name itself.

#### Static attributes initiated at compile time (compiler build macros defined usually in the project manifest `platformio.ini`)

  * **version** with definition name `versionStatic` and value _MAIN_VERSION_. It is the semantic version of a firmware version and servers in IoT platform as a client attribute for current microcontroller firmware identifier, e.g. 1.2.3.
  * **broker** with definition name `brokerStatic` and value _BROKER_. It is informative name (mark, tag, etc.) of a computer where the ThingsBoard IoT platform runs and to which the microcontroller is connected, e.g., server, notebook, etc.
  * **hostname** with definition name `hostnameStatic` and value 'WIFI_HOSTNAME'. It is the host name of the microcontroller on a WiFi network.
  * **portOTA** with definition name `portOTAStatic` and value _OTA_PORT_. It is the TCP port, there an HTTP server with functionality of OTA listens, usually 80.

#### Static attributes initiated at at runtime right after boot of the microcontroller, but only once
  * **mcuBoot** with definition name `mcuBootStatic`. It is the boot reason of the recent microcontroller reset in form of name defined in the library `gbj_appcore` and reachable by parent getter `getResetName()`.
  * **addressMAC** with definition name `addressMACStatic`. It is the MAC address of the microcontroller WiFi interface.

#### Dynamic attributes updated immediatelly (usually stored in the EEPROM)
  * **mcuRestarts** with definition name `mcuRestartsPrm`. It is number of the microcontroller restarts initiated by failed attempts at WiFi connection process (similar to this connection process). The parameter is stored in the EEPROM. It is published only at change, i.e., after the microcontroller restart (boot) immediatelly, mostly due to failed wifi connection process.
  * **addressIP** with definition name `addressIPPrm`. It is the current IP address of the microcontroller on the network.  It is published only at change (usually at WiFi reconnect), but immediatelly.
  * **periodPublish** with definition name `periodPublishPrm`. It is the curren time period for publishing telemetry data to IoT platform. The parameter is stored in the EEPROM. It is published only at change by an RPC function immediatelly.

#### Measures updated periodically (telemetry)
* **rssi** with definition name `rssiTelem`. It is the value of WiFi _Received Signal Strength Indicator_ in _decibel milliwats_ (dBm). It is published at every publish period regardless it has been changed or not since recent publishing. The parameter serves as the signal of active microcontroller for IoT platform.


<a id="dependency"></a>

## Dependency
* **gbj\_appbase**: Parent library for all application libraries loaded from the file `gbj_appbase.h`.
* **gbj\_serial\_debug**: Auxilliary library for debug serial output loaded from the file `gbj_serial_debug.h`. It enables to exclude serial outputs from final compilation.
* **gbj\_timer**: Library for executing internal timer within an instance object loaded from the file `gbj_timer.h`.
* **gbj\_appwifi**: Application library for managing wifi connection loaded from the file `gbj_appwifi.h`.
* **ThingsBoard**: Library for managing connection to IoT platform loaded from the file `ThingsBoard.h` in library `ThingsBoard-Arduino-MQTT-SDK`.
* **config_params_gen**: Include file with definition of keys (names) of generic parameters, which the library provides loaded from the file `config_params_gen.h`. It is usually located on a shared location (folder), so that all projects can share and use it.

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
* [Parameter](#parameter)

## Interface
The methods in bold are virtual methods and should be implemented in a project specific libraries.

* [gbj_appthingsboard()](#gbj_appthingsboard)
* [begin()](#begin)
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
* [**setPeriod()**](#period)
* [**getPeriod()**](#period)
* [getPrmName()](#getPrmName)
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
      Handler *onConnectStart;
      Handler *onConnectTry;
      Handler *onConnectSuccess;
      Handler *onConnectFail;
      Handler *onDisconnect;
      Handler *onSubscribeSuccess;
      Handler *onSubscribeFail;
    }

#### Parameters
* **onConnectStart**: Pointer to a callback function, which is call right before a new connection set.
* **onConnectTry**: Pointer to a callback function, which is call after every failed connection attempt. It allows to observer the pending connection set.
* **onConnectStart**: Pointer to a callback function, which is call right after successful connection to IoT platform.
* **onConnectFail**: Pointer to a callback function, which is call right after failed connection set.
* **onDisconnect**: Pointer to a callback function, which is call at lost of connection to IoT platform. It allows to create alarm or signal about it.
* **onSubscribeSuccess**: Pointer to a callback function, which is call right after successful subscribing RPC functions.
* **onSubscribeFail**: Pointer to a callback function, which is call right after failed subscribing RPC functions.

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


<a id="parameter"></a>

## Parameter

#### Description
The structure with members and member methods as a template of an publishing parameter.
* The structure is protected, so that it is accessable only for derived classes, usually in project specific libraries, which only can define new parameters and their lists.
* A publishing parameter as an instance of this structure acts as a parameter's cache for the IoT platform.
* A parameter's cache is updated within particular virtual method implementation in the child class of a project specific library with parameter's setter.
* Usually, if the parameter's cache is updated with the same value as already stored and has been already used, i.e., read by parameter's getter, the corresponding internal parameter's flag is set, which signals, that the values has not been changed since recent reading. This internal flag can be reset by corresponding parameter's method.

#### Parameter methods
* **Parameter(), Parameter(const char *key)**: Constructor for null parameter and a named parameter without initial value. This value can added by a parameter's setter.
* **Parameter(const char *key, _\<datatype\>_ value)**: Constructors for a named parameter with initial value of particular data type. Valid data types are:
  * const char* - pointer to an external string buffer
  * String
  * bool
  * int
  * long
  * unsigned int
  * unsigned long
* **void set()**: The setter for updating a parameter with none value.
* **void set(_\<datatype\>_ value)**: The setter for updating a parameter with initial value of particular data type. Valid data types are the same as at constructors.
* **String get()**: The parameter's getter. It always converts the original value of the parameter to String. The result can be published to IoT platform even if it has the data type not supported by that platform. At the same time the getter sets the internal flag, which determines that the parameter value has been already read (used). It enables to a parameter's setter set another internal flag at updating with the same value, that the parameter should be ignored for subsequent publishings until a change occurs.
* **void set(byte value)**: Setter of cache parameter value as an argument. Provided value is sanitized with valid range. If it is lower than minimum or greater than maximum, the default value is cached. If this sanitized value differs from already cached value, the change flag _chg_ is set. The setter returns cached value by calling its getter.
* **bool getIgnore()**: It returns the internal flag determining that the parameter is marked to be ignored for next publishing.
* **void setIgnore()**: It sets the internal flag determining that the parameter should be ignored for next publishing.
* **void resetIgnore()**: It clears the internal flag determining that the parameter should be ignored for next publishing.

#### See also
[Generic publishing parameters](#generics)

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


<a id="begin"></a>

## begin()

#### Description
The initialization method of the instance object, which should be called in the setup section of a sketch.
* The method realizes the dependency injection of utilized external objects.

#### Syntax
	void begin(gbj_appwifi *wifi)

#### Parameters
* **wifi**: Pointer to the instance object for processing WiFi connection.
  * *Valid values*: pointer to an object of type `gbj_appwifi`
  * *Default value*: none

#### Returns
None

#### Example
Initialization instance object in the sketch loop. Instantiation of the library is only for illustration here. Use the appropriate child class of a project specific library instead.
```cpp
gbj_appwifi wifi = gbj_appwifi(...);
gbj_appthingsboard device = gbj_appthingsboard("MyServer", "MyToken", handlersDevice);
void setup()
{
  device.begin(&wifi);
}
```

[Back to interface](#interface)


<a id="callbacks"></a>

## callbacks()

#### Description
The registration method for subscribing external functions as RPC callbacks to the IoT platform.
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
PC_Response processSetDelay(const RPC_Data &data){...}
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
* The method `publishAttribsStatic()` is dedicated for static client attributes of the device, which cannot be changed from IoT platform by RPC, but they are set at compile or boot time of the device. So that they are published just once by either multiple calls of the method [publishAttrib()](#publishAttrib) or the single call of the method [publishAttribsBatch()](#p.ublishAttribsBatch ).
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


<a id="period"></a>

## setPeriod(), getPeriod()

#### Description
The methods are just straitforward implementation of the virual methods from the parent class.

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
The method returns a flag whether the RPC functions RE successfully subscribed to the IoT platform.

#### Syntax
    bool isSubscribed()

#### Parameters
None

#### Returns
Flag about subscribe status to IoT platform.

[Back to interface](#interface)
