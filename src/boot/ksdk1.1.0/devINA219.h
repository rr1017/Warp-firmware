/*
	Authored 2021. Ruben Ruiz-Mateos Serrano.
*/

void		initINA219(const uint8_t i2cAddress, uint16_t operatingVoltageMillivolts);
WarpStatus	readSensorRegisterINA219(uint8_t deviceRegister, int numberOfBytes);
WarpStatus	writeSensorRegisterINA219(uint8_t deviceRegister,
					uint8_t payloadBtye);
WarpStatus	configureSensorINA219();
void		printSensorDataINA219(int itr);