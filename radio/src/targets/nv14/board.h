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

#include "board_common.h"
#include "hal.h"
#include "hal/serial_port.h"

#define FLASHSIZE                       0x200000
#define BOOTLOADER_SIZE                 0x20000
#define FIRMWARE_ADDRESS                0x08000000

#define MB                              *1024*1024
#define LUA_MEM_EXTRA_MAX               (2 MB)    // max allowed memory usage for Lua bitmaps (in bytes)
#define LUA_MEM_MAX                     (6 MB)    // max allowed memory usage for complete Lua  (in bytes), 0 means unlimited

extern uint16_t sessionTimer;

#define SLAVE_MODE()                    (g_model.trainerData.mode == TRAINER_MODE_SLAVE)

// initilizes the board for the bootloader
#define HAVE_BOARD_BOOTLOADER_INIT 1
void boardBootloaderInit();

// Board driver
void boardInit();
void boardOff();

// CPU Unique ID
#define LEN_CPU_UID                     (3*8+2)
void getCPUUniqueID(char * s);

// SD driver
#define BLOCK_SIZE                      512 /* Block Size in Bytes */
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
#define SD_IS_HC()                      (0)
#define SD_GET_SPEED()                  (0)
#define sdInit()
#define sdMount()
#define sdDone()
#define SD_CARD_PRESENT()               true
#endif

// Flash Write driver
#define FLASH_PAGESIZE 256
void unlockFlash();
void lockFlash();
void flashWrite(uint32_t * address, const uint32_t * buffer);
uint32_t isFirmwareStart(const uint8_t * buffer);
uint32_t isBootloaderStart(const uint8_t * buffer);

// SDRAM driver
void SDRAM_Init();

enum {
  PCBREV_NV14 = 0,
  PCBREV_EL18 = 1,
};

typedef struct {
  uint8_t pcbrev;
} HardwareOptions;

extern HardwareOptions hardwareOptions;

#if !defined(SIMU)

#define INTERNAL_MODULE_OFF()                                     \
  do {                                                            \
    if (hardwareOptions.pcbrev == PCBREV_NV14)                    \
      GPIO_SetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN);   \
    else                                                          \
      GPIO_ResetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN); \
  } while (0)

#define INTERNAL_MODULE_ON()                                      \
  do {                                                            \
    if (hardwareOptions.pcbrev == PCBREV_NV14)                    \
      GPIO_ResetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN); \
    else                                                          \
      GPIO_SetBits(INTMODULE_PWR_GPIO, INTMODULE_PWR_GPIO_PIN);   \
  } while (0)

#define EXTERNAL_MODULE_ON()            GPIO_SetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)
#define EXTERNAL_MODULE_OFF()           GPIO_ResetBits(EXTMODULE_PWR_GPIO, EXTMODULE_PWR_GPIO_PIN)

#define BLUETOOTH_MODULE_ON()           GPIO_ResetBits(BLUETOOTH_ON_GPIO, BLUETOOTH_ON_GPIO_PIN)
#define BLUETOOTH_MODULE_OFF()          GPIO_SetBits(BLUETOOTH_ON_GPIO, BLUETOOTH_ON_GPIO_PIN)

#else

#define INTERNAL_MODULE_OFF()
#define INTERNAL_MODULE_ON()
#define EXTERNAL_MODULE_ON()
#define EXTERNAL_MODULE_OFF()
#define BLUETOOTH_MODULE_ON()
#define BLUETOOTH_MODULE_OFF()
#define IS_INTERNAL_MODULE_ON()         (false)
#define IS_EXTERNAL_MODULE_ON()         (false)

#endif // defined(SIMU)

#define EXTERNAL_MODULE_PWR_OFF         EXTERNAL_MODULE_OFF
#define IS_UART_MODULE(port)            (port == INTERNAL_MODULE)
#define IS_PXX2_INTERNAL_ENABLED()      (false)

#if !defined(NUM_FUNCTIONS_SWITCHES)
#define NUM_FUNCTIONS_SWITCHES        0
#endif

#define NUM_TRIMS_KEYS                  (NUM_TRIMS * 2)

#define DEFAULT_STICK_DEADZONE          2

// 2 pots without detent
#define DEFAULT_POTS_CONFIG   \
  (POT_WITHOUT_DETENT << 0) + \
      (POT_WITHOUT_DETENT << 2)

#define BATTERY_WARN                  36 // 3.6V
#define BATTERY_MIN                   35 // 3.5V
#define BATTERY_MAX                   42 // 4.2V

enum EnumPowerupState
{
  BOARD_POWER_OFF = 0xCAFEDEAD,
  BOARD_POWER_ON = 0xDEADBEEF,
  BOARD_STARTED = 0xBAADF00D,
  BOARD_REBOOT = 0xC00010FF,
};

bool UNEXPECTED_SHUTDOWN();
void SET_POWER_REASON(uint32_t value);

#if defined(__cplusplus) && !defined(SIMU)
extern "C" {
#endif

// Power driver
#define SOFT_PWR_CTRL
#define POWER_ON_DELAY               10 // 1s
void pwrInit();
void extModuleInit();
uint32_t pwrCheck();
uint32_t lowPowerCheck();

void pwrOn();
void pwrSoftReboot();
void pwrOff();
void pwrResetHandler();
bool pwrPressed();
bool pwrOffPressed();
#if defined(PWR_EXTRA_SWITCH_GPIO)
  bool pwrForcePressed();
#else
  #define pwrForcePressed() false
#endif
uint32_t pwrPressedDuration();;
  
const etx_serial_port_t* auxSerialGetPort(int port_nr);
  
// LCD driver
#define LCD_W                           320
#define LCD_H                           480

#define LCD_PHYS_W                      320
#define LCD_PHYS_H                      480

#define LCD_DEPTH                       16
#define LCD_CONTRAST_DEFAULT            20

void lcdInit();
void lcdCopy(void * dest, void * src);

void DMAFillRect(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, uint16_t w, uint16_t h, uint16_t color);
void DMACopyBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void DMACopyAlphaBitmap(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint16_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h);
void DMACopyAlphaMask(uint16_t * dest, uint16_t destw, uint16_t desth, uint16_t x, uint16_t y, const uint8_t * src, uint16_t srcw, uint16_t srch, uint16_t srcx, uint16_t srcy, uint16_t w, uint16_t h, uint16_t bg_color);
void DMABitmapConvert(uint16_t * dest, const uint8_t * src, uint16_t w, uint16_t h, uint32_t format);

void lcdOff();
void lcdOn();

#define lcdRefreshWait(...)

// Backlight driver
#define BACKLIGHT_LEVEL_MAX             100
#define BACKLIGHT_FORCED_ON             BACKLIGHT_LEVEL_MAX + 1
#define BACKLIGHT_LEVEL_MIN             1

extern bool boardBacklightOn;
void backlightLowInit( void );
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
#define USB_NAME                        "FlySky NV14"
#define USB_MANUFACTURER                'F', 'l', 'y', 'S', 'k', 'y', ' ', ' '  /* 8 bytes */
#define USB_PRODUCT                     'N', 'V', '1', '4', ' ', ' ', ' ', ' '  /* 8 Bytes */

#if defined(__cplusplus) && !defined(SIMU)
}
#endif

// Audio driver
void audioInit();
void audioConsumeCurrentBuffer();
void audioSpiWriteBuffer(const uint8_t * buffer, uint32_t size);
void audioSpiSetSpeed(uint8_t speed);
uint8_t audioHardReset();
uint8_t audioSoftReset();
void audioSendRiffHeader();
void audioOn();
void audioOff();
bool isAudioReady();
bool audioChipReset();

#define SPI_SPEED_2                    0
#define SPI_SPEED_4                    1
#define SPI_SPEED_8                    2
#define SPI_SPEED_16                   3
#define SPI_SPEED_32                   4
#define SPI_SPEED_64                   5
#define SPI_SPEED_128                  6
#define SPI_SPEED_256                  7

#define audioDisableIrq()             // interrupts must stay enabled on Horus
#define audioEnableIrq()              // interrupts must stay enabled on Horus
#if defined(PCBNV14)
#define setSampleRate(freq)
#else
void setSampleRate(uint32_t frequency);
#endif
void setScaledVolume(uint8_t volume);
void setVolume(uint8_t volume);
int32_t getVolume();
#define VOLUME_LEVEL_MAX               23
#define VOLUME_LEVEL_DEF               12

// Telemetry driver
#define INTMODULE_FIFO_SIZE            512
#define TELEMETRY_FIFO_SIZE            512

// Haptic driver
void hapticInit();
void hapticDone();
void hapticOff();
void hapticOn(uint32_t pwmPercent);

// Second serial port driver
//#define AUX_SERIAL
#define DEBUG_BAUDRATE                  115200
#define LUA_DEFAULT_BAUDRATE            115200

extern uint8_t currentTrainerMode;
void checkTrainerSettings();

// Touch panel driver
bool touchPanelEventOccured();
struct TouchState touchPanelRead();
struct TouchState getInternalTouchState();

#define BATTERY_DIVIDER 2942

#endif // _BOARD_H_
