void setup()   {
  Serial.begin(38400);
  pinMode(0, INPUT_PULLUP);
  pinMode(1, INPUT_PULLUP);
  pinMode(2, INPUT_PULLUP);
  pinMode(3, INPUT_PULLUP);
  pinMode(4, INPUT_PULLUP);
  pinMode(5, INPUT_PULLUP);
  pinMode(12, OUTPUT);
}

void loop()
{
  if (digitalRead(0) == LOW) {
    Serial.println("Pressed 0");
    digitalWrite(12,HIGH);
  } else if (digitalRead(1) == LOW) {
    Serial.println("Pressed 1");
    digitalWrite(12,HIGH);
  } else if (digitalRead(2) == LOW) {
    Serial.println("Pressed 2");
    digitalWrite(12,HIGH);
  } else if (digitalRead(3) == LOW) {
    Serial.println("Pressed 3");
    digitalWrite(12,HIGH);
  } else if (digitalRead(4) == LOW) {
    Serial.println("Pressed 4");
    digitalWrite(12,HIGH);
  } else if (digitalRead(5) == LOW) {
    Serial.println("Pressed 5");
    digitalWrite(12,HIGH);
  }
  else {
    digitalWrite(12,LOW);
  }
  delay(50);
  
}

