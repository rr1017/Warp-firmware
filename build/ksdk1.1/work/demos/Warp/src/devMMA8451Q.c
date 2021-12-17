/*
	Authored 2016-2018. Phillip Stanley-Marbell. Additional contributors,
	2018-onwards, see git log.

	All rights reserved.

	Redistribution and use in source and binary forms, with or without
	modification, are permitted provided that the following conditions
	are met:

	*	Redistributions of source code must retain the above
		copyright notice, this list of conditions and the following
		disclaimer.

	*	Redistributions in binary form must reproduce the above
		copyright notice, this list of conditions and the following
		disclaimer in the documentation and/or other materials
		provided with the distribution.

	*	Neither the name of the author nor the names of its
		contributors may be used to endorse or promote products
		derived from this software without specific prior written
		permission.

	THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS
	"AS IS" AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT
	LIMITED TO, THE IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS
	FOR A PARTICULAR PURPOSE ARE DISCLAIMED. IN NO EVENT SHALL THE
	COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR ANY DIRECT, INDIRECT,
	INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES (INCLUDING,
	BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES;
	LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
	CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
	LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN
	ANY WAY OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE
	POSSIBILITY OF SUCH DAMAGE.
*/
#include <stdlib.h>
#include <math.h>
//#include <string.h>

/*
 *	config.h needs to come first
 */
#include "config.h"

#include "fsl_misc_utilities.h"
#include "fsl_device_registers.h"
#include "fsl_i2c_master_driver.h"
#include "fsl_spi_master_driver.h"
#include "fsl_rtc_driver.h"
#include "fsl_clock_manager.h"
#include "fsl_power_manager.h"
#include "fsl_mcglite_hal.h"
#include "fsl_port_hal.h"

#include "gpio_pins.h"
#include "SEGGER_RTT.h"
#include "warp.h"


extern volatile WarpI2CDeviceState	deviceMMA8451QState;
extern volatile uint32_t		gWarpI2cBaudRateKbps;
extern volatile uint32_t		gWarpI2cTimeoutMilliseconds;
extern volatile uint32_t		gWarpSupplySettlingDelayMilliseconds;



void
initMMA8451Q(const uint8_t i2cAddress, uint16_t operatingVoltageMillivolts)
{
	deviceMMA8451QState.i2cAddress			= i2cAddress;
	deviceMMA8451QState.operatingVoltageMillivolts	= operatingVoltageMillivolts;

	return;
}

WarpStatus
writeSensorRegisterMMA8451Q(uint8_t deviceRegister, uint8_t payload)
{
	uint8_t		payloadByte[1], commandByte[1];
	i2c_status_t	status;

	switch (deviceRegister)
	{
		case 0x09: case 0x0a: case 0x0e: case 0x0f:
		case 0x11: case 0x12: case 0x13: case 0x14:
		case 0x15: case 0x17: case 0x18: case 0x1d:
		case 0x1f: case 0x20: case 0x21: case 0x23:
		case 0x24: case 0x25: case 0x26: case 0x27:
		case 0x28: case 0x29: case 0x2a: case 0x2b:
		case 0x2c: case 0x2d: case 0x2e: case 0x2f:
		case 0x30: case 0x31:
		{
			/* OK */
			break;
		}
		
		default:
		{
			return kWarpStatusBadDeviceCommand;
		}
	}

	i2c_device_t slave =
	{
		.address = deviceMMA8451QState.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	warpScaleSupplyVoltage(deviceMMA8451QState.operatingVoltageMillivolts);
	commandByte[0] = deviceRegister;
	payloadByte[0] = payload;
	warpEnableI2Cpins();

	status = I2C_DRV_MasterSendDataBlocking(
							0 /* I2C instance */,
							&slave,
							commandByte,
							1,
							payloadByte,
							1,
							gWarpI2cTimeoutMilliseconds);
	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

WarpStatus
configureSensorMMA8451Q(uint8_t payloadF_SETUP, uint8_t payloadCTRL_REG1, uint8_t payloadXYZ_DATA_CFG, uint8_t payloadCTRL_REG2)
{
	WarpStatus	i2cWriteStatus1, i2cWriteStatus2, i2cWriteStatus3, i2cWriteStatus4;


	warpScaleSupplyVoltage(deviceMMA8451QState.operatingVoltageMillivolts);

	i2cWriteStatus1 = writeSensorRegisterMMA8451Q(kWarpSensorConfigurationRegisterMMA8451QF_SETUP /* register address F_SETUP */,
							payloadF_SETUP /* payload: Disable FIFO */
							);

	i2cWriteStatus2 = writeSensorRegisterMMA8451Q(kWarpSensorConfigurationRegisterMMA8451QCTRL_REG1 /* register address CTRL_REG1 */,
							payloadCTRL_REG1 /* payload */
							);
                                  
  i2cWriteStatus3 = writeSensorRegisterMMA8451Q(0x0E /* register address XYZ_DATA_CFG */,
							payloadXYZ_DATA_CFG /* payload */
							);
                                     
  i2cWriteStatus4 = writeSensorRegisterMMA8451Q(0x2B /* register address CTRL_REG2 */,
							payloadCTRL_REG2 /* payload */
							);

	return (i2cWriteStatus1 | i2cWriteStatus2 | i2cWriteStatus3 | i2cWriteStatus4);
}

WarpStatus
readSensorRegisterMMA8451Q(uint8_t deviceRegister, int numberOfBytes)
{
	uint8_t		cmdBuf[1] = {0xFF};
	i2c_status_t	status;


	USED(numberOfBytes);
	switch (deviceRegister)
	{
		case 0x00: case 0x01: case 0x02: case 0x03: 
		case 0x04: case 0x05: case 0x06: case 0x09:
		case 0x0a: case 0x0b: case 0x0c: case 0x0d:
		case 0x0e: case 0x0f: case 0x10: case 0x11:
		case 0x12: case 0x13: case 0x14: case 0x15:
		case 0x16: case 0x17: case 0x18: case 0x1d:
		case 0x1e: case 0x1f: case 0x20: case 0x21:
		case 0x22: case 0x23: case 0x24: case 0x25:
		case 0x26: case 0x27: case 0x28: case 0x29:
		case 0x2a: case 0x2b: case 0x2c: case 0x2d:
		case 0x2e: case 0x2f: case 0x30: case 0x31:
		{
			/* OK */
			break;
		}
		
		default:
		{
			return kWarpStatusBadDeviceCommand;
		}
	}

	i2c_device_t slave =
	{
		.address = deviceMMA8451QState.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	warpScaleSupplyVoltage(deviceMMA8451QState.operatingVoltageMillivolts);
	cmdBuf[0] = deviceRegister;
	warpEnableI2Cpins();

	status = I2C_DRV_MasterReceiveDataBlocking(
							0 /* I2C peripheral instance */,
							&slave,
							cmdBuf,
							1,
							(uint8_t *)deviceMMA8451QState.i2cBuffer,
							numberOfBytes,
							gWarpI2cTimeoutMilliseconds);

	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

void
printSensorDataMMA8451Q(bool hexModeFlag)
{
	uint16_t	readSensorRegisterValueLSB;
	uint16_t	readSensorRegisterValueMSB;
	int16_t		readSensorRegisterValueCombined;
	WarpStatus	i2cReadStatus;


	warpScaleSupplyVoltage(deviceMMA8451QState.operatingVoltageMillivolts);

	/*
	 *	From the MMA8451Q datasheet:
	 *
	 *		"A random read access to the LSB registers is not possible.
	 *		Reading the MSB register and then the LSB register in sequence
	 *		ensures that both bytes (LSB and MSB) belong to the same data
	 *		sample, even if a new data sample arrives between reading the
	 *		MSB and the LSB byte."
	 *
	 *	We therefore do 2-byte read transactions, for each of the registers.
	 *	We could also improve things by doing a 6-byte read transaction.
	 */
	i2cReadStatus = readSensorRegisterMMA8451Q(kWarpSensorOutputRegisterMMA8451QOUT_X_MSB, 2 /* numberOfBytes */);
	readSensorRegisterValueMSB = deviceMMA8451QState.i2cBuffer[0];
	readSensorRegisterValueLSB = deviceMMA8451QState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 6) | (readSensorRegisterValueLSB >> 2);

	/*
	 *	Sign extend the 14-bit value based on knowledge that upper 2 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 13)) - (1 << 13);

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}

	i2cReadStatus = readSensorRegisterMMA8451Q(kWarpSensorOutputRegisterMMA8451QOUT_Y_MSB, 2 /* numberOfBytes */);
	readSensorRegisterValueMSB = deviceMMA8451QState.i2cBuffer[0];
	readSensorRegisterValueLSB = deviceMMA8451QState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 6) | (readSensorRegisterValueLSB >> 2);

	/*
	 *	Sign extend the 14-bit value based on knowledge that upper 2 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 13)) - (1 << 13);

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}

	i2cReadStatus = readSensorRegisterMMA8451Q(kWarpSensorOutputRegisterMMA8451QOUT_Z_MSB, 2 /* numberOfBytes */);
	readSensorRegisterValueMSB = deviceMMA8451QState.i2cBuffer[0];
	readSensorRegisterValueLSB = deviceMMA8451QState.i2cBuffer[1];
	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 6) | (readSensorRegisterValueLSB >> 2);

	/*
	 *	Sign extend the 14-bit value based on knowledge that upper 2 bit are 0:
	 */
	readSensorRegisterValueCombined = (readSensorRegisterValueCombined ^ (1 << 13)) - (1 << 13);

	if (i2cReadStatus != kWarpStatusOK)
	{
		warpPrint(" ----,");
	}
	else
	{
		if (hexModeFlag)
		{
			warpPrint(" 0x%02x 0x%02x,", readSensorRegisterValueMSB, readSensorRegisterValueLSB);
		}
		else
		{
			warpPrint(" %d,", readSensorRegisterValueCombined);
		}
	}
}

// NEW FUNCTION THAT DETECTS FALLS
void
falldetectorMMA8451Q(void)
{
	uint16_t	readSensorRegisterValueLSB = 0;
	uint16_t	readSensorRegisterValueMSB = 0;
	int16_t		readSensorRegisterValueCombined = 0;
  int16_t   accData[3] = {0}, oldaccData[3] = {0};
  uint16_t  diffData[3] = {0};
  uint16_t   avg_data[20] = {0};
  uint16_t   avg=0;
  int i=0,elem=0;
	WarpStatus	i2cReadStatus, check;
 
  // Configure sensor
  check = configureSensorMMA8451Q(0x00,/* Payload: Disable FIFO */0x39,/* Normal read 14-bit, data rate 1.56Hz, active mode */0x11,/* Set full scale to -4g to 3.99g range and high-pass filter fc=0.5Hz*/0x01/*Define "low power and low noise" mode - current 8uA*/);
  
	warpScaleSupplyVoltage(deviceMMA8451QState.operatingVoltageMillivolts);

	/*
	 *	Falls can be detected by measuring the average difference in acceleration
   *  from x, y and z acceleration information.
	 */
   
  while(check == kWarpStatusOK){  // make sure to proceed only if cofiguration is successful 
   
    // X acceleration
  	i2cReadStatus = readSensorRegisterMMA8451Q(kWarpSensorOutputRegisterMMA8451QOUT_X_MSB, 2 /* numberOfBytes */);
  	readSensorRegisterValueMSB = deviceMMA8451QState.i2cBuffer[0];
  	readSensorRegisterValueLSB = deviceMMA8451QState.i2cBuffer[1];
  	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 6) | (readSensorRegisterValueLSB >> 2); // push MSB (16) 6 to the left and LSB (16) 2 to the right, OR them to get 14-bit data (2 most left bits are zero)
  
  	/*
  	 *	Sign extend the 14-bit value based on knowledge that upper 2 bit are 0:
  	 */
  	accData[0] = (readSensorRegisterValueCombined ^ (1 << 13)) - (1 << 13);
  
  
    // Y acceleration
  	i2cReadStatus = readSensorRegisterMMA8451Q(kWarpSensorOutputRegisterMMA8451QOUT_Y_MSB, 2 /* numberOfBytes */);
  	readSensorRegisterValueMSB = deviceMMA8451QState.i2cBuffer[0];
  	readSensorRegisterValueLSB = deviceMMA8451QState.i2cBuffer[1];
  	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 6) | (readSensorRegisterValueLSB >> 2); // push MSB (16) 6 to the left and LSB (16) 2 to the right, OR them to get 14-bit data (2 most left bits are zero)
  
  	/*
  	 *	Sign extend the 14-bit value based on knowledge that upper 2 bit are 0:
  	 */
  	accData[1] = (readSensorRegisterValueCombined ^ (1 << 13)) - (1 << 13);
   
   
    // Z acceleration
  	i2cReadStatus = readSensorRegisterMMA8451Q(kWarpSensorOutputRegisterMMA8451QOUT_Z_MSB, 2 /* numberOfBytes */);
  	readSensorRegisterValueMSB = deviceMMA8451QState.i2cBuffer[0];
  	readSensorRegisterValueLSB = deviceMMA8451QState.i2cBuffer[1];
  	readSensorRegisterValueCombined = ((readSensorRegisterValueMSB & 0xFF) << 6) | (readSensorRegisterValueLSB >> 2); // push MSB (16) 6 to the left and LSB (16) 2 to the right, OR them to get 14-bit data (2 most left bits are zero)
  
  	/*
  	 *	Sign extend the 14-bit value based on knowledge that upper 2 bit are 0:
  	 */
  	accData[2] = (readSensorRegisterValueCombined ^ (1 << 13)) - (1 << 13);
   
    // Calculate difference in acceleration from old and current acceleration measurements and make values strictly positive and unsigned (to increase magnitude value range)
    for (i=0;i<3;i++){
      diffData[i] = (uint16_t)abs(accData[i]-oldaccData[i]);
      oldaccData[i] = accData[i];
    }  
    
    // Store 20 values to perform moving average (smoothing) of jerk magnitude in x, y and z directions
    avg_data[elem] = round(sqrt((diffData[0]*diffData[0]+diffData[1]*diffData[1]+diffData[2]*diffData[2])));
    
    if (elem==19){
      for (i=0;i<20;i++){  // calculate mean
        avg = avg + avg_data[i];
      } 
      avg = round(avg/20);
      warpPrint("%d\n",avg);
      if (avg>2000){ // define threshold 1g -> conversion Delta_a = 3.995g/(2^13-1)*avg
        warpPrint("FALL!\n");
        // Define RED LED pin
        enum{
          pin = GPIO_MAKE_PIN(HW_GPIOB, 10),
        };
        PORT_HAL_SetMuxMode(PORTB_BASE, 10u, kPortMuxAsGpio);
        while(1){  // intermittent light
     	    GPIO_DRV_SetPinOutput(pin);
    	    OSA_TimeDelay(200);
    	    GPIO_DRV_ClearPinOutput(pin);
   	      OSA_TimeDelay(200);
        }
      }
      elem = 0;
      avg = 0;   
//      memset(avg_data, 0, sizeof avg_data); // clear entire array - should not be necessary, values will be overwritten 
    }
    elem = elem+1;
  }
  // Define GREEN LED pin
  enum{
    pin = GPIO_MAKE_PIN(HW_GPIOB, 11),
  };
  PORT_HAL_SetMuxMode(PORTB_BASE, 11u, kPortMuxAsGpio);
  while(1){  // intermittent light
 	    GPIO_DRV_SetPinOutput(pin);
 	    OSA_TimeDelay(200);
 	    GPIO_DRV_ClearPinOutput(pin);
 	    OSA_TimeDelay(200);
  }
}