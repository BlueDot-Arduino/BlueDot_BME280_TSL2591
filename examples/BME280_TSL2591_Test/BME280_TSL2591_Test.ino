
/***************************************************************************
  Example for the BME280+TSL2591 Advanced Weather Station
  written by Thiago Barros for BlueDot UG (haftungsbeschränkt)
  BSD License
  
  Version 1.0.4 (2018-04-27)

  This sketch was written for the Bosch Sensor BME280 and the AMS Sensor TSL2591.
  The BME280 is a MEMS device for measuring temperature, humidity and atmospheric pressure, while the TSL2591 is a light sensor.
  For more technical information on the BME280 and on the TSL2591, please go to ------> http://www.bluedot.space

 ***************************************************************************/


#include <Wire.h>
#include "BlueDot_BME280_TSL2591.h"
BlueDot_BME280_TSL2591 bme280;
BlueDot_BME280_TSL2591 tsl2591;


void setup() {
  Serial.begin(9600);
  Serial.println(F("Basic Weather Station"));

 
  bme280.parameter.I2CAddress = 0x77;                 //The BME280 is hardwired to use the I2C Address 0x77              
  tsl2591.parameter.I2CAddress = 0x29;   

  //*********************************************************************
  //*************BASIC SETUP - READ BEFORE GOING ON!*********************
    
  //Choose between Arduino Boards and the NodeMCU board (ESP8266)
  //Arduino boards have predefined SDA and SCL pins for the I2C communication
  //For NodeMCU boards we need to assign two pins for the SDA and SCL lines
  //The Wire.begin() function allows you to define which pins you want for the I2C mode
  //It works like this: Wire.begin([SDA],[SCL])
    
   Wire.begin();                          //Use this function for Arduino boards                                                                   
 
  //Wire.begin(0,2);                      //Use this for NodeMCU boards
                                          //For the NodeMCU V3 board: 
                                          //GPIO0 = Pin D3 = SDA
                                          //GPIO2 = Pin D4 = SCL
  
                                          
     
  //*********************************************************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************        

   //Here we can configure the TSL2591 Light Sensor
   //First we set the gain value
   //Higher gain values are better for dimmer light conditions, but lead to sensor saturation with bright light 
   //We can choose among four gain values:
    
   //0b00:    Low gain mode
   //0b01:    Medium gain mode
   //0b10:    High gain mode
   //0b11:    Maximum gain mode

   tsl2591.parameter.gain = 0b01;

   //Longer integration times also helps in very low light situations, but the measurements are slower
 
   //0b000:   100ms (max count = 37888)
   //0b001:   200ms (max count = 65535)
   //0b010:   300ms (max count = 65535)
   //0b011:   400ms (max count = 65535)
   //0b100:   500ms (max count = 65535)
   //0b101:   600ms (max count = 65535)
    
   tsl2591.parameter.integration = 0b000;    

   //The values for the gain and integration times are written transfered to the sensor through the function config_TSL2591
   //This function powers the device ON, then configures the sensor and finally powers the device OFF again 
       
   tsl2591.config_TSL2591();  

   
  
  //*********************************************************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
  
  //Now choose on which mode your device will run
  //On doubt, just leave on normal mode, that's the default value

  //0b00:     In sleep mode no measurements are performed, but power consumption is at a minimum
  //0b01:     In forced mode a single measured is performed and the device returns automatically to sleep mode
  //0b11:     In normal mode the sensor measures continually (default value)
  
    bme280.parameter.sensorMode = 0b11;                   //Choose sensor mode

    

  //*********************** TSL2591 *************************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
  
  //Great! Now set up the internal IIR Filter
  //The IIR (Infinite Impulse Response) filter suppresses high frequency fluctuations
  //In short, a high factor value means less noise, but measurements are also less responsive
  //You can play with these values and check the results!
  //In doubt just leave on default

  //0b000:      factor 0 (filter off)
  //0b001:      factor 2
  //0b010:      factor 4
  //0b011:      factor 8
  //0b100:      factor 16 (default value)

    bme280.parameter.IIRfilter = 0b100;                    //Setup for IIR Filter

    

  //************************** BME280 ***********************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
  
  //Next you'll define the oversampling factor for the humidity measurements
  //Again, higher values mean less noise, but slower responses
  //If you don't want to measure humidity, set the oversampling to zero

  //0b000:      factor 0 (Disable humidity measurement)
  //0b001:      factor 1
  //0b010:      factor 2
  //0b011:      factor 4
  //0b100:      factor 8
  //0b101:      factor 16 (default value)
  
    bme280.parameter.humidOversampling = 0b101;            //Setup Humidity Oversampling

    

  //************************** BME280 ***********************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
  
  //Now define the oversampling factor for the temperature measurements
  //You know now, higher values lead to less noise but slower measurements
  
  //0b000:      factor 0 (Disable temperature measurement)
  //0b001:      factor 1
  //0b010:      factor 2
  //0b011:      factor 4
  //0b100:      factor 8
  //0b101:      factor 16 (default value)

    bme280.parameter.tempOversampling = 0b101;             //Setup Temperature Ovesampling

    

  //************************** BME280 ***********************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
  
  //Finally, define the oversampling factor for the pressure measurements
  //For altitude measurements a higher factor provides more stable values
  //On doubt, just leave it on default
  
  //0b000:      factor 0 (Disable pressure measurement)
  //0b001:      factor 1
  //0b010:      factor 2
  //0b011:      factor 4
  //0b100:      factor 8
  //0b101:      factor 16 (default value)
  
    bme280.parameter.pressOversampling = 0b101;            //Setup Pressure Oversampling 

    
  
  //************************** BME280 ***********************************
  //*************ADVANCED SETUP - SAFE TO IGNORE!************************
  
  //For precise altitude measurements please put in the current pressure corrected for the sea level
  //On doubt, just leave the standard pressure as default (1013.25 hPa)
  
    bme280.parameter.pressureSeaLevel = 1013.25;           //default value of 1013.25 hPa

  //Now write here the current average temperature outside (yes, the outside temperature!)
  //You can either use the value in Celsius or in Fahrenheit, but only one of them (comment out the other value)
  //In order to calculate the altitude, this temperature is converted by the library into Kelvin
  //For slightly less precise altitude measurements, just leave the standard temperature as default (15°C)
  //Remember, leave one of the values here commented, and change the other one!
  //If both values are left commented, the default temperature of 15°C will be used
  //But if both values are left uncommented, then the value in Celsius will be used    
  
    bme280.parameter.tempOutsideCelsius = 15;              //default value of 15°C
  //bme280.parameter.tempOutsideFahrenheit = 59;           //default value of 59°F

  
  //*********************************************************************
  //*************ADVANCED SETUP IS OVER - LET'S CHECK THE CHIP ID!*******

  if (bme280.init_BME280() != 0x60)  
  {        
    Serial.println(F("Ops! BME280 could not be found!"));
    //while(1);
  }
  else
  {
    Serial.println(F("BME280 detected!"));
  }

  if (tsl2591.init_TSL2591() != 0x50)  
  {        
    Serial.println(F("Ops! TSL2591 could not be found!"));
    //while(1);
  }
  else
  {
    Serial.println(F("TSL2591 detected!"));
  }
  
  Serial.println();
  Serial.println(); 
}
  //*********************************************************************
  //*************NOW LET'S START MEASURING*******************************
void loop() 
{ 
     
   Serial.print(F("Duration in Seconds:\t\t"));
   Serial.println(float(millis())/1000);
 
   Serial.print(F("Temperature in Celsius:\t\t")); 
   Serial.println(bme280.readTempC());
   
   Serial.print(F("Humidity in %:\t\t\t")); 
   Serial.println(bme280.readHumidity());

   Serial.print(F("Pressure in hPa:\t\t")); 
   Serial.println(bme280.readPressure());

   Serial.print(F("Altitude in Meters:\t\t")); 
   Serial.println(bme280.readAltitudeMeter());

   Serial.print(F("Illuminance in Lux:\t\t")); 
   Serial.println(tsl2591.readIlluminance_TSL2591());

   Serial.println();
   Serial.println();
   

   delay(1000);   
 
}
