#include <Arduino.h>
#include <WiFi.h>
#include <WiFiMulti.h>
#include "soc/rtc_wdt.h"

WiFiMulti WiFiMulti;

#define PIN_STROBE_N 13
#define PIN_D0 4
#define PIN_D1 14
#define PIN_D2 27
#define PIN_D3 26
#define PIN_D4 25
#define PIN_D5 33
#define PIN_D6 32
#define PIN_D7 18
#define PIN_ACK_N 15
#define PIN_BUSY 17
#define PIN_PAPEROUT 16
#define PIN_SELECTIN 35
#define PIN_LINEFEED_N 23
#define PIN_ERROR 22
#define PIN_RESET 21
#define PIN_SELECTPRINTER_N 19

volatile bool strobed = false;
const uint16_t port = 31337;
const char * host = "targetIP";

void IRAM_ATTR strobe()
{
	digitalWrite(PIN_BUSY, HIGH);
	strobed = true;
}

void setup()
{
	Serial.begin(115200);

	rtc_wdt_protect_off();
	rtc_wdt_disable();

	WiFiMulti.addAP("SSID", "PSK");

	Serial.println();
	Serial.println();
	Serial.print("Waiting for WiFi... ");

	while(WiFiMulti.run() != WL_CONNECTED)
	{
		Serial.print(".");
		delay(500);
	}

	Serial.println("");
	Serial.println("WiFi connected");
	Serial.println("IP address: ");
	Serial.println(WiFi.localIP());

	pinMode(PIN_STROBE_N, INPUT_PULLUP);
	pinMode(PIN_D0, INPUT_PULLUP);
	pinMode(PIN_D1, INPUT_PULLUP);
	pinMode(PIN_D2, INPUT_PULLUP);
	pinMode(PIN_D3, INPUT_PULLUP);
	pinMode(PIN_D4, INPUT_PULLUP);
	pinMode(PIN_D5, INPUT_PULLUP);
	pinMode(PIN_D6, INPUT_PULLUP);
	pinMode(PIN_D7, INPUT_PULLUP);
	pinMode(PIN_ACK_N, INPUT_PULLUP);
	pinMode(PIN_BUSY, INPUT_PULLUP);
	pinMode(PIN_PAPEROUT, INPUT_PULLUP);
	pinMode(PIN_SELECTIN, INPUT);
	pinMode(PIN_LINEFEED_N, INPUT_PULLUP);
	pinMode(PIN_ERROR, INPUT_PULLUP);
	pinMode(PIN_RESET, INPUT_PULLUP);
	pinMode(PIN_SELECTPRINTER_N, INPUT_PULLUP);

	pinMode(PIN_BUSY, OUTPUT);
	digitalWrite(PIN_BUSY, LOW);

	pinMode(PIN_PAPEROUT, OUTPUT);
	digitalWrite(PIN_PAPEROUT, LOW);

	pinMode(PIN_ERROR, OUTPUT);
	digitalWrite(PIN_ERROR, HIGH);

	pinMode(PIN_SELECTPRINTER_N, OUTPUT);
	digitalWrite(PIN_SELECTPRINTER_N, HIGH);

	pinMode(PIN_ACK_N, OUTPUT);
	digitalWrite(PIN_ACK_N, HIGH);

	Serial.println("ready");
	attachInterrupt(PIN_STROBE_N, strobe, FALLING);
}

WiFiClient client;
uint32_t lastByte = 0;
bool printActive = 0;

void loop()
{
	while (true)
	{
		yield();
		if (strobed)
		{
			strobed = false;
			printActive = true;

			if (!client.connected())
			{
				Serial.println("connecting to TCP socket");
				if (!client.connect(host, port)) {
					Serial.println("Connection failed.");
					Serial.println("Waiting 5 seconds before retrying...");
					return;
				}
				Serial.println("connected!");
			}


			while(!digitalRead(PIN_STROBE_N)) {}

			uint8_t data = 0x00;
			bitWrite(data, 0, digitalRead(PIN_D0));
			bitWrite(data, 1, digitalRead(PIN_D1));
			bitWrite(data, 2, digitalRead(PIN_D2));
			bitWrite(data, 3, digitalRead(PIN_D3));
			bitWrite(data, 4, digitalRead(PIN_D4));
			bitWrite(data, 5, digitalRead(PIN_D5));
			bitWrite(data, 6, digitalRead(PIN_D6));
			bitWrite(data, 7, digitalRead(PIN_D7));
			//Serial.write(data);
			//Serial.flush();
			
			client.write(data);
			lastByte = millis();

			digitalWrite(PIN_BUSY, LOW);
			digitalWrite(PIN_ACK_N, LOW);
			delayMicroseconds(10);
			digitalWrite(PIN_ACK_N, HIGH);
		}

		if ((lastByte + 1000 < millis()) && printActive)
		{
			Serial.println("No data for a while. Ending TCP connection.");
			client.stop();
			printActive = false;
		}
	}
}