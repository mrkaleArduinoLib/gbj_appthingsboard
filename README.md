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
* The library utilizes internal timer for periodical data publishing to <abbr title="Internet of Things">IoT</abbr> platform.
* The connection to Wifi and to ThingsBoard <abbr title="Internet of Things">IoT</abbr> platform is checked at every loop of a main sketch.
* If no connection to <abbr title="Internet of Things">IoT</abbr> platform is detected, the library starts the [connection process](#connection).
* The library subscibes externally defined (in a main sketch) <abbr title="Remote Procedure Call">RPC</abbr> functions to the <abbr title="Internet of Things">IoT</abbr> platform.
* The class from the library is not intended to be used directly in a sketch, just as a parent class for project specific device libraries communicating with <abbr title="Internet of Things">IoT</abbr> platform, e.g., `apptb_device`.
* The library provides a couple of generic parameters with names stored in flash of the microcontroller and defined in the shared (common) include file `config_params_gen.h`.


<a id="internals"></a>

## Internal parameters
Internal parameters are hard-coded in the library as enumerations and none of them have setters or getters associated. The waiting for next connection attempt is in non-blocking mode. However, waiting on IoT platform connection `timeout is cca 5 seconds` in either case.

* **Publishing period** (`12 seconds`): It is a default time period for publishing data to IoT platform. Real publishing period is associated with corresponding setter and getter.
* **1st stage time period of waiting for next connection attempt** (`5 seconds`): It is a time period during which the system is waiting for next attempt to connect to IoT platform at this stage of connection process. The real time period between failed connection attempts is increased by the IoT platform timeout, so that the real time period amont attempts is cca `10 seconds`.
* **2nd stage time period of waiting for next connection attempt** (`1 minute`): It is a time period during which the system is waiting for next attempt to connect to IoT platform at this stage of connection process. The real time period among failed connection attempts is increased by the IoT platform timeout, so that the real time period amont attempts is cca `65 seconds`.
* **3rd stage time period of waiting for next connection attempt** (`5 minutes`): It is a time period during which the system is waiting for next attempt to connect to IoT platform at this stage of connection process. The real time period among failed connection attempts is increased by the IoT platform timeout, so that the real time period amont attempts is cca `305 seconds`.
* **1st stage number of failed connection attempts** (`6`): It is a number of failed connections with waiting among them for _short time period_. With the aforementioned time period for this stage, its duration including IoT platform timeouts is `60 seconds`, i.e. 1 minute.
After reaching this number of connection fails, the library starts next connection stage.
* **2nd stage number of failed connection attempts** (`11`): It is a number of cummulative failed connections with waiting among them for _medium time period_. The number of failed connections at this stage of connection process is difference between the cummulative number and number for previous stage, i.e., 11 - 6 = `5`. With the aforementioned time period for this stage, its duration including IoT platform timeouts is `325 seconds`, i.e., 5 minutes and 25 seconds.
After reaching this number of connection fails, the library starts next connection stage.
* **3rd stage number of failed connection attempts** (`23`): It is a number of cummulative failed connections with waiting among them for _long time period_. The number of failed connections at this stage of connection process is difference between the cummulative number and number for previous stage, i.e., 23 - 11 = `12`. With the aforementioned time period for this stage, its duration including IoT platform timeouts is `3660 seconds`, i.e., 1 hour and 1 minute.
After reaching this number of connection fails, the library repeates the connection process from scratch with the first stage.


<a id="connection"></a>

## Connection process
The connection process is composed of 3 stages aiming to be robust. It gives the chance either the microcontroller itself or IoT platform to recover from failure and when connect automatically. On the other hand, failed connection attempts block the microcontroller as less as possible, mostly due to IoT platform timeout. If a connection attemp is successful, the library breaks entire connection process and goes to connection checking mode. The library provides handlers and getters for observing connection process, e.g., for statistical purposes.

The connection process is controlled by [internal parameters](#internals) and starts at the first stage with short waiting time period.
* When number of failed connections reaches the predefined number for first stage, the library increases the waiting period for second stage with medium time period.
* When number of failed connections reaches the predefined number for second stage, the library increases the waiting period for third stage with long time period.
* When number of failed connections reaches the predefined number for third stage, the library cycles the entire connection process with first stage.


<a id="generics"></a>

## Generic publishing parameters
Library provides definition of following generic parameter names aimed for publishing to IoT platform that are utilized almost in every project.

* They all are protected, so accessible only for derived child classes.
* The definition name of a parameters defines the name of a parameter (attribute or telemetry measure), which is visibile in IoT platform.
* The variable of definition name takes name of corresponding parameter with appropriate suffix denoting its role.
* The variable name of a parameter is usually same as the parameter's name itself.

#### Static attributes initiated at at runtime right after boot of the microcontroller, but only once
  * **version** with definition name `versionStatic`. Semantic version of a firmware version in IoT platform as a client attribute for current microcontroller firmware identifier, e.g. 1.2.3.
  * **broker** with definition name `brokerStatic`. <abbr title="Internet Protocol">IP</abbr> or <abbr title="Multicast Domain Name System">mDNS</abbr> address of ThingsBoard server, i.e., a computer where the ThingsBoard IoT platform runs and to which the microcontroller is connected.
  * **hostname** with definition name `hostnameStatic`. Host name of the microcontroller on a WiFi network.
  * **portOTA** with definition name `portOTAStatic`. <abbr title="Transmission Control Protocol">TCP</abbr> port, there an <abbr title="Hypertext Transfer Protocol">HTTP</abbr> server with functionality of <abbr title="Over The Air">OTA</abbr> listens, usually 80.
  * **mcuBoot** with definition name `mcuBootStatic`. Boot reason of the recent microcontroller reset in form of name defined in the library `gbj_appcore` and reachable by parent getter `getResetName()`.
  * **addressIP** with definition name `addressIpStatic`. <abbr title="Internet Protocol">IP</abbr> address of the microcontroller on the network, usually set as static (fixed) one directly by the microcontroller or indirectly by <abbr title="Dynamic Host Configuration Protocol">DHCP</abbr> server.
  * **addressMAC** with definition name `addressMacStatic`. <abbr title="Media Access Control">MAC</abbr> address of the microcontroller WiFi interface.

#### Dynamic attributes updated immediatelly (usually stored in the EEPROM)
  * **mcuRestarts** with definition name `mcuRestartsPrm`. Number of the microcontroller restarts initiated by failed attempts at WiFi connection process (similar to this connection process). The parameter is stored in the <abbr title="Electrically Erasable Programmable Read-Only Memory">EEPROM</abbr>. It is published only at change, i.e., after the microcontroller restart (boot) immediatelly, mostly due to failed wifi connection process.
  * **periodPublish** with definition name `periodPublishPrm`. Current time period for publishing telemetry data to IoT platform. The parameter is stored in the <abbr title="Electrically Erasable Programmable Read-Only Memory">EEPROM</abbr>. It is published only at change by an <abbr title="Remote Procedure Call">RPC</abbr> function immediatelly.

#### Measures updated periodically (telemetry)
* **rssi** with definition name `rssiTelem`. <abbr title="Received Signal Strength Indicator">RSSI</abbr> value of WiFi connection with the network expressed in <abbr title="Decibel milliwats">dBm</abbr>. It is published at every publish period regardless of it has been changed since recent publishing. The parameter serves as the signal of active microcontroller for IoT platform.


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
* [getStage()](#getConnStat)
* [getFails()](#getConnStat)
* [getCycles()](#getConnStat)
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
* **onConnectStart**: Pointer to a callback function, which is called right before a new connection set.
* **onConnectTry**: Pointer to a callback function, which is called after every failed connection attempt. It allows to observe the pending connection set.
* **onConnectSuccess**: Pointer to a callback function, which is called right after successful connection to IoT platform.
* **onConnectFail**: Pointer to a callback function, which is called right after failed connection set.
* **onDisconnect**: Pointer to a callback function, which is called at lost of connection to IoT platform. It allows to create an alarm or a signal about it.
* **onSubscribeSuccess**: Pointer to a callback function, which is called right after successful subscribing <abbr title="Remote Procedure Call">RPC</abbr> functions.
* **onSubscribeFail**: Pointer to a callback function, which is called right after failed subscribing <abbr title="Remote Procedure Call">RPC</abbr> functions.

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
* By default every parameter is internally marked as publishable only at change. This can be permanently discarded by corresponding constructor's argument or temporarily (for next publishing period only, i.e., until subsequent using new value) by corresponding method.

#### Parameter methods
* **Parameter(const char *key[, bool always = false])**: Constructor for a parameter and its flag about publishing.
  * **key**: The name of a parameter as it is used for IoT platform for a measure's key-value pair. The parameter's name is usually stored in flash memory for reducing operational memory usage.
  * **always**: The optional flag determining the publish mode of parameter. By default it is _false_ and sets that mode for publishing at change only.
* **void set(_\<datatype\>_ value)**: The setter for updating a parameter with value of particular data type. Valid data types are:
  1. const char* - pointer to an external string buffer
  2. String
  3. bool
  4. int
  5. long
  6. unsigned int
  7. unsigned long
  8. float
* **char *getName()**: The getter returns the parameter's name for publishing purposes.
* **String get()**: The parameter's getter returns the parameter's current value for publishing purposes. It always converts the original value of the parameter to data type _String_. The result can be published to IoT platform even if it has the data type not supported by that platform. At the same time the getter sets the internal flag, which determines that the parameter has been already initiated, i.e., initially published. From now on the parameter's publishing mode comes into force.
* **bool isReady()**: It determines whether the publishing of the parameter is available.
* **void init()**: It forces the publishing of the parameter for the next publishing period as the parameter would used for the first time. It is useful, e.g., for publishing long time stable telemetry measures at particular multiple of a publishing period.
* **void hide()**: It suppresses the publishing of the parameter for the next publishing period (until calling the setter). It is useful, e.g., at detecting improper or unreasonable parameter's value within pending publishing period.

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
* **server**: Pointer to an address of ThingsBoard server. It should be either IP address, mDNS address, or web address.
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


<a id="getConnStat"></a>

## getStage(), getFails(), getCycles()

#### Description
The corresponding method returns a measure of pending connection process, which can be used for statistical evaluation of the process, especially in particular handlers.

#### Syntax
    byte getStage()
    byte getFails()
    byte getCycles()

#### Parameters
None

#### Returns
* **getStage** returns current stage of the connection process, which determines waiting time period among connection attempts.
* **getFails** returns current number of failed connection attempts withing pending connection cycle of the connection process, which determines threshold for changing waiting time period.
* **getCycles** returns number of concluded connection cycles of the connection process, which determines how many times the connection process has been repeated.

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
