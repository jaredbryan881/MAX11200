# MAX11200
A basic driver for the MAX11200 ADC. It is designed for use with STM32 microcontrollers using the STM32 HAl and an SPI interface.

## Features
- Low-level register access. Functions to read/write 8-bit control registers and 24-bit data registers over SPI.
- Simple functions to initialize the ADC and read/write its operational modes.
- Single-cycle or continuous conversion.
- Self-calibration.
- Tracks control/status registers values.

## Installation
1. Clone or download the repository
2. Include the following files in your project:
  - `MAX11200.c`
  - `MAX11200.h`
3. Add the `#include MAX11200.h` directive where it's needed.

## Usage
Below are examples illustrating how to call each main driver function. For brevity, we'll assume that HAL_Init() and SPI initialization have already been done.

**Initialization**
```c
// Initialize the driver state (ensure CS pin is high to begin)
// Read the control and status registers to populate the config struct
MAX11200_Init();
```

We can now configure the ADC by populating a config struct with our settings.
```c
// Initialize a config struct with default settings
MAX11200_Config_Data config;
MAX11200_Init_Config(&config);
// Modify fields for our use
config.scycle = MAX11200_CONFIG_CONVERSION_SINGLE;
config.format = MAX11200_CONFIG_FORMAT_OFFSET_BINARY;
config.sigbuf = MAX11200_CONFIG_SIGBUF_DISABLE;
config.refbuf = MAX11200_CONFIG_REFBUF_DISABLE;
config.extclk = MAX11200_CONFIG_CLK_INTERNAL;
config.unipolar_bipolar = MAX11200_CONFIG_UNIPOLAR;
config.line_filter = MAX11200_CONFIG_LINEF_60HZ;
```

We can write the config to the ADC and read it again to make sure it changed.
```c
//Write the modified config to CTRL1 register
MAX11200_Write_Config(&config);

// Read back the CTRL1 register settings
MAX11200_Read_Config(&config);
```

**Conversion**
We can perform a single-cycle conversion
```c
// This is an example of a blocking conversion
uint32_t data = MAX11200_Convert(MAX11200_SCYCLE_RATE_120SPS);
```
We could also perform a non-blocking conversion and check for conversion readiness ourselves.
```c
//Start single-cycle conversion, non-blocking
MAX11200_Start_Conversion(MAX11200_SCYCLE_RATE_120SPS);

// We can then poll for completion
while (!MAX11200_Conversion_Ready()){
	// e.g. HAL_Delay(1);
}
```
Of course, you can also do this manually by reading the relevant bit in the STAT1 register.
```c
// Read the STAT1 register to check DRDY, overrange, underrange, etc.
uint8_t status = MAX11200_Read_Stat();
// For example, check if data is ready:
bool isDataReady = (status & MAX11200_STAT1_RDY) != 0;
```

Once the conversion is ready, we can read the data.
```c
uint32_t data = MAX11200_ReadReg24(MAX11200_DATA_REG);
```

Also of interest is MAX11200's self-calibration function.
```c
// Built-in offset and gain self-calibration
uint32_t offset, gain;
MAX11200_Self_Calibration(&offset, &gain);
```

## Limitations
- The driver uses a hard-coded SPI handle, so multiple ADCs or different SPI peripherals will require modification to accept an SPI handle as an argument.
- Blocking SPI transfers.
- Minimal error handling/reporting.
- Haven't tested much on continuous mode.
- No advanced features like GPIO usage on the MAX11200 or complicated calibration procedures.

## Next steps
- Add non-blocking/DMA support. DMA or interrupt-based SPI transfers would be much better for higher sample rates and to reduce CPU load.
- Improve error handling.
- Support multiple instances to help manage multiple MAX11200 devices.
- RTOS integration.

## Licence
This library is provided under the MIT licence.

## Acknowledgements
- Another implementation of a MAX11200 driver (https://github.com/andreanobile/max11200)
- MAX11200 datasheet (https://www.analog.com/media/en/technical-documentation/data-sheets/max11200-max11210.pdf)