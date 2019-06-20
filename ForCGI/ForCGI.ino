#include<EEPROM.h>

/*
    Ultrasonic sensor Pins:
        VCC: +5VDC
        Trig : Trigger (INPUT) - Pin 9, Pin 11
        Echo: Echo (OUTPUT) - Pin 10, Pin 12
        GND: GND
 */

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

const int indicatorLedPin = 13;
const int eeIndexAdress  = 0;
const long interval = 360;         // interval in seconds

int eeAddress = 0;
int sizeOfSensorData = 0;
int totalDataLimit = 0;

struct SensorData sensorData;
struct EEDataIndex eeDataIndex;


int trigPin_1 = 9;
int echoPin_1 = 10;
int trigPin_2 = 11;
int echoPin_2 = 12;


long duration, distance;       // distance in cm

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  
  initSensors();
  
  initEEPROM();
  
  Serial.println("Waiting sensor to sattle...");

  delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:

  if(eeDataIndex._isValid == 1)
  {    
    int distance_1 = readSensor(trigPin_1, echoPin_1);
    delay(100);
    int distance_2 = readSensor(trigPin_2, echoPin_2);

    Serial.print("Sensor 1: "); Serial.print(distance_1); Serial.print("\t Sensor 2: "); Serial.println(distance_2);

    sensorData = {roundValue(distance_1), roundValue(distance_2)};
    saveData();

    //Serial.print("#Sensor 1: "); Serial.print(sensorData.sensor_A); Serial.print("\t Sensor 2: "); Serial.println(sensorData.sensor_B);
  
    delay(interval * 1000);
  }
  else
  {
    digitalWrite(indicatorLedPin, HIGH);
    delay(1000);
    digitalWrite(indicatorLedPin, LOW);
    delay(1000);
  }
}

long readSensor(int trigPin, int echoPin)
{
  digitalWrite(trigPin, LOW);
  delay(5);
  digitalWrite(trigPin, HIGH);
  delay(10);
  digitalWrite(trigPin, LOW);

  duration = pulseIn(echoPin, HIGH);

  distance = (duration/2) / 29.1;

  return distance;
}

void saveData()
{
  if(eeDataIndex._isValid == 1)
  {
    eeAddress = eeDataIndex._index * sizeOfSensorData;
    EEPROM.put(eeAddress, sensorData);

    delay(50);
    EEPROM.get(eeAddress, sensorData);
    
    
    if(eeDataIndex._index >= totalDataLimit)
    {
      eeDataIndex._isValid = 0;
      Serial.println("Full");
    }

    EEPROM.put(eeIndexAdress, eeDataIndex);

    Serial.print(eeDataIndex._index); Serial.print(": "); Serial.print(sensorData.sensor_A); Serial.print(", "); Serial.println(sensorData.sensor_B);

    eeDataIndex._index++;
    delay(50);
  }
  else
  {
    digitalWrite(indicatorLedPin, HIGH);
    delay(1000);
    digitalWrite(indicatorLedPin, LOW);
    delay(1000);
  }
}

/************************** all inits **************************/

void initSensors()
{
  pinMode(trigPin_1, OUTPUT);
  pinMode(echoPin_1, INPUT);
  
  pinMode(trigPin_2, OUTPUT);
  pinMode(echoPin_2, INPUT);
}

void initEEPROM()
{  
  pinMode(indicatorLedPin, OUTPUT); 
  
  sizeOfSensorData = sizeof(sensorData);

  //eeSize = EEPROM.length();
  
  totalDataLimit = (EEPROM.length()-sizeof(eeDataIndex))/sizeOfSensorData - 1;
  //totalDataLimit = 10;

  EEPROM.get(eeIndexAdress, eeDataIndex);  

  if(eeDataIndex._isValid == -1)
  {
    eeDataIndex._isValid = 1;
    //eeDataIndex._index = sizeof(eeDataIndex);
    eeDataIndex._index = 1;

    EEPROM.put(eeIndexAdress, eeDataIndex);
  }   
  else if(eeDataIndex._isValid == 0)
  {
    Serial.println("EEPROM full !");
  }

  Serial.println(eeDataIndex._isValid);
  Serial.println(eeDataIndex._index);

  digitalWrite(indicatorLedPin, LOW);  
}

int roundValue(int value)
{
  int roundedValue;
  roundedValue = value;
  if(value < 10) roundedValue = 0;
  else if(value>255){
    roundedValue = (int) value/256;
    if(roundedValue > 9) roundedValue = 9;
  }

  return roundedValue;
}
