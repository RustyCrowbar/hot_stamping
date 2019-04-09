#define HYSTERESIS 10
#define UPDATE_PERIOD 1000
//#define CALIBRATION_FACTOR 1
//#define CALIBRATION_OFFSET 0
#define POT_RANGE_LOW 30
#define POT_RANGE_HIGH 70
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
  //measured = steinhart;
	//return measured;
  return steinhart;
}

uint8_t read_target()
{
  uint16_t target = read_and_average(POTENTIOMETER_PIN);
	return map(target, 0, 1023, POT_RANGE_LOW, POT_RANGE_HIGH);
}

void control_relay(uint8_t measured, uint8_t target)
{
	static bool activated = true;
	if (activated)
	{
		if (measured + HYSTERESIS > target)
			activated = false;
	}
	else
	{
		if (measured - HYSTERESIS < target)
			activated = true;
		
	}
	Serial.print("Relay is in the ");
	Serial.print(activated ? "on" : "off");
	Serial.println(" state.");
	//write !activated on pin (relay must be off by default).
  digitalWrite(RELAY_PIN, activated);
}

void print_temperatures(uint8_t measured, uint8_t target)
{
	Serial.print("Measured temp: ");
	Serial.print(measured);
	Serial.print(" *C; Target temp: ");
	Serial.print(target);
	Serial.println(" *C");
}

void loop()
{
	uint8_t measured = read_measured();
	uint8_t target = read_target();
	print_temperatures(measured, target);
	control_relay(measured, target);
	delay(UPDATE_PERIOD); //Put to sleep?
}
