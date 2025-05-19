#ifndef MAX11200_H
#define MAX11200_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32wlxx_hal.h"
#include "cmsis_os.h"
#include "spiMutex.h"

typedef struct
{
  SPI_HandleTypeDef *hspi;
  GPIO_TypeDef *CS_GPIO_Port;
  uint16_t CS_GPIO_Pin;
} MAX11200_ADC;

// Config data structure for the CTRL1 register
typedef struct
{
    uint8_t scycle;             // Single-cycle or continuous
    uint8_t format;             // Offset-binary or two's complement
    uint8_t sigbuf;             // Signal buffer enable/disable
    uint8_t refbuf;             // Reference buffer enable/disable
    uint8_t extclk;             // Internal or external clock
    uint8_t unipolar_bipolar;   // Unipolar or bipolar input range
    uint8_t line_filter;        // 50 Hz or 60 Hz line filter
} MAX11200_Config_Data;

/***********************************
 * Register definitions, command bytes, and macros
 ***********************************/
/* Status register (read only)
   Contains bits on general chip operational status, e.g. Data Ready (DRDY) and error flags.
   Reading this register does not affect ongoing conversions.
   See Table 11 */
#define MAX11200_STAT1_REG 0x00
/* Status register bits
   | B7    | B6    | B5    | B4    | B3 | B2 | B1    | B0  | 
   | SYSOR | RATE2 | RATE1 | RATE0 | OR | UR | MSTAT | RDY |*/
// SYSOR: system gain overrange bit. SYSOR=1 means system gain calibration over range
#define MAX11200_STAT1_SYSOR (1 << 7)
// RATE[2:0]: data rate bits. Rate corresponds to the result in the DATA register
#define MAX11200_STAT1_RATE0 (1 << 6)
#define MAX11200_STAT1_RATE1 (1 << 5)
#define MAX11200_STAT1_RATE2 (1 << 4)
// OR: overrange bit. OR=1 means conversion result exceeds max value.
#define MAX11200_STAT1_OR    (1 << 3)
// UR: underrange bit. UR=1 menas conversion result exceeds min value.
#define MAX11200_STAT1_UR    (1 << 2)
// MSTAT: measurement status bit. MSTAT=1 when measurement in progress. 
#define MAX11200_STAT1_MSTAT (1 << 1)
// RDY: ready bit. RDY=1 if conversion result is available.
#define MAX11200_STAT1_RDY   (1 << 0)

/* Control 1 register (read/write)
   Contains bits that configure additional ADC funcionality, including
   - internal oscillator frequency
   - unipolar or bipolar input range
   - internal or external clock
   - enable or disable reference and input signal buffers
   - output data format (offset binary or two's complement)
   - single-cycle or continuous conversion mode
   See Table 12 */
#define MAX11200_CTRL1_REG    0x01
/* Control 1 register bits
   | B7    | B6   | B5     | B4     | B3     | B2     | B1     | B0     | 
   | LINEF | U/~B | EXTCLK | REFBUF | SIGBUF | FORMAT | SCYCLE | UNUSED |*/
// LINEF: line frequency bit. LINEF=1 for 50 Hz power mains, LINEF=0 for 60 Hz power mains
#define MAX11200_CTRL1_LINEF  (1 << 7)
// U/~B: unipolar/bipolar bit. U/~B=1 for unipolar input range, U/~B=0 for bipolar input range.
#define MAX11200_CTRL1_UB     (1 << 6)
// EXTCLK: external clock bit. EXTCLK=1 for external system clock, EXTCLK=0 enables the internal clock.
#define MAX11200_CTRL1_EXTCLK (1 << 5)
// REFBUF: reference buffer bit. REFBUF=1 enables reference buffers.
#define MAX11200_CTRL1_REFBUF (1 << 4)
// SIGBUF: signal buffer bit. SIGBUF=1 enables signal buffers.
#define MAX11200_CTRL1_SIGBUF (1 << 3)
// FORMAT: format bit controlling digital format of the data. FORMAT=0 for two's complement, FORMAT=1 for offset binary.
#define MAX11200_CTRL1_FORMAT (1 << 2)
// SCYCLE: single-cycle bit. SCYCLE=1 for "no-latency" single-conversion mode or SCYCLE=0 for "latent" continuous-conversion mode.
#define MAX11200_CTRL1_SCYCLE (1 << 1)

/* Control 2 register (read/write)
   Contains bits that configure GPIOs as inputs or outputs and their values
   See Table 13 */
#define MAX11200_CTRL2_REG   0x02
/* Control 2 register bits
   | B7   | B6   | B5   | B4   | B3   | B2   | B1   | B0   | 
   | DIR4 | DIR3 | DIR2 | DIR1 | DIO4 | DIO3 | DIO2 | DIO1 |*/
// DIR[4:1]: direction bits configure the direction of the DIO bit. DIR[i]=0 means DIO[i] configured as input.
#define MAX11200_CTRL2_DIR4  (1 << 7)
#define MAX11200_CTRL2_DIR3  (1 << 6)
#define MAX11200_CTRL2_DIR2  (1 << 5)
#define MAX11200_CTRL2_DIR1  (1 << 4)
// DIO[4:1]: data input/output bits associated with the GPIO ports
#define MAX11200_CTRL2_DIO4  (1 << 3)
#define MAX11200_CTRL2_DIO3  (1 << 2)
#define MAX11200_CTRL2_DIO2  (1 << 1)
#define MAX11200_CTRL2_DIO1  (1 << 0)

/* Control 3 register (read/write)
   Contains bits that configure the MAX11210 programmable gain setting and the calibration register settings
   See Table 14 */
#define MAX11200_CTRL3_REG    0x03
/* Control 3 register bits
   | B7      | B6      | B5      | B4     | B3     | B2    | B1    | B0       | 
   | DGAIN2* | DGAIN1* | DGAIN0* | NOSYSG | NOSYSO | NOSCG | NOSCO | RESERVED |*/
// DGAIN[2:0]: digital gain bits. Only defined for the MAX11210.
#define MAX11210_CTRL3_DGAIN2 (1 << 7)
#define MAX11210_CTRL3_DGAIN1 (1 << 6)
#define MAX11210_CTRL3_DGAIN0 (1 << 5)
// NOSYSG: no system gain bit. NOSYSG=1 disables the use of the system gain value when computing final offset and gain corrected data value.
#define MAX11200_CTRL3_NOSYSG (1 << 4)
// NOSYSO: no system offset bit. NOSYSO=1 disables the use of the system offset value when computing final offset and gain corrected data value.
#define MAX11200_CTRL3_NOSYSO (1 << 3)
// NOSCG: no self-calibration gain bit. NOSCG=1 disables the use of the self-calibration gain value when computing the final offset and gain corrected data value.
#define MAX11200_CTRL3_NOSCG  (1 << 2)
// NOSCO: no self-calibration offset bit. NOSCO=1 disables the use of the self-calibration offset value when computing the final offset and gain corrected data value.
#define MAX11200_CTRL3_NOSCO  (1 << 1)

/* Data register (read only)
   See Table 15*/
#define MAX11200_DATA_REG 0x04

/* System Offset Calibration register (read/write)
   Bits contain the digital value that corrects the data for system zero scale.
   See Table 17 */
#define MAX11200_SOC_REG 0x05

/* System Gain Calibration register (read/write)
   Bits contain the digital value that corrects the data for system full scale
   See Table 18 */
#define MAX11200_SGC_REG 0x06

/* Self-Calibration Offset register (read/write)
   Bits contain the value that corrects the data for chip zero scale
   See Table 19 */
#define MAX11200_SCOC_REG 0x07

/* Self-Calibration Gain register (read/write)
   Bits contain the value that corrects the data for chip full scale
   See Table 20 */
#define MAX11200_SCGC_REG 0x08

/*************
  Command byte
**************/
#define MAX11200_START (1 << 7)
#define MAX11200_MODE0 0x00
#define MAX11200_MODE1 (1 << 6)
#define MAX11200_WRITE 0x00
#define MAX11200_READ (1 << 0)

// User-friendly config bit definitions
#define MAX11200_CONFIG_CONVERSION_CONTINUOUS 0x00
#define MAX11200_CONFIG_CONVERSION_SINGLE (1 << 1)

#define MAX11200_CONFIG_FORMAT_2COMPLEMENT 0x00
#define MAX11200_CONFIG_FORMAT_OFFSET_BINARY (1 << 2)

#define MAX11200_CONFIG_SIGBUF_ENABLE (1 << 3)
#define MAX11200_CONFIG_SIGBUF_DISABLE 0x00

#define MAX11200_CONFIG_REFBUF_ENABLE (1 << 4)
#define MAX11200_CONFIG_REFBUF_DISABLE 0x00

#define MAX11200_CONFIG_CLK_EXTERNAL (1 << 5)
#define MAX11200_CONFIG_CLK_INTERNAL 0x00

#define MAX11200_CONFIG_UNIPOLAR (1 << 6)
#define MAX11200_CONFIG_BIPOLAR 0x00

#define MAX11200_CONFIG_LINEF_50HZ (1 << 7)
#define MAX11200_CONFIG_LINEF_60HZ 0x00

#define MAX11200_STAT_MEASURE_RDY (1 << 0)
#define MAX11200_STAT_MODULATOR_BSY (1 << 1)
#define MAX11200_STAT_MEASURE_UNDER_RANGE (1 << 2)
#define MAX11200_STAT_MEASURE_OVER_RANGE (1 << 3)

// Data rate settings for single-cycle mode (SCYCLE=1)
#define MAX11200_SCYCLE_RATE_1SPS   0x00
#define MAX11200_SCYCLE_RATE_2p5SPS 0x01
#define MAX11200_SCYCLE_RATE_5SPS   0x02
#define MAX11200_SCYCLE_RATE_10SPS  0x03
#define MAX11200_SCYCLE_RATE_15SPS  0x04
#define MAX11200_SCYCLE_RATE_30SPS  0x05
#define MAX11200_SCYCLE_RATE_60SPS  0x06
#define MAX11200_SCYCLE_RATE_120SPS 0x07

// Data rate settings for continuous mode (SCYCLE=0)
#define MAX11200_CONT_RATE_60SPS  0x04
#define MAX11200_CONT_RATE_120SPS 0x05
#define MAX11200_CONT_RATE_240SPS 0x06
#define MAX11200_CONT_RATE_480SPS 0x07

#define MAX11200_CMD_RATE0     0x01
#define MAX11200_CMD_RATE1     0x02
#define MAX11200_CMD_RATE2     0x04
#define MAX11200_CMD_IMPD      0x08
#define MAX11200_CMD_CAL1      0x20
#define MAX11200_CMD_CAL0      0x10

/************ 
  Public API
 ************/
// Initialize internal driver state and initial registers
void MAX11200_Init(MAX11200_ADC *adc);

// Set default fields in config struct
void MAX11200_Init_Config(MAX11200_Config_Data *config);

// Read CTRL1 config from device
void MAX11200_Read_Config(MAX11200_ADC *adc, MAX11200_Config_Data *config);

// Write CTRL1 config to device
void MAX11200_Write_Config(MAX11200_ADC *adc, MAX11200_Config_Data *config);

// Perform self-calibration (offset and gain)
void MAX11200_Self_Calibration(MAX11200_ADC *adc, uint32_t *calib_offset, uint32_t *calib_gain);

// Read the status registers (STAT1)
uint8_t MAX11200_Read_Stat(MAX11200_ADC *adc);

// Check if conversion data is ready (RDY=1)
int32_t MAX11200_Conversion_Ready(MAX11200_ADC *adc);

// Check if conversion is in progress (MSTAT=1)
int32_t MAX11200_Measure_In_Progress(MAX11200_ADC *adc);

// Start a single-cycle conversion at a specified rate
// Block until complete
uint32_t MAX11200_Convert(MAX11200_ADC *adc, uint8_t rate);

// Read data register
int32_t MAX11200_ReadData24(MAX11200_ADC *adc);

// Start a single-cycle conversion at a specified rate
// Non-blocking
void MAX11200_Start_Conversion(MAX11200_ADC *adc, uint8_t rate);

#endif