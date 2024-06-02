vint potmeterPin = A0;//
int potmeterVal = 0;

void setup() {
  // put your setup code here, to run once:
  Serial.begin(9600);
}

void loop() {
  // put your main code here, to run repeatedly:
    potmeterVal = analogRead(potmeterPin);
    int mappedValue = mapPotmeterValue(potmeterVal);
    Serial.println(mappedValue);
    //delay(100);
}

int mapPotmeterValue(int val) {
  if (val == 0)
    return 0;
  else if (val < 28)
    return 1;
  else if (val < 280)
    return 2;
  else if (val < 586)
    return 3;
  else if (val < 953)
    return 4;
  else
    return 5;
}

