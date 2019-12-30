#include <Wire.h>
#include <SPI.h>
#include <SD.h>
#include <TroykaRTC.h>
#include <TroykaMeteoSensor.h>
#include <TroykaMQ.h>

#define PIN_MQ9         A0
#define PIN_MQ9_HEATER  13
#define SD_CS_PIN       8

RTC clock;
TroykaMeteoSensor meteoSensor;
MQ9 mq9(PIN_MQ9, PIN_MQ9_HEATER);

String dataString;

void setup() {
  Serial.begin(9600);
  while(!Serial) {
  }
  Serial.println("Serial init OK");
  
  clock.begin();
  // clock.set(10,25,45,27,07,2005,THURSDAY);    
  // clock.set(__TIMESTAMP__);
  
  Serial.println("Initializing SD card...");
  if (!SD.begin(SD_CS_PIN)) {
    Serial.println("Card failed, or not present");
    return;
  } else {
    Serial.println("Card initialized.");
  }

  meteoSensor.begin();
  Serial.println("Meteo Sensor init OK");
  
  delay(1000);

  mq9.cycleHeat();
}

void loop() {

  if (!mq9.isCalibrated() && mq9.atHeatCycleEnd()) {
    mq9.calibrate();
    Serial.print("Ro = ");
    Serial.println(mq9.getRo());
    mq9.cycleHeat();
  }

  if (mq9.isCalibrated() && mq9.atHeatCycleEnd()) {  
    clock.read();
   
    long Utime = clock.getUnixTime();
    String ftime, date, weekDay; 
  
    clock.getTimeStamp(ftime, date, weekDay);
  
    Serial.println(Utime);
    Serial.println(String(ftime + "\t" + date + "\t" + weekDay));

    int stateSensor = meteoSensor.read();
    switch (stateSensor) {
      case SHT_OK:

        Serial.print("Ratio: ");
        Serial.print(mq9.readRatio());
        Serial.print(" LPG: ");
        Serial.print(mq9.readLPG());
        Serial.print(" ppm ");
        Serial.print(" Methane: ");
        Serial.print(mq9.readMethane());
        Serial.print(" ppm ");
        Serial.print(" CarbonMonoxide: ");
        Serial.print(mq9.readCarbonMonoxide());
        Serial.println(" ppm ");
      
        Serial.println("Data sensor is OK");
        Serial.print("Temperature = ");
        Serial.print(meteoSensor.getTemperatureC());
        Serial.println(" C \t");
        Serial.print("Humidity = ");
        Serial.print(meteoSensor.getHumidity());
        Serial.println(" %\r\n");

        dataString += String(Utime) + ",";
        dataString += String(meteoSensor.getTemperatureC()) + ",";
        dataString += String(meteoSensor.getHumidity()) + ",";
        dataString += String(mq9.readLPG()) + ",";
        dataString += String(mq9.readMethane()) + ",";
        dataString += String(mq9.readCarbonMonoxide());
        saveSD(String(dataString));
        dataString = "";
        
        break;
      case SHT_ERROR_DATA:
        Serial.println("Data error or sensor not connected");
        break;
      case SHT_ERROR_CHECKSUM:
        Serial.println("Checksum error");
        break;
    }
    delay(5000);
    mq9.cycleHeat();
  }
}

void saveSD(String data) {
  File dataFile = SD.open("datalog.csv", FILE_WRITE);
  if (dataFile) {
    dataFile.println(data);
    dataFile.close();
    Serial.println("Save OK");
  } else {
    Serial.println("Error opening datalog.txt");
  }
}
