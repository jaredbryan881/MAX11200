#ifndef MAX11200_H
#define MAX11200_H

#include <stdint.h>
#include <stdbool.h>
#include "stm32wlxx_hal.h"

/***********************************
 * Register definitions
 ***********************************/
/* Status register (read only)
   Contains bits on general chip operational status, e.g. Data Ready (DRDY) and error flags.
   Reading this register does not affect ongoing conversions.
   See Table 11 */
#define MAX11200_STAT1_REG 0x00U
/* Status register bits
   | B7    | B6    | B5    | B4    | B3 | B2 | B1    | B0  | 
   | SYSOR | RATE2 | RATE1 | RATE0 | OR | UR | MSTAT | RDY |*/
// SYSOR: system gain overrange bit. SYSOR=1 means system gain calibration over range
#define MAX11200_CMD_SYSOR 0x00
// RATE[2:0]: data rate bits. Rate corresponds to the result in the DATA register
#define MAX11200_CMD_RATE0 0x01
#define MAX11200_CMD_RATE1 0x02
#define MAX11200_CMD_RATE2 0x04
// OR: overrange bit. OR=1 means conversion result exceeds max value.
#define MAX11200_CMD_OR    0x00
// UR: underrange bit. UR=1 menas conversion result exceeds min value.
#define MAX11200_CMD_UR    0x00
// MSTAT: measurement status bit. MSTAT=1 when measurement in progress. 
#define MAX11200_CMD_MSTAT 0x00
// RDY: ready bit. RDY=1 if conversion result is available.
#define MAX11200_CMD_RDY   0x00

/* Control 1 register (read/write)
   Contains bits that configure additional ADC funcionality, including
   - internal oscillator frequency
   - unipolar or bipolar input range
   - internal or external clock
   - enable or disable reference and input signal buffers
   - output data format (offset binary or two's complement)
   - single-cycle or continuous conversion mode
   See Table 12 */
#define MAX11200_CTRL1_REG    0x01U
/* Control 1 register bits
   | B7    | B6   | B5     | B4     | B3     | B2     | B1     | B0     | 
   | LINEF | U/~B | EXTCLK | REFBUF | SIGBUF | FORMAT | SCYCLE | UNUSED |*/
// LINEF: line frequency bit. LINEF=1 for 50 Hz power mains, LINEF=0 for 60 Hz power mains
#define MAX11200_CTRL1_LINEF  (1U << 7)
// U/~B: unipolar/bipolar bit. U/~B=1 for unipolar input range, U/~B=0 for bipolar input range.
#define MAX11200_CTRL1_UB     (1U << 6)
// EXTCLK: external clock bit. EXTCLK=1 for external system clock, EXTCLK=0 enables the internal clock.
#define MAX11200_CTRL1_EXTCLK (1U << 5)
// REFBUF: reference buffer bit. REFBUF=1 enables reference buffers.
#define MAX11200_CTRL1_REFBUF (1U << 4)
// SIGBUF: signal buffer bit. SIGBUF=1 enables signal buffers.
#define MAX11200_CTRL1_SIGBUF (1U << 3)
// FORMAT: format bit controlling digital format of the data. FORMAT=0 for two's complement, FORMAT=1 for offset binary.
#define MAX11200_CTRL1_FORMAT (1U << 2)
// SCYCLE: single-cycle bit. SCYCLE=1 for "no-latency" single-conversion mode or SCYCLE=0 for "latent" continuous-conversion mode.
#define MAX11200_CTRL1_SCYCLE (1U << 1)

/* Control 2 register (read/write)
   Contains bits that configure GPIOs as inputs or outputs and their values
   See Table 13 */
#define MAX11200_CTRL2_REG   0x02U
/* Control 2 register bits
   | B7   | B6   | B5   | B4   | B3   | B2   | B1   | B0   | 
   | DIR4 | DIR3 | DIR2 | DIR1 | DIO4 | DIO3 | DIO2 | DIO1 |*/
// DIR[4:1]: direction bits configure the direction of the DIO bit. DIR[i]=0 means DIO[i] configured as input.
#define MAX11200_CTRL2_DIR1  0x10
#define MAX11200_CTRL2_DIR2  0x20
#define MAX11200_CTRL2_DIR3  0x40
#define MAX11200_CTRL2_DIR4  0x80
// DIO[4:1]: data input/output bits associated with the GPIO ports
#define MAX11200_CTRL2_DIO1  0x01
#define MAX11200_CTRL2_DIO2  0x02
#define MAX11200_CTRL2_DIO3  0x04
#define MAX11200_CTRL2_DIO4  0x08


/* Control 3 register (read/write)
   Contains bits that configure the MAX11210 programmable gain setting and the calibration register settings
   See Table 14 */
#define MAX11200_CTRL3_REG    0x03U
/* Control 2 register bits
   | B7      | B6      | B5      | B4     | B3     | B2    | B1    | B0       | 
   | DGAIN2* | DGAIN1* | DGAIN0* | NOSYSG | NOSYSO | NOSCG | NOSCO | RESERVED |*/
// DGAIN[2:0]: digital gain bits. Only defined for the MAX11210.
#define MAX11210_CTRL3_DGAIN0 0x20
#define MAX11210_CTRL3_DGAIN1 0x40
#define MAX11210_CTRL3_DGAIN2 0x80
// NOSYSG: no system gain bit. NOSYSG=1 disables the use of the system gain value when computing final offset and gain corrected data value.
#define MAX11200_CTRL3_NOSYSG 0x10
// NOSYSO: no system offset bit. NOSYSO=1 disables the use of the system offset value when computing final offset and gain corrected data value.
#define MAX11200_CTRL3_NOSYSO 0x08
// NOSCG: no self-calibration gain bit. NOSCG=1 disables the use of the self-calibration gain value when computing the final offset and gain corrected data value.
#define MAX11200_CTRL3_NOSCG  0x04
// NOSCO: no self-calibration offset bit. NOSCO=1 disables the use of the self-calibration offset value when computing the final offset and gain corrected data value.
#define MAX11200_CTRL3_NOSCO  0x02

/* Data register (read only)
   See Table 15*/
#define MAX11200_DATA_REG 0x04U

/* System Offset Calibration register (read/write)
   Bits contain the digital value that corrects the data for system zero scale.
   See Table 17 */
#define MAX11200_SOC_REG 0x05U

/* System Gain Calibration register (read/write)
   Bits contain the digital value that corrects the data for system full scale
   See Table 18 */
#define MAX11200_SGC_REG 0x06U

/* Self-Calibration Offset register (read/write)
   Bits contain the value that corrects the data for chip zero scale
   See Table 19 */
#define MAX11200_SCOC_REG 0x07U

/* Self-Calibration Gain register (read/write)
   Bits contain the value that corrects the data for chip full scale
   See Table 20 */
#define MAX11200_SCGC_REG 0x08U


/******************** 
  Function prototypes
 ********************/
void MAX11200_Init(void);
int32_t MAX11200_ReadData24(void);

#endif