<a id="library"></a>

# gbj\_appthingsboard
This is an application library, which is used usually as a project library for particular PlatformIO project. However; in every project utilizing the ThingsBoard IoT platform should be copied the same library, so that it is located in central library storage.

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

#### Espressif ESP8266 platform
- **Arduino.h**: Main include file for the Arduino platform.
- **ESP8266WiFi.h**: Main include file for the wifi connection.

#### Espressif ESP32 platform
- **Arduino.h**: Main include file for the Arduino platform.
- **WiFi.h**: Main include file for the wifi connection.

#### Particle platform
- **Particle.h**: Includes alternative (C++) data type definitions.

> Library is not intended to be utilized on platforms without WiFi capabality.


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
- [publishMeasure()](#publishMeasure)
- [publishMeasures()](#publish)
- [publishMeasuresBatch()](#publishMeasuresBatch)
- [publishAttrib()](#publishAttrib)
- [publishAttribsStatic()](#publish)
- [publishAttribsDynamic()](#publish)
- [publishAttribsBatch()](#publishAttribsBatch)
- [setAttribChange()](#setAttribChange)
- [setPeriod()](#period)
- [getPeriod()](#period)
- [isConnected()](#isConnected)
- [isSubscribed()](#isSubscribed)


<a id="gbj_appthingsboard"></a>

## gbj_appthingsboard()

#### Description
Constructor creates the class instance object and initiates internal resources.
- It inputs credentials for identifying a IoT Platform device.
- It creates one internal timer without a timer handler.

#### Syntax
    gbj_appthingsboard(const char *server, const char *token)

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
	void begin(gbj_appwifi *wifi)

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
	void callbacks(RPC_Callback *callbacks = 0, size_t callbacks_size = 0)

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
The execution method as the implementation of the virtual method from parent class, which should be called frequently, usually in the loop function of a sketch.
- The method connects to the IoT platform at the very first calling it.
- At the start of each timer period the method checks the connection to the wifi network as well as to the IoT platform. If wifi network is available it reconnects to the IoT platform if neccesary.
- If the serial connection is active, the library outputs flow of the connection and at success lists input credentials of the connection to the IoT platform.

[Back to interface](#interface)


<a id="publishMeasure"></a>

## publishMeasure()

#### Description
The method publishes input key-value pair as the telemetry data item of the device to the IoT platform.
- The method is templated by input value data type.
- The method does not need to be called by templating syntax, because it is able to identify proper data type by data type of the just value parameter.

#### Syntax
    template<class T>
    ResultCodes publishMeasure(const char *key, T value)

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
[publishMeasures()](#publish)

[publishMeasuresBatch()](#publishMeasuresBatch)

[publishAttrib()](#publishAttrib)

[Back to interface](#interface)


<a id="publishMeasuresBatch"></a>

## publishMeasuresBatch()

#### Description
The method publishes input array of key-value pairs as a batch of the telemetry data items of the device to the IoT platform.
- The key-value pair itself is an array with two items, where the first one is the key and second one is the value.

#### Syntax
    ResultCodes publishMeasuresBatch(const Telemetry *data, size_t data_count)

#### Parameters

- **data**: Pointer to an array of arrays with telemetry data item key-value pairs.
  - *Valid values*: Pointer to a constant array of arrays.
  - *Default value*: none


- **data_count**: Number of key-value pairs in input array
  - *Valid values*: Non-negative integer
  - *Default value*: none

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
The method publishes input key-value pair as the clilent attributes of the device to the IoT platform.
- The method is templated by input value data type.
- The method does not need to be called by templating syntax, because it is able to identify proper data type by data type of the just value parameter.

#### Syntax
    template<class T>
    ResultCodes publishAttrib(const char *attrName, T value)

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
[publishAttribsStatic()](#publish)

[publishAttribsDynamic()](#publish)

[publishAttribsBatch()](#publishAttribsBatch)

[publishMeasure()](#publishMeasure)

[Back to interface](#interface)


<a id="publishAttribsBatch"></a>

## publishAttribsBatch()

#### Description
The method publishes input array of key-value pairs as a batch of the client attributes of the device to the IoT platform.
- The key-value pair itself is an array with two items, where the first one is the key and second one is the value.

#### Syntax
    ResultCodes publishAttribsBatch(const Attribute *data, size_t data_count)

#### Parameters

- **data**: Pointer to an array of arrays with client attributes key-value pairs.
  - *Valid values*: Pointer to a constant array of arrays.
  - *Default value*: none


- **data_count**: Number of key-value pairs in input array
  - *Valid values*: Non-negative integer
  - *Default value*: none

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

## publishMeasures(), publishAttribsStatic(), publishAttribsDynamic()

#### Description
The virtual methods that every child class derived from this library class should implement.
- The method `publishMeasures()` should contain either multiple calls of the method [publishMeasure()](#publishMeasure) or the single call of the method [publishMeasuresBatch()](#publishMeasuresBatch) for all desired telemetry data items of the device.
- The method `publishAttribsStatic()` should contain either multiple calls of the method [publishAttrib()](#publishAttrib) or the single call of the method [publishAttribsBatch()](#publishAttribsBatch ) for all desired static client attributes of the device. Static attributes cannot be changed from IoT platform by RPC. They are set and published at boot time of the device together at once.
- The method `publishAttribsDynamic()` should contain either multiple calls of the method [publishAttrib()](#publishAttrib) or the single call of the method [publishAttribsBatch()](#publishAttribsBatch ) for all desired dynamic client attributes of the device. Dynamic attributes can be changed from IoT platform by RPC. They are set and published dynamicli and individually.

#### Syntax
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


<a id="setAttribChange"></a>

## setAttribChange()

#### Description
The virtual method that every child class derived from this library class should implement.
The method sets an internal flag about changing particular dynamic device client attribute in order to refresh it in the IoT platform.
- The method realizes receiving a signal from a sketch, usually in an RPC method for setting the parameter, that the attribute has changed.
- The method causes that particular device client attribute is publish individually, so that all dynamic attributes have real time stamp in the IoT platform.

#### Syntax
    void setAttribChange(byte idx)

#### Parameters
- **idx**: Index (sequence order counting from zero) of a dynamic attribute usually defined as preprocessor macro.
  - *Valid values*: Non-negative integer
  - *Default value*: none

#### Returns
None

[Back to interface](#interface)


<a id="isConnected"></a>

## isConnected()

#### Description
The method returns a flag whether the device is connected to the IoT platform.

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
The method returns a flag whether the device is successfully subscribed to the IoT platform.

#### Syntax
    bool isSubscribed()

#### Parameters
None

#### Returns
Flag about subscribe status to IoT platform.

[Back to interface](#interface)
