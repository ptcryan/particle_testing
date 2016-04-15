// This #include statement was automatically added by the Spark IDE.
#include "application.h"
#include "Adafruit_BMP085.h"
#include "Adafruit_DHT.h"
//#include "RTCLib.h"
#include "blynk.h"
#include "SimpleTimer.h"

void SendLight(void);
void SendMotion(void);


#define DHTPIN A0     // what pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11		// DHT 11
#define DHTTYPE DHT22		// DHT 22 (AM2302)
//#define DHTTYPE DHT21		// DHT 21 (AM2301)

#define LIGHTSENSOR A2
#define PIRSENSOR A1

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
//RTCLib rtc;
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
		//while (1) {}
	}
}

// Publish Pressure, Altitude
void PublishBMP085Info(){
    Serial.print("Temperature = ");
    Serial.print(bmp.readTemperature());
    Serial.print(" *C  ");
    Serial.print((bmp.readTemperature() * 1.8) + 32);
    Serial.println(" *F");

    Serial.print("Pressure = ");
    Serial.print(bmp.readPressure());
    Serial.println(" Pa");

    // Calculate altitude assuming 'standard' barometric
    // pressure of 1013.25 millibar = 101325 Pascal
    Serial.print("Altitude = ");
    Serial.print(bmp.readAltitude());
    Serial.println(" meters");

  // you can get a more precise measurement of altitude
  // if you know the current sea level pressure which will
  // vary with weather and such. If it is 1015 millibars
  // that is equal to 101500 Pascals.
    Serial.print("Real altitude = ");
    Serial.print(bmp.readAltitude(101500));
    Serial.println(" meters");

    char szEventInfo[64];

    sprintf(szEventInfo, "Temperature=%.2f *C, Pressure=%.2f hPa", bmp.readTemperature(), bmp.readPressure()/100.0);

    Particle.publish("bmp085info", szEventInfo);
    delay(2000);

    Particle.publish("librato_pressure", String(bmp.readPressure()/100.0), 60, PRIVATE);
    delay(2000);
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
	if (digitalRead(LIGHTSENSOR) && (light != TRUE)) {
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

void PublishDHTInfo() {

  // Reading temperature or humidity takes about 250 milliseconds!
  // Sensor readings may also be up to 2 seconds 'old' (its a
  // very slow sensor)
	float h = dht.getHumidity();
  // Read temperature as Celsius
  float t = dht.getTempCelcius();
  // Read temperature as Farenheit
  float f = dht.getTempFarenheit();

  // Check if any reads failed and exit early (to try again).
  if (isnan(h) || isnan(t) || isnan(f)) {
		Serial.println("Failed to read from DHT sensor!");
    return;
	}

  // Compute heat index
  // Must send in temp in Fahrenheit!
  float hi = dht.getHeatIndex();
  float dp = dht.getDewPoint();
  float k = dht.getTempKelvin();

  Serial.print("Humid: ");
  Serial.print(h);
  Serial.print("% - ");
  Serial.print("Temp: ");
  Serial.print(t);
  Serial.print("*C ");
  Serial.print(f);
  Serial.print("*F ");
	Serial.print(k);
  Serial.print("*K - ");
  Serial.print("DewP: ");
  Serial.print(dp);
  Serial.print("*C - ");
  Serial.print("HeatI: ");
  Serial.print(hi);
	Serial.println("*C");
  Serial.println(Time.timeStr());

  char szDHTEventInfo[64];

  sprintf(szDHTEventInfo, "Temperature=%.2f *C, humidity=%.2f", t, h);

  // Particle.publish("DHTinfo", szDHTEventInfo);

  Particle.publish("librato_dht_t", String(t), 60, PRIVATE);
  delay(2000);
  Particle.publish("librato_dht_h", String(h), 60, PRIVATE);
  delay(2000);

	while (Blynk.connect() == false) {
		delay(10); // Wait until connected
	}

	terminal.print(szDHTEventInfo);
	terminal.print("\n");
	terminal.flush();
}



void PublishLightInfo() {

  char szLightEventInfo[64];

  sprintf(szLightEventInfo, "Light=%s", digitalRead(LIGHTSENSOR)? "ON" : "OFF");

  Particle.publish("LightInfo", szLightEventInfo);
  delay(2000);

  Particle.publish("librato_light", digitalRead(LIGHTSENSOR)? "ON" : "OFF", 60, PRIVATE);
  delay(2000);
}

void PublishPIRInfo() {

  char szPIREventInfo[64];

  sprintf(szPIREventInfo, "Movement=%s", digitalRead(PIRSENSOR)? "YES" : "NO");

  Particle.publish("PIRInfo", szPIREventInfo);
  delay(2000);

  Particle.publish("librato_PIR", digitalRead(PIRSENSOR)? "YES" : "NO", 60, PRIVATE);
  delay(2000);
}

/*void PublishRTCInfo() {

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

	sprintf(szRTCEventInfo, "RTC Date: %d/%d/%d %d:%d:%d %d", rtc.year(), rtc.month(), rtc.day(), rtc.hour(), rtc.minute(), rtc.second(), rtc.dayOfWeek());

	Particle.publish("RTCInfo", szRTCEventInfo);
	delay(2000);
}*/

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

	timer.setInterval(5000L, GetTemperature);
	timer.setInterval(10000L, SendTemperature);
	timer.setInterval(5000L, GetHumidity);
	timer.setInterval(10000L, SendHumidity);
	timer.setInterval(5000L, GetPressure);
	timer.setInterval(10000L, SendPressure);
	timer.setInterval(100, GetMotion);
	// timer.setInterval(20000L, SendMotion);
	timer.setInterval(100, GetLight);
	// timer.setInterval(20000L, SendLight);
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

void loop() {
    // Publish events. Wait for 2 second between publishes
    //PublishBMP085Info();
    //PublishDHTInfo();
    //PublishLightInfo();
    //PublishPIRInfo();

		Blynk.run();
		timer.run();

		// terminal.println("Hello!");
		// terminal.flush();

		// rtc.refresh();

		// PublishRTCInfo();

    BlinkLED(1);

  // Wait a few seconds between measurements.
  //delay(2000);
}
