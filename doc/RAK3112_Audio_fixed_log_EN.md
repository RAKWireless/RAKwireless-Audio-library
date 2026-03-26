# RAK3112-Audio Change Log 

> This document documents key changes in the form of **code snippets with line numbers** for viewing on the Git platform without relying on local images.

---

## 1. BSP changes

### 1.1 Add RAK3112 variant macro in BSP (`pins_arduino.h` / `Pins_variant.h`)

Add the following macro to your RAK3112 variant header (BSP path depends on your Arduino-ESP32 install):

```c
// (BSP) Add near file top
// L1-L4
#ifndef _VARIANT_RAK3112_
#define _VARIANT_RAK3112_
#endif
```

---

## 2. Audio-Library changes (Arduino library `src/`)

### 2.1 `audio.h`: include `ESP_I2S.h` for RAK3112

```c
// audio.h
// L1-L10 (example)
#ifdef _VARIANT_RAK3112_
  #include <ESP_I2S.h>
#else
  #include <I2S.h>
#endif
```

### 2.2 `audio.h`: RAK3112 definitions

```c
// audio.h
// L1-L10 (example)
#elif defined (_VARIANT_RAK3112_)
#define CYBERON_FILEPATH    "Cyberon_License"
#define Stereo              2
#define BUFFER_SIZE         1920
```

---

## 3. `src/dspg.cpp` changes (full list for this PR)

This section summarizes **all RAK3112-related changes** currently present in the repository `src/dspg.cpp`.

### 3.1 RAK3112 I2S initialization (`DSPG::i2sInit`)

File: `src/dspg.cpp`

```c
// src/dspg.cpp
// L30-L50
#elif defined(_VARIANT_RAK3112_)  // for RAK3112
  I2S.setPins(WB_IO6,WB_IO5,SDA1,SCL1);

  if (!I2S.begin(I2S_MODE_STD, 16000, I2S_DATA_BIT_WIDTH_16BIT, I2S_SLOT_MODE_STEREO) )
  {
    Serial.println("Failed to initialize I2S!");
  }
```

### 3.2 RAK3112 I2S de-init (`DSPG::i2sEnd`)

File: `src/dspg.cpp`

```c
// src/dspg.cpp
// L51-L62
#elif defined(_VARIANT_RAK3112_)  // for RAK3112
  I2S.end();
```

### 3.3 RAK3112 interrupt trigger selection (`DSPG::detectedCallback`)

Problem background:
- DSPG INT is active-low; on ESP32-S3, using level-trigger (LOW) can cause an interrupt storm.
- For RAK3112 we use an **edge trigger** to keep the system stable.

File: `src/dspg.cpp`

```c
// src/dspg.cpp
// L331-L341
pinMode(FW_INT_PIN, INPUT_PULLUP);
#if defined(_VARIANT_RAK3112_)
  /* RAK3112: use edge trigger to avoid interrupt storms. */
  attachInterrupt(FW_INT_PIN, function, FALLING);
#else
  attachInterrupt(FW_INT_PIN, function, CHANGE);
#endif
```

### 3.4 RAK3112 VT event parsing robustness (`DSPG::eventCallBack`)

Issue:
- On some runs the VT event flags may not behave like a single constant (`interrupt_events == 2`).
- On RAK3112 we make parsing more robust by:
  - accepting `VT1_DET | VT2_DET`
  - reading both VT1/VT2 WORD_ID, and selecting a valid source (fallback if one reads as 0)
  - keeping non-RAK3112 behavior unchanged

File: `src/dspg.cpp`

```c
// src/dspg.cpp
// L1427-L1515 (excerpt)
uint16_t interrupt_events = dbmdx_read_register((uint16_t)(DETECT_AND_SYS_EVENTS));
delay(10);
dbmdx_write_register(DETECT_AND_SYS_EVENTS, interrupt_events);
delay(20);

#if defined(_VARIANT_RAK3112_)
{
  const uint16_t vt_ev_mask = (uint16_t)(VT1_DET | VT2_DET);
  if ((interrupt_events & vt_ev_mask) == 0)
  {
    command[0] = '\0';
    *command_id = 0;
    event = false;
    return;
  }

  const int vt1_word_id = (int)dbmdx_read_register((uint16_t)(VT1_REGS_OFFSET | VT_REG_WORD_ID));
  const int vt2_word_id = (int)dbmdx_read_register((uint16_t)(VT2_REGS_OFFSET | VT_REG_WORD_ID));

  if ((interrupt_events & VT2_DET) && (vt2_word_id != 0))
    VT_offset = VT2_REGS_OFFSET;
  else if ((interrupt_events & VT1_DET) && (vt1_word_id != 0))
    VT_offset = VT1_REGS_OFFSET;
  else if (vt1_word_id != 0)
    VT_offset = VT1_REGS_OFFSET;
  else
    VT_offset = VT2_REGS_OFFSET;
}
#else
  VT_offset = VT1_REGS_OFFSET;
#endif

word_id = dbmdx_read_register((uint16_t)(VT_offset | VT_REG_WORD_ID));

#if defined(_VARIANT_RAK3112_)
  *command_id = word_id;
#else
  if (interrupt_events == 2)
  {
    *command_id = word_id;
  }
#endif
```

### 3.5 RAK3112 SPI re-init guards in register read/write

To improve stability on ESP32 targets, the code re-initializes SPI for register access on RAK11200 and RAK3112.

File: `src/dspg.cpp`

```c
// src/dspg.cpp
// L1642-L1657
#if defined(_VARIANT_RAK11200_) || defined(_VARIANT_RAK3112_)  // add rak3112
  SPI_USER.end();
  SPI_USER.begin();
#endif
```

```c
// src/dspg.cpp
// L1702-L1715
#if defined(_VARIANT_RAK11200_) || defined(_VARIANT_RAK3112_)  // add rak3112
  SPI_USER.end();
  SPI_USER.begin();
#endif
```

---

## 4. Speech recognition roadmap note

The current solution uses **RAK18080 (DSPG)** for wake-word + command recognition.

Future work:
- gradually add **AI speech recognition** capabilities (e.g. on-device / edge inference).

