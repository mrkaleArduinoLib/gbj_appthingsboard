<a id="library"></a>
# gbj\_appthingsboard
This is an application library, wich is used usually as a project library for particular PlatformIO project. However; in every project utilizing the ThingsBoard IoT platform should be copied the same library, so that it is located in central library storage.

- Library specifies (inherits from) the system `gbj_appbase` library.
- Library utilizes error handling from the parent class.
- The class from the library is not intended to be used directly in a sketch, just as a parent class for specific device libraries communicating with IoT platform.


<a id="dependency"></a>
## Dependency

- **gbj\_appbase**: Parent library for all application libraries loaded from the file `gbj_appbase.h`.
- **gbj\_appwifi**: Application library for wifi connection loaded from the file `gbj_appwifi.h`.
- **gbj\_timer**: Library for executing internal timer within an instance object loaded from the file `gbj_timer.h`.
- **gbj\_serial\_debug**: Auxilliary library for debug serial output loaded from the file `gbj_serial_debug.h`. It enables to exclude serial outputs from final compilation.
- **ThingsBoard**: Library for managing connection with IoT platform loaded from the file `ThingsBoard.h` in library `ThingsBoard-Arduino-MQTT-SDK`.

#### Particle platform
- **Particle.h**: Includes alternative (C++) data type definitions.

#### Arduino platform
- **Arduino.h**: Main include file for the Arduino SDK.
- **inttypes.h**: Integer type conversions. This header file includes the exact-width integer definitions and extends them with additional facilities provided by the implementation.

#### Espressif ESP8266 platform
- **Arduino.h**: Main include file for the Arduino platform.
- **ESP8266WiFi.h**: Main include file for the wifi connection.

#### Espressif ESP32 platform
- **Arduino.h**: Main include file for the Arduino platform.
- **WiFi.h**: Main include file for the wifi connection.


<a id="constants"></a>
## Constants

- **gbj\_appthingsboard::VERSION**: Name and semantic version of the library.

Other constants and enumerations are inherited from the parent library.


<a id="interface"></a>
## Interface

- [gbj_appthingsboard()](#gbj_appthingsboard)
- [begin()](#begin)
- [callbacks()](#callbacks)
- [run()](#run)
- [publishDataItem()](#publishDataItem)
- [publishAttrib()](#publishAttrib)
- [publishData()](#publishAttrib)
- [publishData()](#publish)
- [publishAttribs()](#publish)
- [setAttribsChange()](#setAttribsChange)
- [setPeriod()](#setPeriod)
- [getPeriod()](#getPeriod)
- [isConnected()](#isConnected)
- [isSubscribed()](#isSubscribed)


<a id="gbj_appthingsboard"></a>
## gbj_appthingsboard()

#### Description
Constructor creates the class instance object and initiates internal resources.
- It inputs credentials for identifying a IoT Platform device.
- It creates one internal timer without a timer handler.

#### Syntax
    gbj_appthingsboard(const char *server, const char *token);

#### Parameters

- **server**: Pointer to an address of ThingsBoard server. It should be either IP address of webk address.
  - *Valid values*: Constant pointer to string
  - *Default value*: none


- **token**: Pointer to the authorization token of a ThingsBoard device.
  - *Valid values*: Constant pointer to string
  - *Default value*: none


#### Returns
Object performing connection and reconnection to the IoT platform.

[Back to interface](#interface)


<a id="begin"></a>
## begin()

#### Description
The initialization method of the instance object, which should be called in the setup section of a sketch.
- The method realizes the dependency injection of utilized external objects.
- The method set the internal flag about changed published attributes in order to achieve initial publishing of all attributes at the start of the sketch.

#### Syntax
	void begin(gbj_appwifi *wifi);

#### Parameters

- **wifi**: Pointer to the instance object for processing WiFi connection.
  - *Valid values*: Pointer to an object of type `gbj_appwifi`.
  - *Default value*: none

#### Returns
None

#### Example
Initialization instance object in the sketch loop. Instantiation of the library is only for illustration here. Use the appropriate child library instead.
```cpp
gbj_appwifi wifi = gbj_appwifi(...);
gbj_appthingsboard device = gbj_appthingsboard("MyServer", "MyToken");
void setup()
{
  device.begin(&wifi);
}
```

[Back to interface](#interface)


<a id="callbacks"></a>
## callbacks()

#### Description
The registration method for subscribing external functions as callbacks to the IoT platform.
- The method should be called in the setup section of a sketch.
- The method subscribes all external function in the input list.

#### Syntax
	void callbacks(RPC_Callback *callbacks = 0, size_t callbacks_size = 0);

#### Parameters

- **callbacks**: Array (list) of external function to be subscribed.
  - *Valid values*: Pointer to an array of type `RPC_Callback`.
  - *Default value*: 0


- **callbacks_size**: Number of callback functions in the list.
  - *Valid values*: Positive integer.
  - *Default value*: 0

#### Returns
None

#### Example
Initialization instance object in the sketch loop. Instantiation of the library is only for illustration here. Use the appropriate child library instead.
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
gbj_appthingsboard device = gbj_appthingsboard("MyServer", "MyToken");
void setup()
{
  device.callbacks(callbacks, callbacks_size);
}
```

[Back to interface](#interface)


<a id="run"></a>
## run()

#### Description
The execution method, which should be called frequently, usually in the loop function of a sketch.
- The method connects to the IoT platform at the very first calling it.
- At the start of each timer period the method checks the connection to the wifi network as well as to the IoT platform. If wifi network is available it reconnects to the IoT platform if neccesary.
- If the serial connection is active, the library outputs flow of the connection and at success lists input credentials of the connection to the IoT platform.

#### Syntax
	void run();

#### Parameters
None

#### Returns
None

[Back to interface](#interface)


<a id="publishDataItem"></a>
## publishDataItem()

#### Description
The method publishes input key-value pair as the telemetry of the device to the IoT platform.
- The method is templated by input value data type.
- The method does not need to be called by templating syntax, because it is able to identify proper data type by data type of the just value parameter.

#### Syntax
    template<class T>
    ResultCodes publishDataItem(const char *key, T value);

#### Parameters

- **key**: Pointer to a telemetry data item key (name). It should be in lower pascal case (camel case).
  - *Valid values*: Pointer to a constant string.
  - *Default value*: none


- **value**: Value of corresponding data type for a telemetry data item.
  - *Valid values*: Any value of type `int`, `bool`, `float`, `const char*`.
  - *Default value*: none

#### Returns
Some of [result or error codes](#constants) from the parent class.

#### See also
[publishAttrib()](#publishAttrib)

[Back to interface](#interface)


<a id="publishAttrib"></a>
## publishAttrib()

#### Description
The method publishes input key-value pair as the clilent attributes of the device to the IoT platform.
- The method is templated by input value data type.
- The method does not need to be called by templating syntax, because it is able to identify proper data type by data type of the just value parameter.

#### Syntax
    template<class T>
    ResultCodes publishAttrib(const char *attrName, T value);

#### Parameters

- **attrName**: Pointer to a client attribute name. It should be in lower pascal case (camel case).
  - *Valid values*: Pointer to a constant string.
  - *Default value*: none


- **value**: Value of corresponding data type for an attribute.
  - *Valid values*: Any value of type `int`, `bool`, `float`, `const char*`.
  - *Default value*: none

#### Returns
Some of [result or error codes](#constants) from the parent class.

#### See also
[publishDataItem()](#publishDataItem)

[Back to interface](#interface)


<a id="publish"></a>
## publishData(), publishAttribs()

#### Description
The virtual methods that should implement every child class derived from this library class.
- The method `publishData()` should contain multiple calls of the method [publishDataItem()](#publishDataItem) for all desired telemetry data itemsof the device.
- The method `publishAttribs()` should contain multiple calls of the method [publishAttrib()](#publishAttrib) for all desired client attributes of the device.

#### Syntax
    ResultCodes publishData();
    ResultCodes publishAttribs();

#### Parameters
None

#### Returns
Some of [result or error codes](#constants) from the parent class.

#### See also
[publishDataItem()](#publishDataItem)

[publishAttrib()](#publishAttrib)

[Back to interface](#interface)


<a id="setPeriod"></a>
## setPeriod()

#### Description
The method sets a new internal timer period. It allows to dynamically change a frequency of publishing data to the IoT platform.

#### Syntax
    void setPeriod(unsigned long period)

#### Parameters
- **period**: Duration of a repeating interval in milliseconds.
  - *Valid values*: 0 ~ 2^32 * 1
  - *Default value*: none

#### Returns
None

#### See also
[getPeriod()](#getPeriod)

[Back to interface](#interface)


<a id="setAttribsChange"></a>
## setAttribsChange()

#### Description
The method sets an internal flag about changing some device client attribute in order to refresh it in the IoT platform.
- The method realizes receiving a signal from a sketch, usually in a RPC method for setting parameters, that some attribute has changed.
- The method causes that all device client attributes are publish at once regardless some of them has not changed their value.

#### Syntax
    void setAttribsChange()

#### Parameters
None

#### Returns
None

[Back to interface](#interface)


<a id="getPeriod"></a>
## getPeriod()

#### Description
The method returns current internal timer period, i.e., frequency of the wifi connection check.

#### Syntax
    unsigned long getPeriod()

#### Parameters
None

#### Returns
Current timer period in milliseconds.

#### See also
[setPeriod()](#getPeriod)

[Back to interface](#interface)


<a id="isConnected"></a>
## isConnected()

#### Description
The method returns a flag whether the device is connected to the IoT platform.

#### Syntax
    bool isConnected();

#### Parameters
None

#### Returns
Flag about connecting status to IoT platform.

[Back to interface](#interface)


<a id="isSubscribed"></a>
## isSubscribed()

#### Description
The method returns a flag whether the device is successfully subscribed to the IoT platform.

#### Syntax
    bool isSubscribed();

#### Parameters
None

#### Returns
Flag about subscribe status to IoT platform.

[Back to interface](#interface)
