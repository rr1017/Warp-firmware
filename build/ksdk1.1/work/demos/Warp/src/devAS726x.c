/*
	Authored 2018. Rae Zhao. Additional contributors 2018-onwards,
	see git log.

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
#include "devAS726x.h"


WarpStatus
ledOnAS726x(uint8_t i2cAddress)
{
	i2c_status_t	status1, status2;
	uint8_t		cmdBuf_LEDCTRL[2]	= {kWarpI2C_AS726x_SLAVE_WRITE_REG, 0x87};
	uint8_t		cmdBuf_LEDON[2]		= {kWarpI2C_AS726x_SLAVE_WRITE_REG, 0x1B};
	
	i2c_device_t slave =
	{ 
		.address = i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	warpEnableI2Cpins();

	status1 = I2C_DRV_MasterSendDataBlocking(
							0				/*	I2C peripheral instance					*/,
							&slave				/*	The pointer to the I2C device information structure	*/,
							cmdBuf_LEDCTRL			/*	The pointer to the commands to be transferred		*/,
							2				/*	The length in bytes of the commands to be transferred	*/,
							NULL				/*	The pointer to the data to be transferred		*/,
							0				/*	The length in bytes of the data to be transferred	*/,
							gWarpI2cTimeoutMilliseconds);

	status2 = I2C_DRV_MasterSendDataBlocking(
							0 /* I2C peripheral instance */,
							&slave /* The pointer to the I2C device information structure */,
							cmdBuf_LEDON /* The pointer to the commands to be transferred */,
							2 /* The length in bytes of the commands to be transferred */,
							NULL /* The pointer to the data to be transferred */,
							0 /* The length in bytes of the data to be transferred */,
							gWarpI2cTimeoutMilliseconds);

	if ((status1 != kStatus_I2C_Success) || (status1 != kStatus_I2C_Success))
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

WarpStatus
ledOffAS726x(uint8_t i2cAddress)
{
	i2c_status_t	status1, status2;
	uint8_t		cmdBuf_LEDCTRL[2]	= {kWarpI2C_AS726x_SLAVE_WRITE_REG, 0x87};
	uint8_t		cmdBuf_LEDOFF[2]	= {kWarpI2C_AS726x_SLAVE_WRITE_REG, 0x00};

	i2c_device_t slave =
	{ 
		.address = i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	warpEnableI2Cpins();

	status1 = I2C_DRV_MasterSendDataBlocking(
					0 /* I2C peripheral instance */,
					&slave /* The pointer to the I2C device information structure */,
					cmdBuf_LEDCTRL /* The pointer to the commands to be transferred */,
					2 /* The length in bytes of the commands to be transferred */,
					NULL /* The pointer to the data to be transferred */,
					0 /* The length in bytes of the data to be transferred */,
					gWarpI2cTimeoutMilliseconds);

	status2 = I2C_DRV_MasterSendDataBlocking(
							0 /* I2C peripheral instance */,
							&slave /* The pointer to the I2C device information structure */,
							cmdBuf_LEDOFF /* The pointer to the commands to be transferred */,
							2 /* The length in bytes of the commands to be transferred */,
							NULL /* The pointer to the data to be transferred */,
							0 /* The length in bytes of the data to be transferred */,
							gWarpI2cTimeoutMilliseconds);

	if ((status1 != kStatus_I2C_Success) || (status1 != kStatus_I2C_Success))
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}
