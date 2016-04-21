// This #include statement was automatically added by the Spark IDE.
#include "application.h"
#include "Adafruit_BMP085.h"
#include "Adafruit_DHT.h"
#include "RTCLib.h"
#include "blynk.h"
#include "SimpleTimer.h"

void SendLight(void);
void SendMotion(void);
void PublishRTCInfo(void);


#define DHTPIN A0     // what pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11		// DHT 11
#define DHTTYPE DHT22		// DHT 22 (AM2302)
//#define DHTTYPE DHT21		// DHT 21 (AM2301)

#define LIGHTSENSOR A2
#define PIRSENSOR A1

// This is theh key that binds this app with Blynk
char auth[] = "200b9aa3013f41b6be332549fb67ac21";

// Attach virtual serial terminal to Virtual Pin V1
WidgetTerminal terminal(V2);
WidgetLED lightLed(V7);
WidgetLED motionLed(V6);

/*
	Wiring
	------
	BMP085 Vcc to 3.3V
	BMP085 GND to GND
	BMP085 SCL to D1
	BMP085 SDA to D0
*/

Adafruit_BMP085 bmp;
DHT dht(DHTPIN, DHTTYPE);
RTCLib rtc;
SimpleTimer timer;

float temperature = 0;
float humidity = 0;
float pressure = 0;
bool motion = FALSE;
bool light = FALSE;

// Initialize BMP085

void InitializeBMP085(){
	if (!bmp.begin()) {
		Serial.println("Could not find a valid BMP085 sensor, check wiring!");
	}
}

void GetTemperature() {
	temperature = dht.getTempFarenheit();
}

void SendTemperature() {
	Blynk.virtualWrite(V3, temperature);
	terminal.print("temperature = ");
	terminal.print(temperature);
	terminal.println(" *F");
	terminal.flush();
	Particle.publish("librato_dht_t", String(temperature), 60, PRIVATE);
	terminal.println("published");
	terminal.flush();
}

void GetHumidity() {
	humidity = dht.getHumidity();
}

void SendHumidity() {
	Blynk.virtualWrite(V4, humidity);
	terminal.print("humidity = ");
	terminal.print(humidity);
	terminal.println(" %");
	terminal.flush();
	Particle.publish("librato_dht_h", String(humidity), 60, PRIVATE);
	terminal.println("published");
	terminal.flush();
}

void GetPressure() {
	pressure = bmp.readPressure() * 0.0002953;
}

void SendPressure() {
	Blynk.virtualWrite(V5, pressure);
	terminal.print("Pressure = ");
	terminal.print(pressure);
	terminal.println(" in Hg");
	terminal.flush();
	Particle.publish("librato_pressure", String(pressure), 60, PRIVATE);
	terminal.println("published");
	terminal.flush();
}

void GetMotion() {
	if (digitalRead(PIRSENSOR) && (motion != TRUE)) {
		motion = TRUE;
		motionLed.on();
		SendMotion();
	}
}

void SendMotion() {
	terminal.print("Motion = ");
	terminal.println(motion? "ON" : "OFF");
	terminal.flush();
}

void GetLight() {
	if ((digitalRead(LIGHTSENSOR) == FALSE) && (light != TRUE)) {
		light = TRUE;
		lightLed.on();
		SendLight();
	}
}

void SendLight() {
	terminal.print("Light = ");
	terminal.println(light? "ON" : "OFF");
	terminal.flush();
}

void UpdateTemperature() {
	GetTemperature();
	SendTemperature();
}

void UpdateHumidity() {
	GetHumidity();
	SendHumidity();
}

void UpdatePressure() {
	GetPressure();
	SendPressure();
}

void UpdateTime() {
	PublishRTCInfo();
}

void PublishRTCInfo() {

	char szRTCEventInfo[64];
	Serial.print("RTC DateTime: ");
	Serial.print(rtc.year());
	Serial.print('/');
	Serial.print(rtc.month());
	Serial.print('/');
	Serial.print(rtc.day());
	Serial.print(' ');
	Serial.print(rtc.hour());
	Serial.print(':');
	Serial.print(rtc.minute());
	Serial.print(':');
	Serial.print(rtc.second());
	Serial.print(" DOW: ");
	Serial.println(rtc.dayOfWeek());

	sprintf(szRTCEventInfo, "RTC Date: %02d/%02d/%02d %02d:%02d:%02d %d", rtc.year(), rtc.month(), rtc.day(), rtc.hour(), rtc.minute(), rtc.second(), rtc.dayOfWeek());

	Particle.publish("RTCInfo", szRTCEventInfo);
}

// Initialize applicaiton
void InitializeApplication() {
    Serial.begin(9600);

	pinMode(D7, OUTPUT);

  InitializeBMP085();

  dht.begin();

  // initialize light sensor
  pinMode(LIGHTSENSOR, INPUT);

  // initialize PIR sensor
  pinMode(PIRSENSOR, INPUT);
}

// Blink LED and wait for some time
void BlinkLED(unsigned char blinks) {
	for (unsigned char i = 0; i < blinks; i++) {
		digitalWrite(D7, HIGH);
    delay(150);
    digitalWrite(D7, LOW);
    delay(150);
	}
}

void LEDUpdate() {
	BlinkLED(1);
}

void setup() {

	delay(30000L);  // protection delay in case app is crashing

  InitializeApplication();

	// rtc.set(0, 0, 0, 6, 8, 4, 16);

	Blynk.begin(auth);

	while (Blynk.connect() == false) {
		delay(10); // Wait until connected
	}
	// This will print Blynk Software version to the Terminal Widget when
	// your hardware gets connected to Blynk Server
	terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
	terminal.flush();

	timer.setInterval(30000L, UpdateTemperature);
	timer.setInterval(50000L, UpdateHumidity);
	timer.setInterval(70000L, UpdatePressure);
	timer.setInterval(100, GetMotion);
	timer.setInterval(100, GetLight);
	timer.setInterval(200, LEDUpdate);
	timer.setInterval(2000L, UpdateTime);
}

BLYNK_WRITE(V8) //Button Widget is writing to pin V8
{
  int pinData = param.asInt(); // to reset light LED
	if (pinData == 0) {
		lightLed.off();
		light = FALSE;
		SendLight();
	}
}

BLYNK_WRITE(V9) //Button Widget is writing to pin V9
{
  int pinData = param.asInt(); // to reset motion LED
	if (pinData == 0) {
		motionLed.off();
		motion = FALSE;
		SendMotion();
	}
}

bool timeSet = FALSE;

void loop() {
		Blynk.run();
		timer.run();

		if (!timeSet) {
			rtc.set(0,0,12,7,1,1,66); // 12 pm on 1/1/1966, Saturday
			timeSet = TRUE;
		}

		rtc.refresh();
}
