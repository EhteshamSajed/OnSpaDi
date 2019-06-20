#include<EEPROM.h>

struct SensorData
{
  unsigned int sensor_A:8;
  unsigned int sensor_B:8;
};
struct EEDataIndex
{
  int _isValid:2;       // -1 = uninitialized/invalid, +1 = valid, 0 = full
  unsigned int _index:9;  
};

const int eeIndexAdress  = 0;

int eeAddress = 0, maxEeAddress = 0;
int sizeOfSensorData = 0;
int totalDataLimit = 0;

struct SensorData sensorData;
struct EEDataIndex eeDataIndex;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);  
  
  sizeOfSensorData = sizeof(sensorData);
  
  totalDataLimit = (EEPROM.length()-sizeof(eeDataIndex))/sizeOfSensorData - 1;

  EEPROM.get(eeIndexAdress, eeDataIndex);    

  maxEeAddress = eeDataIndex._index * 2;
  eeAddress = sizeof(eeDataIndex);

  Serial.println(eeDataIndex._isValid);
  Serial.println(eeDataIndex._index);

}

void loop() {
  // put your main code here, to run repeatedly:
  if(eeAddress <= maxEeAddress)
  {
    delay(50);
    
    EEPROM.get(eeAddress, sensorData);
    Serial.print(eeAddress/2); Serial.print(": "); Serial.print(sensorData.sensor_A); Serial.print(", "); Serial.println(sensorData.sensor_B);
    
    eeAddress += sizeOfSensorData;  
  }
}
