#define PIN_ANALOG_IN A0
void setup() {
  Serial.begin(115200);
  pinMode(2,OUTPUT); //blue pin
  pinMode(4,OUTPUT); //green pin
  pinMode(5,OUTPUT); //red pin

}
void loop() {
  int valMin = 18;
  int valMax = 25;
  int adcValue = analogRead(PIN_ANALOG_IN); //read ADC pin
  double voltage = (float)adcValue / 1023.0 * 3.3; // calculate voltage
  double Rt = 10 * voltage / (3.3 - voltage); //calculate resistance value of thermistor
  double tempK = 1 / (1/(273.15 + 25) + log(Rt / 10)/3950.0); //calculate temperature (Kelvin)
  double tempC = tempK - 273.15; //calculate temperature (Celsius)
  if (tempC < valMin) {
    //blue pin on
    digitalWrite(2,HIGH);
    delay(3000);
    digitalWrite(2,LOW);
  }
  if (tempC >= valMin && tempC <= valMax) {
    //green pin on
    digitalWrite(4,HIGH);
    delay(3000);
    digitalWrite(4,LOW);
  }
  if (tempC > valMax) {
    //red pin on
    digitalWrite(5,HIGH);
    delay(3000);
    digitalWrite(5,LOW);
  }
  //print on serial monitor
  Serial.printf("ADC value : %d,\tVoltage : %.2fV, \tTemperature : %.2fC\n", adcValue,
  voltage, tempC);
  delay(1000);
}
