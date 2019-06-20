/*
    Ultrasonic sensor Pins:
        VCC: +5VDC
        Trig : Trigger (INPUT) - Pin 9, Pin 11
        Echo: Echo (OUTPUT) - Pin 10, Pin 12
        GND: GND
 */



int trigPin_1 = 9;
int echoPin_1 = 10;
int trigPin_2 = 11;
int echoPin_2 = 12;

int ledPin = 13;

long duration, distance, interval = 1;       // distance in cm, interval in seconds

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
  pinMode(trigPin_1, OUTPUT);
  pinMode(echoPin_1, INPUT);
  
  pinMode(trigPin_2, OUTPUT);
  pinMode(echoPin_2, INPUT);
  pinMode(ledPin, OUTPUT);

  Serial.println("Waiting sensor to sattle...");
  delay(3000);
}

void loop() {
  // put your main code here, to run repeatedly:

  int distance_1 = readSensor(trigPin_1, echoPin_1);
  delay(100);
  int distance_2 = readSensor(trigPin_2, echoPin_2);

  Serial.print("Sensor 1: "); Serial.print(distance_1); Serial.print("\t Sensor 2: "); Serial.println(distance_2);
  
  delay(interval * 1000);
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
