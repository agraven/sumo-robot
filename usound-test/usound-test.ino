const int trigger = 7;
const int echo = 8;

long duration;
int distance;

void setup() {
  pinMode(trigger, OUTPUT);
  pinMode(echo, INPUT);
  Serial.begin(9600);
}

void loop() {
  // Clear trigger pin
  digitalWrite(trigger, LOW);
  delayMicroseconds(2);
  
  digitalWrite(trigger, HIGH);
  delayMicroseconds(10);
  digitalWrite(trigger, LOW);
  
  duration = pulseIn(echo, HIGH);
  // In centimeters
  distance = duration * 0.034 / 2;
  
  Serial.print(" ");
  Serial.println(distance);
}
