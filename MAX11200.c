#include "MAX11200.h"
#include "stm32wlxx_hal.h"  // Or whatever MCU you're using, e.g. stm32f4xx_hal.h

extern SPI_HandleTypeDef hspi1; // or pass it as a parameter

// Or whatever your hardware calls for
#define MAX11200_CS_GPIO_PORT  GPIOA
#define MAX11200_CS_PIN        GPIO_PIN_4

/************************************************
  Struct to hold control and status register info
*************************************************/
typedef struct
{
    uint8_t ctrl1;
    uint8_t ctrl2;
    uint8_t ctrl3;
    uint8_t stat1;
} MAX11200_ctrl_stat_regs_t;

static MAX11200_CtrlStat_Regs_t MAX11200_CtrlStat_Regs;


/**************************
  Private helper functions
***************************/
// Pull CS pin low
static inline void MAX11200_CS_Low(void){
    HAL_GPIO_WritePin(MAX11200_CS_GPIO_PORT, MAX11200_CS_PIN, GPIO_PIN_RESET);
}

// Pull CS pin high
static inline void MAX11200_CS_High(void){
    HAL_GPIO_WritePin(MAX11200_CS_GPIO_PORT, MAX11200_CS_PIN, GPIO_PIN_SET);
}

// Build a read command for the specified register
static inline uint8_t MAX11200_BuildReadCmd(uint8_t regAddr){
    return (MAX11200_START | MAX11200_MODE1 | MAX11200_READ | (regAddr << 1));
}

// Build a write command for the specified register
static inline uint8_t MAX11200_BuildWriteCmd(uint8_t regAddr){
    return (MAX11200_START | MAX11200_MODE1 | MAX11200_WRITE | (regAddr << 1));
}

// Write an 8-bit value to a specified register
static void MAX11200_WriteReg8(uint8_t regAddr, uint8_t data){
    uint8_t cmd = MAX11200_BuildWriteCmd(regAddr);

    MAX11200_CS_Low();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    MAX11200_CS_High();
}

// Read an 8-bit value from a specified register
static uint8_t MAX11200_ReadReg8(uint8_t regAddr){
    uint8_t cmd = MAX11200_BuildReadCmd(regAddr);
    uint8_t rxByte = 0x00;

    MAX11200_CS_Low();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, &rxByte, 1, HAL_MAX_DELAY);
    MAX11200_CS_High();

    return rxByte;
}

// Read a 24-bit value from a specified register
static int32_t MAX11200_ReadReg24(uint8_t regAddr){
    uint8_t cmd = MAX11200_BuildReadCmd(regAddr);
    uint8_t buf[3] = {0};
    int32_t result;

    MAX11200_CS_Low();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, buf, 3, HAL_MAX_DELAY);
    MAX11200_CS_High();

    // Combine 3 bytes into a 24-bit value
    // buf[0] = MSB, buf[1] = middle, buf[2] = LSB
    result = (((int32_t)buf[0] << 16) | ((int32_t)buf[1] << 8) | (buf[2]));

    return result;
}

// Read control and status registers into MAX11200_CtrlStat_Regs
static void MAX11200_ReadCtrlStatRegs()
{
    MAX11200_ctrl_stat_regs.ctrl1 = MAX11200_ReadReg8(MAX11200_CTRL1_REG);
    MAX11200_ctrl_stat_regs.ctrl2 = MAX11200_ReadReg8(MAX11200_CTRL2_REG);
    MAX11200_ctrl_stat_regs.ctrl3 = MAX11200_ReadReg8(MAX11200_CTRL3_REG);
    MAX11200_ctrl_stat_regs.stat1 = MAX11200_ReadReg8(MAX11200_STAT1_REG);
}

/******************
  Public functions
*******************/
// Initialize the low-level driver state and read initial registers.
// Assuming that SPI and GPIO have already been initialized.
void MAX11200_Init(void)
{
    // Make sure CS is high initially
    MAX11200_CS_High();

    // Read control/status reigisters
    MAX11200_read_ctrl_stat_regs();
}

// Set default config struct fields
void MAX11200_Init_Config(MAX11200_Config_Data *config)
{
    if(config == NULL) return;

    config->scycle           = MAX11200_CONFIG_CONVERSION_SINGLE;
    config->format           = MAX11200_CONFIG_FORMAT_OFFSET_BINARY;
    config->sigbuf           = MAX11200_CONFIG_SIGBUF_DISABLE;
    config->refbuf           = MAX11200_CONFIG_REFBUF_DISABLE;
    config->extclk           = MAX11200_CONFIG_CLK_INTERNAL;
    config->unipolar_bipolar = MAX11200_CONFIG_UNIPOLAR;
    config->line_filter      = MAX11200_CONFIG_LINEF_50HZ;
}

// Read the CTRL1 register and populate the config struct
void MAX11200_Read_Config(MAX11200_Config_Data *config)
{
    if(config == NULL) return;

    uint8_t ctrl1 = MAX11200_ReadReg8(MAX11200_CTRL1_REG);

    config->scycle           = ctrl1 & MAX11200_CTRL1_SCYCLE;
    config->format           = ctrl1 & MAX11200_CTRL1_FORMAT;
    config->sigbuf           = ctrl1 & MAX11200_CTRL1_SIGBUF;
    config->refbuf           = ctrl1 & MAX11200_CTRL1_REFBUF;
    config->extclk           = ctrl1 & MAX11200_CTRL1_EXTCLK;
    config->unipolar_bipolar = ctrl1 & MAX11200_CTRL1_UB;
    config->line_filter      = ctrl1 & MAX11200_CTRL1_LINEF;

    MAX11200_CtrlStat_Regs.ctrl1 = ctrl1;
}

// Write config struct fields to CTRL1 register
void MAX11200_Write_Config(MAX11200_Config_Data *config)
{
    uint8_t ctrl1 = config->scycle | config->format | config->sigbuf | config->refbuf |
            config->extclk | config->unipolar_bipolar | config->line_filter;

    MAX11200_WriteReg8(MAX11200_CTRL1_REG, ctrl1);

    MAX11200_CtrlStat_Regs.ctrl1 = ctrl1;
}

// Read the status register (STAT1)
uint8_t MAX11200_Read_Stat()
{
    uint8_t regval = MAX11200_ReadReg8(MAX11200_STAT1_REG);
    MAX11200_CtrlStat_Regs.stat1 = regval;
    return regval;
}

// Check if conversion is complete (RDY=1)
int32_t MAX11200_Conversion_Ready()
{
    uint8_t stat = MAX11200_Read_Stat();
    return (stat & MAX11200_STAT1_RDY);
}

// Check if measurement is in progress (MSTAT=1)
int32_t MAX11200_Measure_In_Progress()
{
    uint8_t stat = MAX11200_Read_Stat();
    return (stat & MAX11200_STAT1_MSTAT);
} 

// Perform a single-cycle conversion at the specified rate.
uint32_t MAX11200_Convert(uint8_t RATE)
{
    MAX11200_Start_Conversion(RATE);
    while(!MAX11200_Conversion_Ready());

    uint32_t val = MAX11200_ReadReg24(MAX11200_DATA_REG);
    return val;
}

// Start a single-cycle conversion by writing the command byte
void MAX11200_Start_Conversion(uint8_t RATE)
{
    // Ensure single-cycle mode is set
    if((MAX11200_ctrl_stat_regs.ctrl1 & MAX11200_CTRL1_SCYCLE) == 0) {
        uint8_t ctrl1 = MAX11200_ReadReg8(MAX11200_CTRL1_REG);
        ctrl1 |= MAX11200_CTRL1_SCYCLE;
        MAX11200_WriteReg8(MAX11200_CTRL1_REG, ctrl1);
        MAX11200_ctrl_stat_regs.ctrl1 = ctrl1;
    }

    // Build a command byte
    uint8_t cmd = (MAX11200_START | MAX11200_MODE0 | RATE);
    MAX11200_CS_Low();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    MAX11200_CS_High();
}

// Oerfirn a sekf0cakubratuib sequence (offset and gain)
void MAX11200_Self_Calibration(uint32_t *calib_offset, uint32_t *calib_gain)
{
    // Enable self calibration registers, disable system calibration
    uint8_t ctrl3 = MAX11200_CTRL3_NOSYSO | MAX11200_CTRL3_NOSYSG;
    MAX11200_WriteReg8(MAX11200_CTRL3_REG, ctrl3);
    MAX11200_ctrl_stat_regs.ctrl3 = ctrl3;

    // Build a command byte to start self calibration
    uint8_t cmd = (MAX11200_START | MAX11200_MODE0 | MAX11200_CMD_CAL0);
    MAX11200_CS_Low();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    MAX11200_CS_High();

    // Self calibration takes ~300 ms, delay for 500
    HAL_Delay(500);

    *calib_offset = MAX11200_ReadReg24(MAX11200_SCOC_REG);
    *calib_gain = MAX11200_ReadReg24(MAX11200_SCGC_REG);
}
