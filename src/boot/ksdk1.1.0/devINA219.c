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
writeSensorRegisterINA219(uint8_t deviceRegister, uint16_t payload)
{
	uint8_t		payloadByte[2], commandByte[1];
	i2c_status_t	status;

	i2c_device_t slave =
	{
		.address = deviceINA219State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	warpScaleSupplyVoltage(deviceINA219State.operatingVoltageMillivolts);
	commandByte[0] = deviceRegister;
	payloadByte[1] = (payload & 0x00FF);
  *(payloadByte) = ((payload >> 8) & 0x00FF);
	warpEnableI2Cpins();

	status = I2C_DRV_MasterSendDataBlocking(
							0 /* I2C instance */,
							&slave,
							commandByte,
							1,
							(uint8_t *)payloadByte,
							2,
							gWarpI2cTimeoutMilliseconds);
	if (status != kStatus_I2C_Success)
	{
		return kWarpStatusDeviceCommunicationFailed;
	}

	return kWarpStatusOK;
}

WarpStatus
configureSensorINA219()
{
	WarpStatus	i2cWriteStatus1,i2cWriteStatus2;
  uint32_t configuration_value = 0x019F;//0x015F | 0x399F https://github.com/adafruit/Adafruit_INA219/blob/master/ -> 12-bit bus resolution, 1x12-bit shunt sample, shunt and bus voltage continuous
  uint32_t calibration_value = 0xFFFF; //0xFFFF | 0x2000 /* Current LSB = 100uA, VBusMax = 16V, VShuntMax = 0.16, RShunt = 0.1 ohms. Calibration = trunc (0.04096 / (Current_LSB * RSHUNT)) = 40960d */

	warpScaleSupplyVoltage(deviceINA219State.operatingVoltageMillivolts);
 
 i2cWriteStatus1 = writeSensorRegisterINA219(0x00 /* register address CONFIGURATION */,
               configuration_value
							);  

	i2cWriteStatus2 = writeSensorRegisterINA219(0x05 /* register address CALIBRATION */,
               calibration_value /* Calculated from equations 1 and 2 in INA219 datasheet page 8*/
							);

	return (i2cWriteStatus1|i2cWriteStatus2);
}

WarpStatus
readSensorRegisterINA219(uint8_t deviceRegister, int numberOfBytes)
{
	uint8_t		cmdBuf[1] = {0xFF};  
	i2c_status_t	status;

	USED(numberOfBytes);

	i2c_device_t slave =
	{
		.address = deviceINA219State.i2cAddress,
		.baudRate_kbps = gWarpI2cBaudRateKbps
	};

	warpScaleSupplyVoltage(deviceINA219State.operatingVoltageMillivolts);
  cmdBuf[0] = deviceRegister;
 
	warpEnableI2Cpins();

	status = I2C_DRV_MasterReceiveDataBlocking(
							0 /* I2C peripheral instance */,
							&slave,
							cmdBuf,
							1,
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
	uint16_t	BusVoltageMSB, counter = 0, ShuntVoltageMSB, CurrentMSB, PowerMSB, BusVoltageLSB, ShuntVoltageLSB, CurrentLSB, PowerLSB;
	int16_t		correctBusVoltageMeasurement, correctShuntVoltageMeasurement, correctCurrentMeasurement, correctPowerMeasurement;
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
   
   // Perform calibration
   configureSensorINA219();
   	
   while(counter<itr){
   
     // Read MSB and LSB information from Bus Voltage (2 bytes first MSB and second LSB -> page 15 datasheet)
     readSensorRegisterINA219(0x02,2);
     BusVoltageMSB = deviceINA219State.i2cBuffer[0];
     BusVoltageLSB = deviceINA219State.i2cBuffer[1];
     // Convert bytes to voltage values by shifting the bits of the register 3 bits to the right, combining MSB and LSB and multiplying by 4mV (LSB)
     correctBusVoltageMeasurement = ((BusVoltageMSB & 0xFF) << 5) | ((BusVoltageLSB & 0xF8) >> 3); // 0xFF -> 11111111 and 0xF8 -> 11111000
     // Print result
     warpPrint("\nBus Voltage Measurement %d: %d, Bus Voltage Value: %dmV",counter+1,correctBusVoltageMeasurement,correctBusVoltageMeasurement*4);
     
     // Read MSB and LSB information from Shunt Voltage (2 bytes first MSB and second LSB -> page 15 datasheet)
     readSensorRegisterINA219(0x01,2);
     ShuntVoltageMSB = deviceINA219State.i2cBuffer[0];
     ShuntVoltageLSB = deviceINA219State.i2cBuffer[1];
     // Convert bytes to voltage values by shifting combining MSB and LSB and multiplying by 10uV (LSB)
     correctShuntVoltageMeasurement = ((ShuntVoltageMSB & 0xFF) << 8) | ((ShuntVoltageLSB & 0xFF)); // 0xFF -> 11111111
     // Print result
     warpPrint("\nShunt Voltage Measurement %d: %d, Shunt Voltage Conversion: %duV",counter+1,correctShuntVoltageMeasurement,correctShuntVoltageMeasurement*10);
     
     // Read MSB and LSB information from Current (2 bytes first MSB and second LSB -> page 15 datasheet)
     readSensorRegisterINA219(0x04,2);
     CurrentMSB = deviceINA219State.i2cBuffer[0];
     CurrentLSB = deviceINA219State.i2cBuffer[1];
     // Convert bytes to current values by combining MSB and LSB and multiplying by 4mV (LSB)
     correctCurrentMeasurement = ((CurrentMSB & 0xFF) << 8) | ((CurrentLSB & 0xFF)); // 0xFF -> 11111111
     // Print result
     warpPrint("\nCurrent Measurement %d: %d, Current Value: %duA",counter+1,correctCurrentMeasurement,correctCurrentMeasurement*625/100);
     //warpPrint("\n%d",correctCurrentMeasurement*625/100);
     
     // Read MSB and LSB information from Power (2 bytes first MSB and second LSB -> page 15 datasheet)
     readSensorRegisterINA219(0x03,2);
     PowerMSB = deviceINA219State.i2cBuffer[0];
     PowerLSB = deviceINA219State.i2cBuffer[1];
     // Convert bytes to power values by combining MSB and LSB and multiplying by 20mW (LSB)
     correctPowerMeasurement = ((PowerMSB & 0xFF) << 8) | ((PowerLSB & 0xFF)); // 0xFF -> 11111111
     // Print result
     warpPrint("\nPower Measurement %d: %d, Power Value: %duW\n\n",counter+1,correctPowerMeasurement,correctPowerMeasurement*625/100*20);
     counter = counter + 1; // increase counter
   }
}