#define HYSTERESIS 2
#define UPDATE_PERIOD 5000
//#define CALIBRATION_FACTOR 1
//#define CALIBRATION_OFFSET 0
#define POT_RANGE_LOW 200//150
#define POT_RANGE_HIGH 200//230
#define IO_COEFF 0.65F
#define RELAY_PIN 13
#define THERMISTOR_PIN A0
#define POTENTIOMETER_PIN A1
#define NUMSAMPLES 5
#define SERIESRESISTOR 10000
#define THERMISTORNOMINAL 10000
#define BCOEFFICIENT 3950
#define TEMPERATURENOMINAL 25

void setup()
{
	Serial.begin(9600);
  pinMode(RELAY_PIN, OUTPUT);
  pinMode(THERMISTOR_PIN, INPUT);
  pinMode(POTENTIOMETER_PIN, INPUT);
 //pinmodes
}

uint16_t read_and_average(uint8_t pin)
{
  uint32_t average = 0;
  for (int i = 0; i < NUMSAMPLES; i++)
  {
    average += analogRead(pin);
    delay(1);
  }
  average /= NUMSAMPLES;
  return average;
}

uint8_t read_measured()
{
  float measured = read_and_average(THERMISTOR_PIN);
  measured = 1023 / measured - 1;
  measured = SERIESRESISTOR / measured;
  Serial.print("Thermistor resistance: ");
  Serial.print(measured);
  Serial.println(" Ohms");
  float steinhart;
  steinhart = measured / THERMISTORNOMINAL;     // (R/Ro)
  steinhart = log(steinhart);                  // ln(R/Ro)
  steinhart /= BCOEFFICIENT;                   // 1/B * ln(R/Ro)
  steinhart += 1.0 / (TEMPERATURENOMINAL + 273.15); // + (1/To)
  steinhart = 1.0 / steinhart;                 // Invert
  steinhart -= 273.15;
  return steinhart * 0.65;
}

uint8_t read_target()
{
  uint16_t target = read_and_average(POTENTIOMETER_PIN);
  Serial.print("Pot: ");
  Serial.print(target);
  Serial.println("");
	return map(target, 0/*depends on the pot and PSU*/, 990/*depends on the pot and PSU1023*/, POT_RANGE_LOW, POT_RANGE_HIGH);
}

void control_relay(uint8_t estimated, uint8_t target)
{
	static bool activated = true;
	if (activated)
	{
		if (estimated + HYSTERESIS > target)
			activated = false;
	}
	else
	{
		if (estimated - HYSTERESIS < target)
			activated = true;
	}
	Serial.print("Relay is in the ");
	Serial.print(activated ? "on" : "off");
	Serial.println(" state.");
  digitalWrite(RELAY_PIN, activated);
  //digitalWrite(RELAY_PIN, 0);
}

void print_temperatures(uint8_t measured, uint8_t estimated, uint8_t target)
{
  Serial.print("Measured inside temp: ");
  Serial.print(measured);
  Serial.print(" *C; Estimated outside temp: ");
  Serial.print(estimated);
	Serial.print(" *C; Target temp: ");
	Serial.print(target);
	Serial.println(" *C");
}

void loop()
{
	uint8_t measured = read_measured();
	uint8_t target = read_target();
  uint8_t estimated = measured * IO_COEFF;
	print_temperatures(measured, estimated, target);
	control_relay(measured, target);
	delay(UPDATE_PERIOD); //Put to sleep?
}
