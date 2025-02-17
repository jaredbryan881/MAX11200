#include "MAX11200.h"
#include "stm32wlxx_hal.h"  // Or whatever MCU you're using, e.g. stm32f4xx_hal.h

extern SPI_HandleTypeDef hspi1; // or pass it as a parameter

#define MAX11200_CS_GPIO_PORT  GPIOA
#define MAX11200_CS_PIN        GPIO_PIN_4

static inline void MAX11200_CS_Low(void){
    HAL_GPIO_WritePin(MAX11200_CS_GPIO_PORT, MAX11200_CS_PIN, GPIO_PIN_RESET);
}

static inline void MAX11200_CS_High(void){
    HAL_GPIO_WritePin(MAX11200_CS_GPIO_PORT, MAX11200_CS_PIN, GPIO_PIN_SET);
}

static inline uint8_t MAX11200_BuildReadCmd(uint8_t regAddr){
    return (MAX11200_START | MAX11200_MODE1 | MAX11200_READ | (regAddr << 1));
}

static inline uint8_t MAX11200_BuildWriteCmd(uint8_t regAddr){
    return (MAX11200_START | MAX11200_MODE1 | (regAddr << 1));
}

static void MAX11200_WriteReg8(uint8_t regAddr, uint8_t data){
    uint8_t cmd = MAX11200_BuildWriteCmd(regAddr);

    MAX11200_CS_Low();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Transmit(&hspi1, &data, 1, HAL_MAX_DELAY);
    MAX11200_CS_High();
}

static uint8_t MAX11200_ReadReg8(uint8_t regAddr){
    uint8_t cmd = MAX11200_BuildReadCmd(regAddr);
    uint8_t rxByte = 0x00;

    MAX11200_CS_Low();
    HAL_SPI_Transmit(&hspi1, &cmd, 1, HAL_MAX_DELAY);
    HAL_SPI_Receive(&hspi1, &rxByte, 1, HAL_MAX_DELAY);
    MAX11200_CS_High();

    return rxByte;
}

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

int32_t MAX11200_ReadData24(void){
    return MAX11200_ReadReg24(MAX11200_DATA_REG);
}

void MAX11200_Init(void)
{
    /**********************************
      Read status and control registers 
     **********************************/
    //uint8_t stat = MAX11200_ReadReg8(MAX11200_STAT_REG);
    //uint8_t ctrl1 = MAX11200_ReadReg8(MAX11200_CTRL1_REG);
    //uint8_t ctrl2 = MAX11200_ReadReg8(MAX11200_CTRL2_REG);
    //uint8_t ctrl3 = MAX11200_ReadReg8(MAX11200_CTRL3_REG);

    /*************************
      Configure CTRL1 register
     *************************/
    uint8_t ctrl1 = 0;
    ctrl1 |= MAX11200_CONFIG_CONVERSION_CONTINUOUS | MAX11200_CONFIG_UNIPOLAR | MAX11200_CONFIG_REFBUF_DISABLE | MAX11200_CONFIG_SIGBUF_DISABLE;

    MAX11200_WriteReg8(MAX11200_CTRL1_REG, ctrl1);

}