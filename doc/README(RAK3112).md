# RAK3112-Audio-Solutions

> **Speech recognition is supported via RAK18080 (DSPG) in the current solution.**
>
> Future roadmap: will gradually add **AI speech recognition** and related capabilities (e.g. on-device AI/edge inference).

## 1.**Code Modification Notes：**

### 1.1.BSP json:

```
https://espressif.github.io/arduino-esp32/package_esp32_index.json
https://jihulab.com/esp-mirror/espressif/arduino-esp32/-/raw/gh-pages/package_esp32_index_cn.json
```

### 1.2.pins_arduino.h

Please update the official ESP32 library. If compilation fails, please wait for the official ESP32 library to update or manually modify **pins_arduino.h**.

**Note (ESP32 BSP versions):** as of Arduino-ESP32 BSP **v3.3.7**, the latest RAK3112 variant macro is not included yet. You can **manually add** `#define _VARIANT_RAK3112_` in `pins_arduino.h` for now, or **wait for v3.3.8** (or later) where the variant update is expected to land.

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

### 1.3.PDM library

PDM.h/PDM.cpp has been added to the src/esp32-s3 directory, and conditional compilation has been added to src/audio.h. PDM.zip is also provided; if you encounter any problems, you can manually add it to the ESP32 BSP path (..\packages\esp32\\..\libraries).

Add the PDM library to ..\packages\esp32\\..\libraries.Windows:

```
eg:C:\Users\user\AppData\Local\Arduino15\packages\esp32\hardware\esp32\3.3.6\libraries
```





## 2.Bug Fix Instructions (Partial):

#### 2.1 PlayBack

Fixed the issue where volume could not be adjusted. Added a hardware gain function to control volume.

```
AMP_Left.set_gain(5);   // gain can set from 0-15
AMP_Right.set_gain(5); 
```

#### 2.2 PlayBack22K

The issue of doubled playback speed has been fixed; playback is now normal.

```
int channels = 2;   //Stereo mode
...

//Adjust the playback speed to normal.
((uint8_t *)&sample[0])[1] = sound_buff[i * 2 + 1];   
((uint8_t *)&sample[0])[0] = sound_buff[i * 2];

sample[1] = sample[0];  
```

#### 2.3 PlayBackFromSD

Fixed the issue where volume could not be adjusted. Added a software gain function to control volume.

```
Audio rak_audio;   // add 

void setup(){
	rak_audio.setVolume(5); //The volume level can be set to 0-21
}

void play_task(void){
    uint32_t s32 = rak_audio.Gain(sample); // 
    I2S.write((const uint8_t *)&s32, sizeof(uint32_t));
}
```

#### 2.4 HighRatePDMWiFiClient/HighRatePDMWiFiClient_RAK18003

The issue that caused the system to enter an infinite loop (while(1);) after a connection failure, which would cause the system to stop running completely and terminate all audio detection or recovery operations, has been fixed. The infinite loop has now been removed, and a return statement is executed instead.

```
void send_to_host(){
  if (!client.connect(host, httpPort)) {
    // while (1);
    return;   
 }}
```

## 3.Voice recognition test logs (RAK3112 + RAK18003 + RAK18080 + RAK18060)

### 3.1 Test phenomenon

- Wake word **"Hey RAK Star"** can be detected (CMD ID `1001`)
- Command words can be detected after wake (e.g. **"Play Music"**, **"Stop"**)
- The playback task prints the received event (`[DSPG] event in task: ...`)

### 3.2 Example serial log（DSPG_PlayBack.ino）

```
***************** 1
CMD ID: 1001
Hey RAK Star
[DSPG] event in task: id=1001, cmd=Hey RAK Star
DSPG Switch to Command Stage

***************** 3
CMD ID: 2001
Play Music
[DSPG] event in task: id=2001, cmd=Play Music

***************** 4
CMD ID: 2014
Stop
[DSPG] event in task: id=2014, cmd=Stop
```

### 3.3 Example serial log（VoiceRecognition_DSPG.ino）

This demo prints recognized wake/command words continuously (no prompt playback).

```
***************** 1
pinstate:FDDA -> FDDA
CMD ID: 1001
Hey RAK Star

***************** 2
pinstate:FDDA -> FDDA
CMD ID: 2001
Play Music

***************** 4
pinstate:FDDA -> FDDA
CMD ID: 2014
Stop

***************** 6
pinstate:FDDA -> FDDA
CMD ID: 2003
Play Music
```
