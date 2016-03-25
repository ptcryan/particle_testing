// This #include statement was automatically added by the Spark IDE.
#include "application.h"
#include "Adafruit_BMP085.h"
#include "Adafruit_DHT.h"

#define DHTPIN A0     // what pin we're connected to
// Uncomment whatever type you're using!
//#define DHTTYPE DHT11		// DHT 11
#define DHTTYPE DHT22		// DHT 22 (AM2302)
//#define DHTTYPE DHT21		// DHT 21 (AM2301)

#define LIGHTSENSOR A2
#define PIRSENSOR A1

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

// Initialize BMP085

void InitializeBMP085(){
	if (!bmp.begin()) {
		Serial.println("Could not find a valid BMP085 sensor, check wiring!");
		while (1) {}
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

    sprintf(szEventInfo, "Temperature=%.2f �C, Pressure=%.2f hPa", bmp.readTemperature(), bmp.readPressure()/100.0);

    Particle.publish("bmpo85info", szEventInfo);
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

  Particle.publish("DHTinfo", szDHTEventInfo);
}

void PublishLightInfo() {
  char szLightEventInfo[64];

  sprintf(szLightEventInfo, "Light=%s", digitalRead(LIGHTSENSOR)? "ON" : "OFF");

  Particle.publish("LightInfo", szLightEventInfo);
}

void PublishPIRInfo() {
  char szPIREventInfo[64];

  sprintf(szPIREventInfo, "Movement=%s", digitalRead(PIRSENSOR)? "YES" : "NO");

  Particle.publish("PIRInfo", szPIREventInfo);
}

// Initialize applicaiton
void InitializeApplication(){
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
void BlinkLED(){
    digitalWrite(D7, HIGH);
    delay(250);
    digitalWrite(D7, LOW);
    delay(250);
    digitalWrite(D7, HIGH);
    delay(250);
    digitalWrite(D7, LOW);
    delay(250);
}

void setup() {
  InitializeApplication();
}

void loop() {
    // Publish events. Wait for 2 second between publishes
    PublishBMP085Info();
    PublishDHTInfo();
    PublishLightInfo();
    PublishPIRInfo();

    BlinkLED();

  // Wait a few seconds between measurements.
  	delay(2000);
}
