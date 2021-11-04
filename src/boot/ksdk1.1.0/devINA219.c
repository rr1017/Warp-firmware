/*
	Authored 2021. Ruben Ruiz-Mateos Serrano.
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


extern volatile WarpI2CDeviceState	deviceINA219State;
extern volatile uint32_t		gWarpI2cBaudRateKbps;
extern volatile uint32_t		gWarpI2cTimeoutMilliseconds;
extern volatile uint32_t		gWarpSupplySettlingDelayMilliseconds;



void
initINA219(const uint8_t i2cAddress, uint16_t operatingVoltageMillivolts)
{
	deviceINA219State.i2cAddress			= i2cAddress;
	deviceINA219State.operatingVoltageMillivolts	= operatingVoltageMillivolts;

	return;
}

WarpStatus
writeSensorRegisterINA219(uint8_t deviceRegister, uint8_t payload)
{
	uint8_t		payloadByte[1], commandByte[1];
	i2c_status_t	status;

	i2c_device_t slave =
	{
		.address = deviceINA219State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	warpScaleSupplyVoltage(deviceINA219State.operatingVoltageMillivolts);
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

// NOT SURE WHAT THIS FUNCTION DOES YET

//WarpStatus
//configureSensorINA219(uint8_t payloadF_SETUP, uint8_t payloadCTRL_REG1)
//{
//	WarpStatus	i2cWriteStatus1, i2cWriteStatus2;
//
//
//	warpScaleSupplyVoltage(deviceINA219State.operatingVoltageMillivolts);
//
//	i2cWriteStatus1 = writeSensorRegisterINA219(kWarpSensorConfigurationRegisterINA219F_SETUP /* register address F_SETUP */,
//							payloadF_SETUP /* payload: Disable FIFO */
//							);
//
//	i2cWriteStatus2 = writeSensorRegisterINA219(kWarpSensorConfigurationRegisterINA219CTRL_REG1 /* register address CTRL_REG1 */,
//							payloadCTRL_REG1 /* payload */
//							);
//
//	return (i2cWriteStatus1 | i2cWriteStatus2);
//}

WarpStatus
readSensorRegisterINA219(int numberOfBytes)
{
	uint8_t		cmdBuf = NULL;  
	i2c_status_t	status;

	USED(numberOfBytes);

	i2c_device_t slave =
	{
		.address = deviceINA219State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	warpScaleSupplyVoltage(deviceINA219State.operatingVoltageMillivolts);
 
  PORT_HAL_SetMuxMode(PORTB_BASE, 3u, kPortMuxAlt2);
	PORT_HAL_SetMuxMode(PORTB_BASE, 1u, kPortMuxAlt4);
 
	warpEnableI2Cpins();

	status = I2C_DRV_MasterReceiveDataBlocking(
							0 /* I2C peripheral instance */,
							&slave,
							cmdBuf,  // cmdBuf is a pointer to the commands to be transferred to the sensor, since this is a read operation it is NULL.
							0,  // cmdSize is 0 since it is a read operation and cmdBuf is NULL
							(uint8_t *)deviceINA219State.i2cBuffer,
							numberOfBytes,
							gWarpI2cTimeoutMilliseconds);

	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

void
printSensorDataINA219(int itr)
{
	uint16_t	BusVoltageMSB;
  uint16_t  counter = 0;
  uint16_t	ShuntVoltageMSB;
  uint16_t	CurrentMSB;
  uint16_t	PowerMSB;
  uint16_t	BusVoltageLSB;
  uint16_t	ShuntVoltageLSB;
  uint16_t	CurrentLSB;
  uint16_t	PowerLSB;
	int16_t		correctBusVoltageMeasurement;
  int16_t		correctShuntVoltageMeasurement;
  int16_t		correctCurrentMeasurement;
  int16_t		correctPowerMeasurement;
	WarpStatus	i2cReadStatus;


	warpScaleSupplyVoltage(deviceINA219State.operatingVoltageMillivolts);

	/*
	 *	From the INA219 datasheet:
	 *  Accessing a particular register on the INA219 is accomplished by writing the appropriate value to the register
   *  pointer.
   *
   *  POINTER ADDRESS    REGISTER NAME      POWER-ON RESET    TYPE
   *
   *        00           Configuration    00111001 10011111    R/W
   *        01           Shunt Voltage       Shunt voltage      R
   *        02            Bus Voltage         Bus voltage       R
   *        03               Power        00000000 00000000     R
   *        04              Current       00000000 00000000     R
   *        05            Calibration     00000000 00000000    R/W
   *  
   *  Every write operation to the INA219 requires a value for the register pointer.
	 */
   
   warpPrint("\nBeginning INA219 Bus Voltage, Shunt Voltage, Current and Power measurements...\n");
   	
   while(counter<itr){
     // Pointer address to Bus Voltage
     writeSensorRegisterINA219(0x02,0x00);
     
     // Read MSB and LSB information from Bus Voltage (2 bytes first MSB and second LSB -> page 15 datasheet)
     readSensorRegisterINA219(2);
     BusVoltageMSB = deviceINA219State.i2cBuffer[0];
     BusVoltageLSB = deviceINA219State.i2cBuffer[1];
     // Convert bytes to voltage values by shifting the bits of the register 3 bits to the right, combining MSB and LSB and multiplying by 4mV (LSB)
     correctBusVoltageMeasurement = ((BusVoltageMSB & 0xFF) << 5) | ((BusVoltageLSB & 0xF8) >> 3); // 0xFF -> 11111111 and 0xF8 -> 11111000
     // Print result
     warpPrint("\nBus Voltage Measurement %d: %d, Bus Voltage Value: %dmV",counter+1,correctBusVoltageMeasurement,correctBusVoltageMeasurement*4);
     
     // Pointer address to Shunt Voltage
     writeSensorRegisterINA219(0x01,0x00);
     
     // Read MSB and LSB information from Shunt Voltage (2 bytes first MSB and second LSB -> page 15 datasheet)
     readSensorRegisterINA219(2);
     ShuntVoltageMSB = deviceINA219State.i2cBuffer[0];
     ShuntVoltageLSB = deviceINA219State.i2cBuffer[1];
     // Convert bytes to voltage values by shifting combining MSB and LSB and multiplying by 10uV (LSB)
     correctShuntVoltageMeasurement = ((ShuntVoltageMSB & 0xFF) << 8) | ((ShuntVoltageLSB & 0xFF)); // 0xFF -> 11111111
     // Print result
     warpPrint("\nShunt Voltage Measurement %d: %d, Shunt Voltage Conversion: %duV",counter+1,correctShuntVoltageMeasurement,correctShuntVoltageMeasurement*10);
     // Pointer address to Current
     writeSensorRegisterINA219(0x04,0x00);
     
     // Read MSB and LSB information from Bus Voltage (2 bytes first MSB and second LSB -> page 15 datasheet)
     readSensorRegisterINA219(2);
     CurrentMSB = deviceINA219State.i2cBuffer[0];
     CurrentLSB = deviceINA219State.i2cBuffer[1];
     // Convert bytes to current values by combining MSB and LSB and multiplying by 4mV (LSB)
     correctCurrentMeasurement = ((CurrentMSB & 0xFF) << 8) | ((CurrentLSB & 0xFF)); // 0xFF -> 11111111
     // Print result
     warpPrint("\nCurrent Measurement %d: %d, Current Value: %duA",counter+1,correctBusVoltageMeasurement,correctBusVoltageMeasurement*1000);
     
     // Pointer address to Power
     writeSensorRegisterINA219(0x03,0x00);
     
     // Read MSB and LSB information from Bus Voltage (2 bytes first MSB and second LSB -> page 15 datasheet)
     readSensorRegisterINA219(2);
     PowerMSB = deviceINA219State.i2cBuffer[0];
     PowerLSB = deviceINA219State.i2cBuffer[1];
     // Convert bytes to power values by combining MSB and LSB and multiplying by 20mW (LSB)
     correctPowerMeasurement = ((PowerMSB & 0xFF) << 8) | ((PowerLSB & 0xFF)); // 0xFF -> 11111111
     // Print result
     warpPrint("\nPower Measurement %d: %d, Power Value: %dmW\n\n",counter+1,correctPowerMeasurement,correctPowerMeasurement*20);
     counter = counter + 1; // increase counter
   }
}