#include<EEPROM.h>

/*
    Ultrasonic sensor Pins:
        VCC: +5VDC
        Trig : Trigger (INPUT) - Pin 9, Pin 11
        Echo: Echo (OUTPUT) - Pin 10, Pin 12
        GND: GND
 */

/***************************************  For DayTime  ***********************************/

enum Days {Mon, Tue, Wed, Thu, Fri, Sat, Sun};

const unsigned int startingHour = 11;                //    starting time in 24 time format
const Days startingDay = Fri;              //    starting day 

const unsigned int weekDay_Day_Interval = 15;      //   intervals in Minutes
const unsigned int weekDay_Night_Interval = 60;
const unsigned int weekEnd_Day_Interval = 30;
const unsigned int weekEnd_Night_Interval = 60;

const unsigned int timeScale = 1;          //   how faster the clock will tick. 1 is normal, 2+ faster

int currentHour = startingHour;
Days currentDay = startingDay;
int currentSecond = 0, currentMinute = 0;
int currentInterval;                     //    intervals in seconds
int countdownTimer = -1;                 //    in seconds, will used to trigger sensors when countdownTimer = 0, then reset it to countdownTimer = currentInterval

bool dayChanged = false;                 //    to put an indicator if day changed in the stored data
bool shiftChanged = false;                 //    to put an indicator if shift changed in the stored data
bool newSegment = false;
bool isWorkingHour = false;

/***************************************  For EEPROM  ***********************************/

struct SensorData       //  0 to 6 are reserved for special numbers
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
//const long interval = 360;         // interval in seconds

int eeAddress = 0;
int sizeOfSensorData = 0;
int totalDataLimit = 0;

struct SensorData sensorData;
struct EEDataIndex eeDataIndex;


/***************************************  For Sensors  ***********************************/

int trigPin_1 = 9;
int echoPin_1 = 10;
int trigPin_2 = 11;
int echoPin_2 = 12;


long duration, distance;       // distance in cm

void setup() {
  // put your setup code here, to run once:

  Serial.begin(9600);
  
  setInterval();  
  initSensors();  
  initEEPROM();
  
  Serial.println("Waiting for sensors to sattle...");

  delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:
  
  //setTime();

  if(countdownTimer <= 0){
    //  sensor will be triggered, data saved...

    if(eeDataIndex._isValid == 1){
       
      int distance_1 = readSensor(trigPin_1, echoPin_1);
      delay(100);
      int distance_2 = readSensor(trigPin_2, echoPin_2);
      
      Serial.print(currentHour); Serial.print(" : "); Serial.print(currentMinute); Serial.print(" : "); Serial.println(currentSecond);
      
      Serial.print("Sensor 1: "); Serial.print(distance_1); Serial.print("\t Sensor 2: "); Serial.println(distance_2);
      
      sensorData = {roundValue(distance_1), roundValue(distance_2)};
      
      if(distance_1>14 && distance_2>14){
        saveData(sensorData);        
      }
      

      //Serial.print("#Sensor 1: "); Serial.print(sensorData.sensor_A); Serial.print("\t Sensor 2: "); Serial.println(sensorData.sensor_B);      
    }
    countdownTimer = currentInterval;
  }
  else if(eeDataIndex._isValid == 0){             //      indicates storage full, on board led blinks
    if(digitalRead(indicatorLedPin) == LOW)
      digitalWrite(indicatorLedPin, HIGH);
    else
      digitalWrite(indicatorLedPin, LOW);
  }


  setTime();
}

/***************************************  For DayTime  ***********************************/

void setTime(){
  if(currentSecond >= 60){
    currentMinute++;
    currentSecond = 0;
    
    if(currentMinute >= 60){
      currentHour++;
      currentMinute = 0;
      
      setInterval();

      if(currentHour >= 24){
        currentDay++;
        currentHour = 0;
        dayChanged = true;
      }
    }
  }
  
  //Serial.print(currentHour); Serial.print(" : "); Serial.print(currentMinute); Serial.print(" : "); Serial.println(currentSecond);
  //Serial.println(countdownTimer);
  
  delay(1000 * 1 / timeScale);
  
  countdownTimer--;
  currentSecond++;
}

void setInterval(){
  
  switch(currentDay){
    case Mon:
    case Tue:
    case Wed:
    case Thu:
      if(currentHour > 7 && currentHour < 19){                  //    working hour
        currentInterval = weekDay_Day_Interval * 60;
        
        if(!isWorkingHour){
          shiftChanged = true;
          isWorkingHour = !isWorkingHour;
        }
      }else{
        currentInterval = weekDay_Night_Interval * 60;

        if(isWorkingHour){
          shiftChanged = true;
          isWorkingHour = !isWorkingHour;
        }
      }
      break;
    case Fri:
      if(currentHour > 7 && currentHour < 19){                  //    working hour
        currentInterval = weekDay_Day_Interval * 60;

        if(!isWorkingHour){
          shiftChanged = true;
          isWorkingHour = !isWorkingHour;
        }
      }else{
        currentInterval = weekEnd_Night_Interval * 60;

        if(isWorkingHour){
          shiftChanged = true;
          isWorkingHour = !isWorkingHour;
        }
      }
      break;
    case Sat:
    case Sun:
      if(currentHour > 7 && currentHour < 19){                  //    working hour
        currentInterval = weekEnd_Day_Interval * 60;

        if(!isWorkingHour){
          shiftChanged = true;
          isWorkingHour = !isWorkingHour;
        }
      }else{
        currentInterval = weekEnd_Night_Interval * 60;

        if(isWorkingHour){
          shiftChanged = true;
          isWorkingHour = !isWorkingHour;
        }
      }
      break;
  }

  
  //    insert new segment for shift change information
  
  newSegment = (dayChanged || shiftChanged);
  if(newSegment){
    /*
     * Day 0 to 6 (as Days enum)
     * Working hr = 1, else 0
     */

    Serial.println("-------- New shift -----------");
    
    SensorData shiftInformation = {(int) currentDay, (int) isWorkingHour};
    saveData(shiftInformation);
    
    newSegment = shiftChanged = dayChanged = false;
  }
  
  countdownTimer = currentInterval;
}

/***************************************  For Sensors  ***********************************/

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

/***************************************  For EEPROM  ***********************************/

void saveData(SensorData _sensorData)
{
  if(eeDataIndex._isValid == 1)
  { 
    eeAddress = eeDataIndex._index * sizeOfSensorData;
    EEPROM.put(eeAddress, _sensorData);

    delay(50);
    EEPROM.get(eeAddress, _sensorData);
    
    
    if(eeDataIndex._index >= totalDataLimit)
    {
      eeDataIndex._isValid = 0;
      Serial.println("Full");
    }

    EEPROM.put(eeIndexAdress, eeDataIndex);

    Serial.print(eeDataIndex._index); Serial.print("--> "); Serial.print(_sensorData.sensor_A); Serial.print(", "); Serial.println(_sensorData.sensor_B);

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

/***************************************  Extra Functions  ***********************************/

int roundValue(int value)
{
  int roundedValue = value/2;
  if(value>255){
    value = 255;
  }
  return roundedValue;
}

inline Days &operator++(Days &d)
{
  d = Days(d + 1);
  if (d>6) d = Days (0);
  return d;
}
