/*
 * Copyright (C) EdgeTX
 *
 * Based on code named
 *   opentx - https://github.com/opentx/opentx
 *   th9x - http://code.google.com/p/th9x
 *   er9x - http://code.google.com/p/er9x
 *   gruvin9x - http://code.google.com/p/gruvin9x
 *
 * License GPLv2: http://www.gnu.org/licenses/gpl-2.0.html
 *
 * This program is free software; you can redistribute it and/or modify
 * it under the terms of the GNU General Public License version 2 as
 * published by the Free Software Foundation.
 *
 * This program is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 */

#ifndef _BOARD_H_
#define _BOARD_H_

#include "definitions.h"
#include "opentx_constants.h"

// Defines used in board_common.h
#define ROTARY_ENCODER_NAVIGATION

#include "board_common.h"
#include "hal.h"
#include "hal/serial_port.h"

#include "watchdog_driver.h"

#if defined(HARDWARE_TOUCH)
#include "tp_gt911.h"
#endif


PACK(typedef struct {
  uint8_t pcbrev:2;
  uint8_t pxx2Enabled:1;
}) HardwareOptions;

extern HardwareOptions hardwareOptions;

#define FLASHSIZE                      0x200000
#define BOOTLOADER_SIZE                0x20000
#define FIRMWARE_ADDRESS               0x08000000

#define MB                             *1024*1024
#define LUA_MEM_EXTRA_MAX              (2 MB)    // max allowed memory usage for Lua bitmaps (in bytes)
#define LUA_MEM_MAX                    (6 MB)    // max allowed memory usage for complete Lua  (in bytes), 0 means unlimited


extern uint16_t sessionTimer;

#define SLAVE_MODE()                   (g_model.trainerData.mode == TRAINER_MODE_SLAVE)

// Board driver
void boardInit();
void boardOff();

// PCBREV driver
enum {
  // X12S
  PCBREV_X12S_LT13 = 0,
  PCBREV_X12S_GTE13 = 1,

  // X10
  PCBREV_X10_STD = 0,
  PCBREV_X10_EXPRESS = 3,
};

#if defined(SIMU)
  #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() true
#elif defined(PCBX10)
  #if defined(PCBREV_EXPRESS)
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X10_EXPRESS)
  #elif defined(RADIO_FAMILY_T16)
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (true)
  #else
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X10_STD)
  #endif
#else
  #if PCBREV >= 13
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X12S_GTE13)
  #else
    #define IS_FIRMWARE_COMPATIBLE_WITH_BOARD() (hardwareOptions.pcbrev == PCBREV_X12S_LT13)
  #endif
#endif

// SD driver
#define BLOCK_SIZE                     512 /* Block Size in Bytes */
#if !defined(SIMU) || defined(SIMU_DISKIO)
uint32_t sdIsHC();
uint32_t sdGetSpeed();
#define SD_IS_HC()                     (sdIsHC())
#define SD_GET_SPEED()                 (sdGetSpeed())
#define SD_GET_FREE_BLOCKNR()          (sdGetFreeSectors())
#define SD_CARD_PRESENT()              (~SD_PRESENT_GPIO->IDR & SD_PRESENT_GPIO_PIN)
void sdInit();
void sdMount();
void sdDone();
#define sdPoll10ms()
uint32_t sdMounted();
#else
#define SD_IS_HC()                     (0)
#define SD_GET_SPEED()                 (0)
#define sdInit()
#define sdMount()
#define sdDone()
#define SD_CARD_PRESENT()              true
#endif

// Flash Write driver
#define FLASH_PAGESIZE                 256
void unlockFlash();
void lockFlash();
void flashWrite(uint32_t * address, const uint32_t * buffer);
uint32_t isFirmwareStart(const uint8_t * buffer);
uint32_t isBootloaderStart(const uint8_t * buffer);

// SDRAM driver
void SDRAM_Init();

#if defined(INTERNAL_MODULE_PXX1) || defined(INTERNAL_MODULE_PXX2)
  #define HARDWARE_INTERNAL_RAS
#endif

// Pulses driver
#if defined(RADIO_T18) || defined(RADIO_T16)

// TX18S Workaround (see https://github.com/EdgeTX/edgetx/issues/802)
// and also T16     (see https://github.com/EdgeTX/edgetx/issues/1239)
//   Add some delay after turning the internal module ON
//   on the T16, T18 & TX18S, as they seem to have issues
//   with power supply instability
//
#define INTERNAL_MODULE_ON()                                  \
  do {                                                        \
    GPIO_SetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN); \
    delay_ms(1);                                              \
  } while (0)

#else

// Just turn the modue ON for all other targets
#define INTERNAL_MODULE_ON() \
  GPIO_SetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN)

#endif

#define INTERNAL_MODULE_OFF()   GPIO_ResetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN)
#define EXTERNAL_MODULE_ON()    GPIO_SetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
#define EXTERNAL_MODULE_OFF()   GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)

#if !defined(PXX2)
  #define IS_PXX2_INTERNAL_ENABLED()            (false)
  #define IS_PXX1_INTERNAL_ENABLED()            (true)
#elif !defined(PXX1)
  #define IS_PXX2_INTERNAL_ENABLED()            (true)
  #define IS_PXX1_INTERNAL_ENABLED()            (false)
#else
  // TODO #define PXX2_PROBE
  // TODO #define IS_PXX2_INTERNAL_ENABLED()            (hardwareOptions.pxx2Enabled)
  #define IS_PXX2_INTERNAL_ENABLED()            (true)
  #define IS_PXX1_INTERNAL_ENABLED()            (true)
#endif

#if !defined(NUM_FUNCTIONS_SWITCHES)
#define NUM_FUNCTIONS_SWITCHES        0
#endif

// POTS and SLIDERS default configuration
#if defined(RADIO_TX16S)
#define XPOS_CALIB_DEFAULT  {0x3, 0xc, 0x15, 0x1e, 0x26}
#endif

// Trims driver
#define NUM_TRIMS                               6
#define NUM_TRIMS_KEYS                          (NUM_TRIMS * 2)

// Battery driver
#if defined(PCBX10)
  // Lipo 2S
  #define BATTERY_WARN      66 // 6.6V
  #define BATTERY_MIN       67 // 6.7V
  #define BATTERY_MAX       83 // 8.3V
#else
  // NI-MH 9.6V
  #define BATTERY_WARN      87 // 8.7V
  #define BATTERY_MIN       85 // 8.5V
  #define BATTERY_MAX       115 // 11.5V
#endif

bool UNEXPECTED_SHUTDOWN();
void SET_POWER_REASON(uint32_t value);

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif

// Power driver
#define SOFT_PWR_CTRL
void pwrInit();
uint32_t pwrCheck();
void pwrOn();
void pwrOff();
void pwrResetHandler();
bool pwrPressed();
bool pwrOffPressed();
#if defined(PWR_EXTRA_SWITCH_GPIO)
  bool pwrForcePressed();
#else
  #define pwrForcePressed() false
#endif
uint32_t pwrPressedDuration();

// USB Charger
void usbChargerInit();
bool usbChargerLed();

// Led driver
void ledInit();
void ledOff();
void ledRed();
void ledBlue();
#if defined(PCBX10)
  void ledGreen();
#endif

// LCD driver
#define LCD_W                          480
#define LCD_H                          272
#define LCD_PHYS_H                     LCD_H
#define LCD_PHYS_W                     LCD_W
#define LCD_DEPTH                      16

void lcdInit();
void lcdCopy(void * dest, void * src);

void DMAFillRect(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void DMACopyBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void DMACopyAlphaBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void DMACopyAlphaMask(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint8_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h, uint16_t bg_color);
void DMABitmapConvert(uint16_t * dest, const uint8_t * src, uint16_t w, uint16_t h, uint32_t format);

#define lcdOff()              backlightEnable(0) /* just disable the backlight */

#define lcdRefreshWait(...)

// Backlight driver
#define BACKLIGHT_LEVEL_MAX     100
#define BACKLIGHT_FORCED_ON     BACKLIGHT_LEVEL_MAX + 1
#if defined(PCBX12S)
#define BACKLIGHT_LEVEL_MIN   5
#elif defined(RADIO_FAMILY_T16)
#define BACKLIGHT_LEVEL_MIN   1
#else
#define BACKLIGHT_LEVEL_MIN   46
#endif

extern bool boardBacklightOn;
void backlightInit();
void backlightEnable(uint8_t dutyCycle);
void backlightFullOn();
bool isBacklightEnabled();

#define BACKLIGHT_ENABLE()                                               \
  {                                                                      \
    boardBacklightOn = true;                                             \
    backlightEnable(globalData.unexpectedShutdown                        \
                        ? BACKLIGHT_LEVEL_MAX                            \
                        : BACKLIGHT_LEVEL_MAX - currentBacklightBright); \
  }
#define BACKLIGHT_DISABLE()                                                 \
  {                                                                         \
    boardBacklightOn = false;                                               \
    backlightEnable(globalData.unexpectedShutdown ? BACKLIGHT_LEVEL_MAX     \
                    : ((g_eeGeneral.blOffBright == BACKLIGHT_LEVEL_MIN) &&  \
                       (g_eeGeneral.backlightMode != e_backlight_mode_off)) \
                        ? 0                                                 \
                        : g_eeGeneral.blOffBright);                         \
  }

#if !defined(SIMU)
void usbJoystickUpdate();
#endif
#if defined(PCBX12S)
  #define USB_NAME                     "FrSky Horus"
  #define USB_MANUFACTURER             'F', 'r', 'S', 'k', 'y', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'H', 'o', 'r', 'u', 's', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_T16)
  #define USB_NAME                     "Jumper T16"
  #define USB_MANUFACTURER             'J', 'u', 'm', 'p', 'e', 'r', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '1', '6', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */  
#elif defined(RADIO_T18)
  #define USB_NAME                     "Jumper T18"
  #define USB_MANUFACTURER             'J', 'u', 'm', 'p', 'e', 'r', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'T', '1', '8', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */
#elif defined(RADIO_TX16S)
  #define USB_NAME                     "RM TX16S"
  #define USB_MANUFACTURER             'R', 'M', '_', 'T', 'X', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'R', 'M', ' ', 'T', 'X', '1', '6', 'S'  /* 8 Bytes */
#elif defined(PCBX10)
  #define USB_NAME                     "FrSky X10"
  #define USB_MANUFACTURER             'F', 'r', 'S', 'k', 'y', ' ', ' ', ' '  /* 8 bytes */
  #define USB_PRODUCT                  'X', '1', '0', ' ', ' ', ' ', ' ', ' '  /* 8 Bytes */
#endif

#if defined(__cplusplus) && !defined(SIMU)
}
#endif

// Audio driver
void audioInit();
void audioConsumeCurrentBuffer();
#define audioDisableIrq()             // interrupts must stay enabled on Horus
#define audioEnableIrq()              // interrupts must stay enabled on Horus
#if defined(PCBX12S)
#define setSampleRate(freq)
#else
void setSampleRate(uint32_t frequency);
#define audioWaitReady()
#endif
void setScaledVolume(uint8_t volume);
void setVolume(uint8_t volume);
int32_t getVolume();
#define VOLUME_LEVEL_MAX               23
#define VOLUME_LEVEL_DEF               12

// Telemetry driver
#define INTMODULE_FIFO_SIZE            512
#define TELEMETRY_FIFO_SIZE            512
void telemetryPortInit(uint32_t baudrate, uint8_t mode);
void telemetryPortSetDirectionInput();
void telemetryPortSetDirectionOutput();
void sportSendByte(uint8_t byte);
void sportSendBuffer(const uint8_t * buffer, uint32_t count);
bool sportGetByte(uint8_t * byte);
void telemetryClearFifo();
extern uint32_t telemetryErrors;

// soft-serial
void telemetryPortInvertedInit(uint32_t baudrate);


// Aux serial port driver
#if defined(RADIO_TX16S)
  #define DEBUG_BAUDRATE                  400000
  #define LUA_DEFAULT_BAUDRATE            115200
#else
  #define DEBUG_BAUDRATE                  115200
  #define LUA_DEFAULT_BAUDRATE            115200
#endif

const etx_serial_port_t* auxSerialGetPort(int port_nr);

// Haptic driver
void hapticInit();
void hapticDone();
void hapticOff();
void hapticOn(uint32_t pwmPercent);

// BT driver
#define BT_TX_FIFO_SIZE    64
#define BT_RX_FIFO_SIZE    256
#define BLUETOOTH_BOOTLOADER_BAUDRATE  230400
#define BLUETOOTH_FACTORY_BAUDRATE     57600
#define BLUETOOTH_DEFAULT_BAUDRATE     115200
void bluetoothInit(uint32_t baudrate, bool enable);
void bluetoothWriteWakeup();
uint8_t bluetoothIsWriting();
void bluetoothDisable();

#if defined (RADIO_TX16S)
  #define BATTERY_DIVIDER 1495
#else
  #define BATTERY_DIVIDER 1629
#endif 

#endif // _BOARD_H_
