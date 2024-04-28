// type your BLYNK_TEMPLATE_ID
#define BLYNK_TEMPLATE_ID "************"
// type your BLYNK_TEMPLATE_NAME
#define BLYNK_TEMPLATE_NAME "*****"
// type your BLYNK_AUTH_TOKEN
#define BLYNK_AUTH_TOKEN "********************************"
char ssid[] = "****";        // type your wifi SSID
char pass[] = "********";  // type your wifi password
//======================================================// 

#define BLYNK_PRINT Serial
#include <ESP8266WiFi.h>
#include <BlynkSimpleEsp8266.h>
#include <DHT.h>
#include <SPI.h>
#include <NTPClient.h>
#include <WiFiUdp.h>
#include <Wire.h>
#include <Adafruit_GFX.h>
#include <Adafruit_SSD1306.h>
#define SCREEN_WIDTH 128 // OLED display width, in pixels
#define SCREEN_HEIGHT 64 // OLED display height, in pixels
#define OLED_RESET -1	 // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define DHTPIN 0	  // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11 // DHT 11
int timezone = 5.5 * 3600;
int dst = 0;
String arr_days[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
DHT dht(DHTPIN, DHTTYPE);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);
char auth[] = BLYNK_AUTH_TOKEN;
int screen = 1, sec = 0, bledc = 0, wledc = 0, autoMode = 1, fltrdlyctrl = 1, filterdelay = 0, filtersts = 0, ledlay = -1, fltrc = 0, dhtsts = 0, randomled = 0, wifidisconct = 0;
float ldrctrl = 1;
float ldrsts = 0;
const int wledr = 14;  // d5  V14
const int bledr = 12;  // d6  V12
const int filter = 13; // d7  V13
BlynkTimer timer;

// wled button
BLYNK_WRITE(V14) {
	if (autoMode) {
		autoMode = 0;
		Blynk.virtualWrite(V15, LOW);
	}
	wledc = param.asInt();
}
// bled button
BLYNK_WRITE(V12) {
	if (autoMode) {
		autoMode = 0;
		Blynk.virtualWrite(V15, LOW);
	}
	bledc = param.asInt();
}
// ftlr button
BLYNK_WRITE(V13) {
	if (autoMode) {
		autoMode = 0;
		Blynk.virtualWrite(V15, LOW);
	}
	fltrc = param.asInt();
}
// auto button
BLYNK_WRITE(V15) {
	autoMode = param.asInt();
}
// terminal commend
WidgetTerminal terminal(V1);
BLYNK_WRITE(V1) {
	String command = param.asStr(); // Read the received string command
	if (command.startsWith("set ldr")) {
		String valueString = command.substring(8);
		ldrctrl = valueString.toFloat();
		terminal.println("ldr letency set to " + String(ldrctrl));
		terminal.flush();
		return;
	}
	if (command.equals("hi")) {
		if (screen == 0) {
			terminal.println("Good morning :) ");
		} else {
			terminal.println("hello... there !!");
		}
		screen = 1;
	} else if (command.equals("ldrsts")) {
		terminal.println(ldrsts);
		terminal.println("say hi, to display clock again.");
		screen = 4;
	} else if (command.equals("help")) {
		terminal.println("clear\nSLEEP->screen of\nhi->screen on\nRST->restart");
	} else if (command.equals("RST")) {
		terminal.println("Restarting...");
		Serial.print("Initiating restart H.W...");
		ESP.reset();
	} else if (command.equals("SLEEP") || command.equals("Sleep") || command.equals("sleep") ||
			command.equals("slp") || command.equals("SLP") || command.equals("Slp")) {
		terminal.println("GOODNIGHT :) ");
		screen = 0;
	} else if (command.equals("clear")) {
		terminal.clear();
	} else if (command.equals("auto")) {
		if (autoMode == 0) {
			autoMode = 1;
			terminal.println("Auto MODE ON");
		} else {
			autoMode = 0;
			terminal.println("Auto MODE OFF");
		}
	} else if (command.equals("set")) {
		terminal.print("For LDR param type : 'set ldr #'(0.00 to 3.00)\nFor filter time type : 'set fltr #'(1,2)houres\n");
	} else if (command.equals("ctrl fltr 1")) {
		fltrdlyctrl = 1;
	} else if (command.equals("ctrl fltr 2")) {
		fltrdlyctrl = 2;
	} else if (command.equals("wled")) {
		autoMode = 1;
		terminal.println("Auto MODE OFF");
		if (wledc == 0) {
			wledc = 1;
			terminal.println("White LED ON");
		} else {
			wledc = 0;
			terminal.println("White LED OFF");
		}
	} else if (command.equals("bled")) {
		autoMode = 1;
		terminal.println("Auto MODE OFF");
		if (bledc == 0) {
			bledc = 1;
			Blynk.virtualWrite(V15, HIGH);
			terminal.println("Blue LED ON");
		} else {
			bledc = 0;
			Blynk.virtualWrite(V15, LOW);
			terminal.println("Blue LED OFF");
		}
	} else if (command.equals("fltr")) {
		autoMode = 1;
		terminal.println("Auto MODE OFF");
		if (fltrc == 0) {
			fltrc = 1;
			terminal.println("Filter ON");
		} else {
			fltrc = 0;
			terminal.println("Filter OFF");
		}
	} else {
		// Send it back
		terminal.print("You said:");
		terminal.write(command.c_str(), command.length());
		terminal.println();
	}

	// Ensure everything is sent
	terminal.flush();
}

/* Function to clear and display
 * @param clear
 * @param size
 * @param cursorX
 * @param cursorY
 * @param sleep
 * @return void
 */
void newDisplay(bool clear, int size, int cursorX, int cursorY, bool color = true) {
	if (clear) {
		display.clearDisplay();
		display.display();
	}
	if (size)
		display.setTextSize(size);
	if (color){
		display.setTextColor(WHITE);
	} else {
		display.setTextColor(BLACK);
	}
	if (cursorX >= 0 && cursorY >= 0)
		display.setCursor(cursorX, cursorY);
}

/* Function to start display
 * @return void
 */
void setupDisplay() {
	// serial strat
	Serial.begin(9600);
	// check display started.
	if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
		Serial.println(F("SSD1306 allocation failed"));
		for (;;)
			delay(1000); // Don't proceed, loop forever
	}
	display.display();
	delay(2000);
	newDisplay(true, 2, 0, 0);
	display.println("ROOMMY");
	display.display();
	delay(2000);
}

/* Function to start WiFi.
 * @return void
 */
void setupWiFi() {
	newDisplay(true, 1, 0, 0);
	display.println("Wifi connecting to ");
	display.println(ssid);
	WiFi.begin(ssid, pass);
	display.println("\nConnecting");
	display.display();
	// wifi stats
	while (WiFi.status() != WL_CONNECTED) {
		delay(500);
		Serial.print(".");
		display.print(".");
		display.display();
	}
	// Clear the buffer.
	newDisplay(true, 1, 0, 0);
	// wifi start
	display.println("Wifi Connected!");
	display.print("IP:");
	display.println(WiFi.localIP());
	display.display();
}

/* Function to read sensons and sync data with Blynk.
 * @return void
 */
void sendSensor() {
	float h = dht.readHumidity();
	float t = dht.readTemperature();	   // or dht.readTemperature(true) for Fahrenheit
	float rawldr = analogRead(A0);
	float ldrsts = map(rawldr, 0, 200, 0, 100); // ldr sensor
	if (ldrsts >= 100)
		ldrsts = 100;
	if(ldrsts < 0)
		ldrsts = 0;
	if ((isnan(h) || isnan(t))) {
		if (dhtsts) {
			Serial.println("Failed to read from DHT sensor!");
		}
		dhtsts = 0;
		return;
	}
	dhtsts = 1;
	// You can send any value at any time.
	// Please don't send more that 10 values per second.
	Blynk.virtualWrite(V6, t);
	Blynk.virtualWrite(V5, h);
	Blynk.virtualWrite(V7, ldrsts);
	Serial.print("Temperature : ");
	Serial.print(t);
	Serial.print("    Humidity : ");
	Serial.print(h);
	Serial.print("    LRD status : ");
	Serial.println(rawldr);
}

/* Function to connect with NTP,
 * To get current time and date.
 * @return void
 */
void connectToNTP() {
	configTime(timezone, dst, "pool.ntp.org", "time.nist.gov");
	display.println("\nWaiting for NTP...");
	timeClient.begin();
	while (!time(nullptr)) {
		Serial.print("*");
		delay(1000);
	}
	display.println("\nTime response....OK");
	display.display();
	delay(1000);
	display.clearDisplay();
	display.display();
	timer.setInterval(1000L, sendSensor);
}

/* Function to connect with Blynk.
 * @return void
 */
void connectToBlynk() {
	Blynk.begin(auth, ssid, pass);
	dht.begin();
	terminal.clear();
	terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
	terminal.println(F("-------------"));
	terminal.println(F("Say 'hi' , or type help"));
	// Ensure everything is sent
	terminal.flush();
}

/* Function to conection with WiFi and Blynk.
 * @return void
 */
void checkConnection() {
	if (!Blynk.connected()) {
		display.clearDisplay();
		display.setCursor(0, 0);
		display.setTextSize(4);
		display.println("(-_-)");
		display.setCursor(0, 50);
		display.setTextSize(2);
		display.println("No WiFi !");
		display.display();
		delay(2000);
		wifidisconct++;
		if (wifidisconct == 500) {
			ESP.reset();
		}
	}
}

/* Function to display and refrash digital clock on sceen.
 * @return void
 */
void displayDigitalClock( struct tm* p_tm, int hr, int m, float t, int h ) {
	newDisplay(false, 3, 0, 0);
	if (hr < 10) {
		display.print(" ");
	}
	display.print(hr);
	display.print(":");
	if (p_tm->tm_min < 10)
		display.print(" ");
	display.print(p_tm->tm_min);
	newDisplay(false, 2, 90, 5);
	display.print(".");
	if (p_tm->tm_sec < 10)
		display.print(" ");
	display.print(p_tm->tm_sec);
	newDisplay(false, 2, 0, 30);
	if ((p_tm->tm_mday) < 10) {
		display.print("0");
	}
	display.print(p_tm->tm_mday);
	newDisplay(false, 1, -1, -1);
	display.print(".");
	newDisplay(false, 2, -1, -1);
	if ((p_tm->tm_mon + 1) < 10) {
		display.print("0");
	}
	display.print(p_tm->tm_mon + 1);
	newDisplay(false, 1, 54, 25);
	display.print(".");
	display.print(p_tm->tm_year + 1900);
	newDisplay(false, 2, 54, 33);
	display.print(arr_days[timeClient.getDay()]);
	newDisplay(false, 3, 90, 25);
	if (m) {
		display.print("PM");
	} else {
		display.print("AM");
	}
	if(dhtsts) {
		newDisplay(false, 2, 0, 50);
		display.print(t);
		newDisplay(false, 1, -1, -1);
		display.cp437(true);
		display.write(167);
		newDisplay(false, 2, -1, -1);
		display.print("C,");
		display.print(h);
		display.print("%");
	}
	display.display();
}

/* Function to display and refrash LDR gauge on sceen.
 * @return void
 */
void displayLDRgauge(float percentage) {

	// Display on OLED
	int centerX =  (SCREEN_WIDTH/ 2) + (SCREEN_WIDTH - SCREEN_HEIGHT) / 2;
	int centerY = SCREEN_HEIGHT / 2;
	int radius = min(SCREEN_WIDTH, SCREEN_HEIGHT) / 2 - 1; // Adjust based on screen size and gauge design

	// Draw gauge outline
	display.drawCircle(centerX, centerY, radius, WHITE);

	// Calculate end point of gauge indicator based on percentage
	float angle = map(percentage, 50, 100, -135, 135) * PI / 180.0;
	int x2 = centerX + radius * cos(angle);
	int y2 = centerY + radius * sin(angle);

	// Draw gauge indicator
	newDisplay(false, 2, -1,-1);
	display.drawLine(centerX, centerY, x2, y2, WHITE);


	newDisplay(false, 3, centerX - 22, centerY - 10);
	display.println((int)percentage);
	
	display.display();
}

/* Function to switch on/off white led.
 * @return void
 */
void toggelWhiteLED() {
	if (!wledc) {
		digitalWrite(wledr, HIGH);
		Blynk.virtualWrite(V12, HIGH);
		wledc = 1;
	} else {
		digitalWrite(wledr, LOW);
		Blynk.virtualWrite(V12, LOW);
		wledc = 0;
	}
}
/* Function to switch on/off blue led.
 * @return void
 */
void toggelBlueLED() {
	if (!bledc) {
		digitalWrite(bledr, HIGH);
		Blynk.virtualWrite(V14, HIGH);
		bledc = 1;
	} else {
		digitalWrite(bledr, LOW);
		Blynk.virtualWrite(V14, LOW);
		bledc = 0;
	}
}

// setup the init function.
void setup() {
	// relay
	pinMode(wledr, OUTPUT);
	pinMode(bledr, OUTPUT);
	pinMode(filter, OUTPUT);

	setupDisplay();
	setupWiFi();
	connectToNTP();
	connectToBlynk();
}

// setup the main function.
void loop() {
	checkConnection();

	float t = dht.readTemperature();
	int h = dht.readHumidity();
	float rawldr = analogRead(A0);
	float ldrsts = map(rawldr, 50, 200, 0, 100)/10; // ldr sensor
	if(ldrsts<0)
		ldrsts = 0;
	int hr = 1, sec = 60, mint = 1, currentHour = 1;
	int m = 0;

	// Blynk.connected();
	if (Blynk.connected()) {
		wifidisconct = 0;
		timeClient.update();
		time_t now = time(nullptr);
		struct tm* p_tm = localtime(&now);
		hr = p_tm->tm_hour;
		currentHour = p_tm->tm_hour;
		mint = p_tm->tm_min;
		if (hr > 12) {
			hr = hr - 12;
			m = 1;
		}
		if (hr < 1) {
			hr = 12;
			m = 0;
		}

		display.clearDisplay();
		if(screen == 0){
			newDisplay(false, 6, 0, 0, false);
			display.setTextColor(BLACK);
			display.println("00");
			display.display();
		} else if (screen == 1 && sec != p_tm->tm_sec) {
			displayDigitalClock( p_tm, hr,m, t, h);
		} else if (screen == 2) {
			newDisplay(false, 2, 0, (SCREEN_HEIGHT / 2) - 10);
			display.print("TEMP");
			if(dhtsts){
				displayLDRgauge(t);
			} else {
				display.print(" NEED RESTART");
				display.display();
			}
		} else if (screen == 3) {
			newDisplay(false, 2, 0, (SCREEN_HEIGHT / 2) - 10);
			display.print("HMDT");
			if(dhtsts){
				displayLDRgauge(h);
			} else {
				display.print(" NEED RESTART");
				display.display();
			}
		}  else if (screen == 4) {
			newDisplay(false, 3, 0, (SCREEN_HEIGHT / 2) - 10);
			display.print("LDR");
			displayLDRgauge(rawldr);
		}
		sec = p_tm->tm_sec;
		if(screen > 0 && screen < 4 && screen - 1 != mint % 3)
			screen = (mint % 3) + 1;
	}
	// aquarium light
	if (randomled != mint % 3)
		randomled = mint % 3;
	if (autoMode) {
		if (currentHour >= 7 && currentHour < 23) {
			// chnage led color on night
			if (ldrsts < ldrctrl) {
				if (randomled == 0) {
					if (!bledc)
						toggelBlueLED();
					if (wledc)
						toggelWhiteLED();
				}
				if (randomled == 1)
					if (!wledc)
						toggelWhiteLED();
				if (randomled == 2)
					if (bledc)
						toggelBlueLED();
				ledlay = sec + 5;
				if (ledlay >= 60)
					ledlay = ledlay - 60;
			}
			// off led on day light
			if (sec == ledlay && ldrsts > ldrctrl) {
				ledlay = 60;
				if (wledc)
					toggelWhiteLED();
				if (bledc)
					toggelBlueLED();
			}
			// aquarium filter
			if (filterdelay <= hr) {
				if (!filtersts) {
					digitalWrite(filter, HIGH);
					Blynk.virtualWrite(V13, HIGH);
					fltrc = 1;
					filtersts = 1;
					filterdelay = hr + 2;
				} else {
					digitalWrite(filter, LOW);
					Blynk.virtualWrite(V13, LOW);
					fltrc = 0;
					filtersts = 0;
					filterdelay = hr + fltrdlyctrl;
				}
			}
		} else {
			// off all on mid night.
			if (wledc || bledc || fltrc) {
				digitalWrite(bledr, LOW);
				digitalWrite(wledr, LOW);
				digitalWrite(filter, LOW);
				Blynk.virtualWrite(V12, LOW);
				Blynk.virtualWrite(V13, LOW);
				Blynk.virtualWrite(V14, LOW);
				wledc = 0;
				bledc = 0;
				fltrc = 0;
				filterdelay = 0;
				ledlay = -1;
			}
		}
	} else { // Manually controls for all.
		if (wledc == 0)
			digitalWrite(wledr, LOW);
		if (wledc == 1)
			digitalWrite(wledr, HIGH);
		if (bledc == 0)
			digitalWrite(bledr, LOW);
		if (bledc == 1)
			digitalWrite(bledr, HIGH);
		if (fltrc == 0)
			digitalWrite(filter, LOW);
		if (fltrc == 1)
			digitalWrite(filter, HIGH);
		ledlay = sec + 5;
		if (ledlay >= 60)
			ledlay = ledlay - 60;
	}
	Blynk.run();
	timer.run();
}
