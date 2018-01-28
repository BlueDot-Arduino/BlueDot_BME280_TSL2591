




#if defined(_AVR_)
#include <util/delay.h>
#endif

#include "BlueDot_BME280_TSL2591.h"
#include "Wire.h"

BlueDot_BME280_TSL2591::BlueDot_BME280_TSL2591()
{
	parameter.communication;
	parameter.I2CAddress;
	parameter.sensorMode;
	parameter.IIRfilter;
	parameter.tempOversampling;
	parameter.pressOversampling;
	parameter.humidOversampling;
	parameter.pressureSeaLevel = 0;
	parameter.tempOutsideCelsius = 999;
	parameter.tempOutsideFahrenheit = 999;
	parameter.integration;
	parameter.gain;

}


uint8_t BlueDot_BME280_TSL2591::init_BME280(void)
{
	//In order to start the BME280 we need to go through the following steps:
	//1. Read Compensation Coefficients
	//2. Set up IIR Filter (Infinite Impulse Response Filter)
	//3. Set up Oversampling Factors and Sensor Run Mode
	//4. Check Communication (ask and verificate chip ID)		
	
	
	//2. Reading Compensation Coefficients (BME280)
	//####################################
	//After a measurement the device gives values for temperature, pressure and humidity
	//These are all uncompensated and uncablibrated values
	//To correct these values we need the calibration coefficients
	//These are stored into the device during production	
	readCoefficients();
	
	
	
	//3. Set up IIR Filter (BME280)
	//####################
	//The BME280 features an internal IIR (Infinite Impulse Response) Filter
	//The IIR Filter suppresses high frequency fluctuations (i. e. pressure changes due to slamming doors)
	//It improves the pressure and temperature resolution to 20 bits
	//The resolution of the humidity measurement is fixed at 16 bits and is not affected by the filter
	//When enabled, we can set up the filter coefficient (2, 4, 8 or 16)
	//This coefficient defines the filter's time constant (please refer to Datasheet)
	writeIIRFilter();	
	
	
	
	//4. Set up Oversampling Factors and Sensor Mode (BME280)
	//##############################################
	//Oversampling heavily influences the noise in the data (please refer to the Datasheet for more Info)
	//The BME280 Datasheet provides settings suggestions for different applications		
	//Finally we write all those values to their respective registers
	writeCTRLMeas();
	
	
	
	//5. Check Communication
	//######################
	//All BME280 devices share the same chip ID: 0x60
	//If we read anything else than 0x60, we interrupt the program
	//In this case, please check the wiring to the device
	//Also check the correct I2C Address (either 0x76 or 0x77)
	
	return checkID_BME280();

}

uint8_t BlueDot_BME280_TSL2591::init_TSL2591(void)
{
	return checkID_TSL2591();
}

//##########################################################################
//SET UP FUNCTIONS - TSL2591
//##########################################################################
uint8_t BlueDot_BME280_TSL2591::checkID_TSL2591(void)
{
	//In order to read the register TSL2591_CHIP_ID we need the readByte function
	//The register address "reg" is an 8-bit address and is composed of three variables
	//CMD (bit 7), TRANSACTION (bits 6 and 5) and ADDRESS (bits 4, 3, 2, 1 and 0)
	//The CMD or command bit must be set to 1 (0b1000000 or 0x80)
	//TRANSACTION is set here to Normal Operation by writing the bits 6 and 5 to 0 and 1 respectively (0b00100000 or 0x20)
	//ADDRESS represents the chip ID address and is defined as 0x12 or 0b00010010	
	//So by using the OR function to add all three values together we get the register "reg"

	//readByte(reg)
	//reg = (CMD | TRANSACTION | ADDRESS)
	//reg = TSL2591_COMMAND_BIT (0x80) | TSL2591_NORMAL_MODE (0x20) | TSL2591_CHIP_ID (0x12)
	
	uint8_t chipID;
	chipID = readByte(TSL2591_COMMAND_BIT | TSL2591_NORMAL_MODE | TSL2591_CHIP_ID);
	return chipID;
	
}
//##########################################################################
void BlueDot_BME280_TSL2591::enable_TSL2591(void)
{
	//The enable_TSL2591 function is used to power the device ON by writting the ENABLE register	
	//The CMD and TRANSACTION values are the same as for the reading the Chip ID 
	//Therefore the Command Bit (CMD) is set to 1 and TRANSACTION is set to Normal Mode (0x20)
	//ADDRESS represents the address of the ENABLE register (0x00)
	
	//writeByte(reg, val)
	//reg = (CMD | TRANSACTION | ADDRESS)
	//reg = TSL2591_COMMAND_BIT (0x80) | TSL2591_NORMAL_MODE (0x20) | TSL2591_ENABLE_ADDR (0x00)
	//val =  enable_AIEN (0x10) | enable_AEN (0x02) | enable_powerON (0x01)
	
	writeByte(TSL2591_COMMAND_BIT | TSL2591_NORMAL_MODE | TSL2591_ENABLE_ADDR, 0x10 | 0x02 | 0x01);
}
//##########################################################################
void BlueDot_BME280_TSL2591::disable_TSL2591(void)
{
	//The disable_TSL2591 is used to power the device OFF	

	//writeByte(reg, val)
	//reg = (CMD | TRANSACTION | ADDRESS)
	//reg = TSL2591_COMMAND_BIT (0x80) | TSL2591_NORMAL_MODE (0x20) | TSL2591_ENABLE_ADDR (0x00)
	//val = enable_powerOFF (0x00)
	
	writeByte(TSL2591_COMMAND_BIT | TSL2591_NORMAL_MODE | TSL2591_ENABLE_ADDR, 0x00);
}
//##########################################################################
void BlueDot_BME280_TSL2591::config_TSL2591(void)
{
	//Here we set the gain and the integration time of the photodiode channels
	//There are four possible gain modes: low, medium, high and maximum
	//The integration time can be set to 100, 200, 300, 400, 500 and 600 ms
	//Longer integration times lead to more light sensitivity, but also to longer measurements	
	
	//First we power the device ON
	enable_TSL2591();	
	
	//Now we set the values for the GAIN and the INTEGRATION TIME
	//writeByte(reg, val)
	//reg = (CMD | TRANSACTION | ADDRESS)
	//reg = TSL2591_COMMAND_BIT (0x80) | TSL2591_NORMAL_MODE (0x20) | TSL2591_CONFIG_ADDR (0x01)
	//val = GAIN | INTEGRATION	
	byte value;	
	value = (parameter.gain << 4) & 0b00110000;
	value |= parameter.integration & 0b00000111;	
	writeByte(TSL2591_COMMAND_BIT | TSL2591_NORMAL_MODE | TSL2591_CONFIG_ADDR, value);
	
	//Finally we power the device OFF
	disable_TSL2591();	
	
}
//##########################################################################
//DATA READOUT FUNCTIONS - TSL2591
//##########################################################################
uint32_t BlueDot_BME280_TSL2591::getFullLuminosity_TSL2591(void)
{
  
  //The first step is powering the device ON 
  enable_TSL2591();

  //Wait x ms for ADC to complete
  for (uint8_t x=0; x<=parameter.integration; x++) 
  {
    delay(120);
  }
    
  //Now we read the ALS Data Register (0x14 - 0x17)
  //Data is stored as two 16-bit values, one for each Photodiode Channel
  //Here we read both values and write them as a single 32-bit value
  uint32_t y;
  y = readByte16(TSL2591_COMMAND_BIT | TSL2591_NORMAL_MODE | TSL2591_C1DATAL_ADDR);
  y <<= 16;
  y |= readByte16(TSL2591_COMMAND_BIT | TSL2591_NORMAL_MODE | TSL2591_C0DATAL_ADDR);


  //Finally we power the device OFF
  disable_TSL2591();

  return y;
}
//##########################################################################
uint16_t BlueDot_BME280_TSL2591::getFullSpectrum_TSL2591(void)
{
  uint32_t x = getFullLuminosity_TSL2591();
  
  return (x & 0xFFFF);
}
//##########################################################################
uint16_t BlueDot_BME280_TSL2591::getInfrared_TSL2591(void)
{
  uint32_t x = getFullLuminosity_TSL2591();
  
  return (x >> 16);
}
//##########################################################################
uint16_t BlueDot_BME280_TSL2591::getVisibleLight_TSL2591(void)
{
  uint32_t x = getFullLuminosity_TSL2591();
  
  return ((x & 0xFFFF) - (x >> 16));
}
//##########################################################################
float BlueDot_BME280_TSL2591::calculateLux_TSL2591(uint16_t ch0, uint16_t ch1)
{
  uint16_t atime, again;
  float    cpl, lux1, lux2, lux;

  //default
  atime = 100.0F;
  again = 25.0F;
  
  switch (parameter.integration)
  {
	case 0b000 :
	atime = 100.0F;
	break;
	
	case 0b001 :
	atime = 200.0F;
	break;
    
	case 0b010 :
	atime = 300.0F;
	break;
	
	case 0b011 :
	atime = 400.0F;
	break;
	
	case 0b100 :
	atime = 500.0F;
	break;
	
	case 0b101 :
	atime = 600.0F;
	break;    
  } 
  
  switch (parameter.gain)
  {
	case 0b00 :
	again = 1.0F;
	break;
	
	case 0b01 :
	again = 25.0F;
	break;
	
	case 0b10 :
	again = 428.0F;
	break;
	
	case 0b11 :
	again = 9876.0F;
	break;
  
  }
  
  
  cpl = (atime * again)/408.0F;
  lux1 = ( (float)ch0 - (2 * (float)ch1) ) / cpl;
  
  return (float)ch0;
}

//##########################################################################
float BlueDot_BME280_TSL2591::readIlluminance_TSL2591(void)
{
  uint32_t lum = getFullLuminosity_TSL2591();
  uint16_t ir, full;  
  float lux;
  
  ir = lum >> 16;
  full = lum & 0xFFFF;
  
  lux = calculateLux_TSL2591(full, ir);
  return lux;

}
//##########################################################################
//SET UP FUNCTIONS - BME280
//##########################################################################
uint8_t BlueDot_BME280_TSL2591::checkID_BME280(void)
{
	uint8_t chipID;
	chipID = readByte(BME280_CHIP_ID);
	return chipID;
	
}
//##########################################################################
void BlueDot_BME280_TSL2591::readCoefficients(void)
{
	bme280_coefficients.dig_T1 = ((uint16_t)(readByte(BME280_DIG_T1_MSB) << 8) + readByte(BME280_DIG_T1_LSB));
	bme280_coefficients.dig_T2 = ((int16_t)(readByte(BME280_DIG_T2_MSB) << 8) + readByte(BME280_DIG_T2_LSB));
	bme280_coefficients.dig_T3 = ((int16_t)(readByte(BME280_DIG_T3_MSB) << 8) + readByte(BME280_DIG_T3_LSB));
	
	bme280_coefficients.dig_P1 = ((uint16_t)(readByte(BME280_DIG_P1_MSB) << 8) + readByte(BME280_DIG_P1_LSB));
	bme280_coefficients.dig_P2 = ((int16_t)(readByte(BME280_DIG_P2_MSB) << 8) + readByte(BME280_DIG_P2_LSB));
	bme280_coefficients.dig_P3 = ((int16_t)(readByte(BME280_DIG_P3_MSB) << 8) + readByte(BME280_DIG_P3_LSB));
	bme280_coefficients.dig_P4 = ((int16_t)(readByte(BME280_DIG_P4_MSB) << 8) + readByte(BME280_DIG_P4_LSB));
	bme280_coefficients.dig_P5 = ((int16_t)(readByte(BME280_DIG_P5_MSB) << 8) + readByte(BME280_DIG_P5_LSB));
	bme280_coefficients.dig_P6 = ((int16_t)(readByte(BME280_DIG_P6_MSB) << 8) + readByte(BME280_DIG_P6_LSB));
	bme280_coefficients.dig_P7 = ((int16_t)(readByte(BME280_DIG_P7_MSB) << 8) + readByte(BME280_DIG_P7_LSB));
	bme280_coefficients.dig_P8 = ((int16_t)(readByte(BME280_DIG_P8_MSB) << 8) + readByte(BME280_DIG_P8_LSB));
	bme280_coefficients.dig_P9 = ((int16_t)(readByte(BME280_DIG_P9_MSB) << 8) + readByte(BME280_DIG_P9_LSB));
	
	bme280_coefficients.dig_H1 = ((uint8_t)(readByte(BME280_DIG_H1)));
	bme280_coefficients.dig_H2 = ((int16_t)(readByte(BME280_DIG_H2_MSB) << 8) + readByte(BME280_DIG_H2_LSB));
	bme280_coefficients.dig_H3 = ((uint8_t)(readByte(BME280_DIG_H3)));
	bme280_coefficients.dig_H4 = ((int16_t)((readByte(BME280_DIG_H4_MSB) << 4) + (readByte(BME280_DIG_H4_LSB) & 0x0F)));
	bme280_coefficients.dig_H5 = ((int16_t)((readByte(BME280_DIG_H5_MSB) << 4) + ((readByte(BME280_DIG_H4_LSB) >> 4 ) & 0x0F)));
	bme280_coefficients.dig_H6 = ((uint8_t)(readByte(BME280_DIG_H6)));
}
//##########################################################################
void BlueDot_BME280_TSL2591::writeIIRFilter(void)
{
	//We set up the IIR Filter through bits 4, 3 and 2 from Config Register (0xF5)]
	//The other bits from this register won't be used in this program and remain 0
	//Please refer to the BME280 Datasheet for more information
	
	byte value;
	value = (parameter.IIRfilter << 2) & 0b00011100;
	writeByte(BME280_CONFIG, value);
}
//##########################################################################
void BlueDot_BME280_TSL2591::writeCTRLMeas(void)
{
	byte value;
	value = parameter.humidOversampling & 0b00000111;
	writeByte(BME280_CTRL_HUM, value);
	
	value = (parameter.tempOversampling << 5) & 0b11100000;
	value |= (parameter.pressOversampling << 2) & 0b00011100;
	value |= parameter.sensorMode & 0b00000011;
	writeByte(BME280_CTRL_MEAS, value);	
}
//##########################################################################
//DATA READOUT FUNCTIONS - BME280
//##########################################################################
float BlueDot_BME280_TSL2591::readPressure(void)
{
	if (parameter.pressOversampling == 0b000)						//disabling the pressure measurement function
	{
		return 0;
	}
	
	else
	{
		readTempC();
		
		int32_t adc_P;
		adc_P = (uint32_t)readByte(BME280_PRESSURE_MSB) << 12;
		adc_P |= (uint32_t)readByte(BME280_PRESSURE_LSB) << 4;
		adc_P |= (readByte(BME280_PRESSURE_XLSB) >> 4 )& 0b00001111;
		
		int64_t var1, var2, P;
		var1 = ((int64_t)t_fine) - 128000;
		var2 = var1 * var1 * (int64_t)bme280_coefficients.dig_P6;
		var2 = var2 + ((var1 * (int64_t)bme280_coefficients.dig_P5)<<17);
		var2 = var2 + (((int64_t)bme280_coefficients.dig_P4)<<35);
		var1 = ((var1 * var1 * (int64_t)bme280_coefficients.dig_P3)>>8) + ((var1 * (int64_t)bme280_coefficients.dig_P2)<<12);
		var1 = (((((int64_t)1)<<47)+var1))*((int64_t)bme280_coefficients.dig_P1)>>33;
		if (var1 == 0)
		{
			return 0; // avoid exception caused by division by zero
		}
		P = 1048576 - adc_P;
		P = (((P << 31) - var2)*3125)/var1;
		var1 = (((int64_t)bme280_coefficients.dig_P9) * (P >> 13) * (P >> 13)) >> 25;
		var2 = (((int64_t)bme280_coefficients.dig_P8) * P) >> 19;
		P = ((P + var1 + var2) >> 8) + (((int64_t)bme280_coefficients.dig_P7)<<4);
		
		P = P >> 8; // /256
		return (float)P/100;
		
	}
}
//##########################################################################
float BlueDot_BME280_TSL2591::convertTempKelvin(void)
{
	//Temperature in Kelvin is needed for the conversion of pressure to altitude	
	//Both tempOutsideCelsius and tempOutsideFahrenheit are set to 999 as default (see .h file)
	//If the user chooses to read temperature in Celsius, tempOutsideFahrenheit remains 999
	//If the user chooses to read temperature in Fahrenheit instead, tempOutsideCelsius remains 999
	//If both values are used, then the temperature in Celsius will be used for the conversion
	//If none of them are used, then the default value of 288.15 will be used (i.e. 273.15 + 15)
		
	float tempOutsideKelvin;	
	
	if (parameter.tempOutsideCelsius != 999 & parameter.tempOutsideFahrenheit == 999 )   
	{
		tempOutsideKelvin = parameter.tempOutsideCelsius;
		tempOutsideKelvin = tempOutsideKelvin + 273.15;
		return tempOutsideKelvin;		
	}
	
	if (parameter.tempOutsideCelsius != 999 & parameter.tempOutsideFahrenheit != 999 )   
	{
		tempOutsideKelvin = parameter.tempOutsideCelsius;
		tempOutsideKelvin = tempOutsideKelvin + 273.15;
		return tempOutsideKelvin;		
	}
	
	if (parameter.tempOutsideFahrenheit != 999 & parameter.tempOutsideCelsius == 999)
	{
		
		tempOutsideKelvin = (parameter.tempOutsideFahrenheit - 32);
		tempOutsideKelvin = tempOutsideKelvin * 5;
		tempOutsideKelvin = tempOutsideKelvin / 9;
		tempOutsideKelvin = tempOutsideKelvin + 273.15;
		return tempOutsideKelvin;	
	}
	
	if (parameter.tempOutsideFahrenheit == 999 & parameter.tempOutsideCelsius == 999)
	{
		tempOutsideKelvin = 273.15 + 15;
		return tempOutsideKelvin; 
	}
	
	tempOutsideKelvin = 273.15 + 15;
	return tempOutsideKelvin;

}
//##########################################################################
float BlueDot_BME280_TSL2591::readAltitudeFeet(void)
{	
	float heightOutput = 0;
	float tempOutsideKelvin = convertTempKelvin();
	
	heightOutput = readPressure();
	heightOutput = (heightOutput/parameter.pressureSeaLevel);
	heightOutput = pow(heightOutput, 0.190284);
	heightOutput = 1 - heightOutput;
	heightOutput = heightOutput * tempOutsideKelvin;
	heightOutput = heightOutput / 0.0065;
	heightOutput = heightOutput / 0.3048;
	return heightOutput;
	
}

//##########################################################################
float BlueDot_BME280_TSL2591::readAltitudeMeter(void)
{
	float heightOutput = 0;
	float tempOutsideKelvin = convertTempKelvin();
	
	heightOutput = readPressure();
	heightOutput = (heightOutput/parameter.pressureSeaLevel);
	heightOutput = pow(heightOutput, 0.190284);
	heightOutput = 1 - heightOutput;	
	heightOutput = heightOutput * tempOutsideKelvin;
	heightOutput = heightOutput / 0.0065;
	return heightOutput;	
	
}

//##########################################################################
float BlueDot_BME280_TSL2591::readHumidity(void)
{
	if (parameter.humidOversampling == 0b000)					//disabling the humitidy measurement function
	{
		return 0;
	}
	
	else
	{
		int32_t adc_H;
		adc_H = (uint32_t)readByte(BME280_HUMIDITY_MSB) << 8;
		adc_H |= (uint32_t)readByte(BME280_HUMIDITY_LSB);
		
		int32_t var1;
		var1 = (t_fine - ((int32_t)76800));
		var1 = (((((adc_H << 14) - (((int32_t)bme280_coefficients.dig_H4) << 20) - (((int32_t)bme280_coefficients.dig_H5) * var1)) +
		((int32_t)16384)) >> 15) * (((((((var1 * ((int32_t)bme280_coefficients.dig_H6)) >> 10) * (((var1 * ((int32_t)bme280_coefficients.dig_H3)) >> 11) + ((int32_t)32768))) >> 10) + ((int32_t)2097152)) *
		((int32_t)bme280_coefficients.dig_H2) + 8192) >> 14));
		var1 = (var1 - (((((var1 >> 15) * (var1 >> 15)) >> 7) * ((int32_t)bme280_coefficients.dig_H1)) >> 4));
		var1 = (var1 < 0 ? 0 : var1);
		var1 = (var1 > 419430400 ? 419430400 : var1);
		float H = (var1>>12);
		H = H /1024.0;
		return H;
	}
}

//##########################################################################
float BlueDot_BME280_TSL2591::readTempC(void)
{
	
	if (parameter.tempOversampling == 0b000)					//disabling the temperature measurement function
	{
		return 0;
	}
	
	else
	{
		int32_t adc_T;
		adc_T = (uint32_t)readByte(BME280_TEMPERATURE_MSB) << 12;
		adc_T |= (uint32_t)readByte(BME280_TEMPERATURE_LSB) << 4;
		adc_T |= (readByte(BME280_TEMPERATURE_XLSB) >> 4 )& 0b00001111;
		
		int64_t var1, var2;
		
		var1 = ((((adc_T>>3) - ((int32_t)bme280_coefficients.dig_T1<<1))) * ((int32_t)bme280_coefficients.dig_T2)) >> 11;
		var2 = (((((adc_T>>4) - ((int32_t)bme280_coefficients.dig_T1)) * ((adc_T>>4) - ((int32_t)bme280_coefficients.dig_T1))) >> 12) *
		((int32_t)bme280_coefficients.dig_T3)) >> 14;
		t_fine = var1 + var2;
		float T = (t_fine * 5 + 128) >> 8;
		T = T / 100;
		return T;
	}
}

//##########################################################################
float BlueDot_BME280_TSL2591::readTempF(void)
{
	if (parameter.tempOversampling == 0b000)				//disabling the temperature measurement function
	{
		return 0;
	}	
	
	else
	{
		int32_t adc_T;
		adc_T = (uint32_t)readByte(BME280_TEMPERATURE_MSB) << 12;
		adc_T |= (uint32_t)readByte(BME280_TEMPERATURE_LSB) << 4;
		adc_T |= (readByte(BME280_TEMPERATURE_XLSB) >> 4 )& 0b00001111;
		
		int64_t var1, var2;
		
		var1 = ((((adc_T>>3) - ((int32_t)bme280_coefficients.dig_T1<<1))) * ((int32_t)bme280_coefficients.dig_T2)) >> 11;
		var2 = (((((adc_T>>4) - ((int32_t)bme280_coefficients.dig_T1)) * ((adc_T>>4) - ((int32_t)bme280_coefficients.dig_T1))) >> 12) *
		((int32_t)bme280_coefficients.dig_T3)) >> 14;
		t_fine = var1 + var2;
		float T = (t_fine * 5 + 128) >> 8;
		T = T / 100;
		T = (T * 1.8) + 32;
		return T;
	}
}

//##########################################################################
//BASIC FUNCTIONS - TSL2591 + BME280
//##########################################################################
void BlueDot_BME280_TSL2591::writeByte(byte reg, byte value)
{	
	
	Wire.beginTransmission(parameter.I2CAddress);
	Wire.write(reg);
	Wire.write(value);
	Wire.endTransmission();
	
}
//##########################################################################
uint8_t BlueDot_BME280_TSL2591::readByte(byte reg)
{
	uint8_t value;
	
	Wire.beginTransmission(parameter.I2CAddress);
	Wire.write(reg);
	Wire.endTransmission();
	Wire.requestFrom(parameter.I2CAddress,1);		
	value = Wire.read();		
	return value;
	
}
//##########################################################################
uint16_t BlueDot_BME280_TSL2591::readByte16(byte reg)
{
	uint16_t value1;
	uint16_t value2;
	
	Wire.beginTransmission(parameter.I2CAddress);
	Wire.write(reg);
	Wire.endTransmission();
	
	Wire.requestFrom(parameter.I2CAddress,2);		
	value2 = Wire.read();	
	value1 = Wire.read();
	
	value1 <<= 8;
	value1 |= value2;
	
	return value1;
	
}