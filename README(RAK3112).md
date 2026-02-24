# RAK3112-Audio-Solutions

**Code Modification Notes：**

## 1.BSP json:

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
https://jihulab.com/esp-mirror/espressif/arduino-esp32/-/raw/gh-pages/package_esp32_index_cn.json
```



## 2.pins_arduino.h

Please update the official ESP32 library. If compilation fails, please wait for the official ESP32 library to update or manually modify **pins_arduino.h**.

**File pash:**

```
eg:C:\Users\user\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.3.6\variants\rakwireless_rak3112\pins_arduino.h  
```

（Your BSP path is usually the default path.）

 Add a definition at the beginning of the file( **pins_arduino.h**).

```
#ifndef _VARIANT_RAK3112_
#define _VARIANT_RAK3112_
#endif
```



## 3.PDM library

PDM.h/PDM.cpp has been added to the src/esp32-s3 directory, and conditional compilation has been added to src/audio.h. PDM.zip is also provided; if you encounter any problems, you can manually add it to the ESP32 BSP path (..\packages\esp32\\..\libraries).

Add the PDM library to ..\packages\esp32\\..\libraries.Windows:

```
eg:C:\Users\user\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.3.6\libraries
```




**The current version does not support speech recognition; this module will be updated in a future version**.

