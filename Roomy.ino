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
#define SCREEN_WIDTH 128  // OLED display width, in pixels
#define SCREEN_HEIGHT 64  // OLED display height, in pixels
#define OLED_RESET -1     // Reset pin # (or -1 if sharing Arduino reset pin)
Adafruit_SSD1306 display(SCREEN_WIDTH, SCREEN_HEIGHT, &Wire, OLED_RESET);
#define DHTPIN 0       // Digital pin connected to the DHT sensor
#define DHTTYPE DHT11  // DHT 11
int timezone = 5.5 * 3600;
int dst = 0;
String arr_days[] = { "SUN", "MON", "TUE", "WED", "THU", "FRI", "SAT" };
DHT dht(DHTPIN, DHTTYPE);
WiFiUDP ntpUDP;
NTPClient timeClient(ntpUDP, "pool.ntp.org", 19800, 60000);
char auth[] = BLYNK_AUTH_TOKEN;
int screen = 1, sec = 0, dc = 0, bledc = 0, wledc = 0, autoc = 1, fltrdlyctrl = 1, filterdelay = 0, filtersts = 0, ledlay = -1, fltrc = 0, dhtsts = 1, randomled = 0, wifidisconct = 0;
float ldrctrl = 0.6;
float ldrsts = 0;
const int wledr = 14;   // d5
const int bledr = 12;   // d6
const int filter = 13;  // d7
BlynkTimer timer;
void sendSensor() {
  float h = dht.readHumidity();
  float t = dht.readTemperature();        // or dht.readTemperature(true) for Fahrenheit
  float ldrsts = analogRead(A0) / 2.046;  // ldr sensor
  if (ldrsts >= 100) ldrsts = 100;
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
  Serial.println(ldrsts);
}
// wled button
BLYNK_WRITE(V14) {
  if (autoc) {
    autoc = 0;
    Blynk.virtualWrite(V15, LOW);
  }
  wledc = param.asInt();
}
// bled button
BLYNK_WRITE(V12) {
  if (autoc) {
    autoc = 0;
    Blynk.virtualWrite(V15, LOW);
  }
  bledc = param.asInt();
}
// ftlr button
BLYNK_WRITE(V13) {
  if (autoc) {
    autoc = 0;
    Blynk.virtualWrite(V15, LOW);
  }
  fltrc = param.asInt();
}
// auto button
BLYNK_WRITE(V15) {
  autoc = param.asInt();
}
// terminal commend
WidgetTerminal terminal(V1);
BLYNK_WRITE(V1) {
  String command = param.asStr();  // Read the received string command
  if (command.startsWith("ldr")) {
    String valueString = command.substring(4);
    ldrctrl = valueString.toFloat();
    terminal.println("ldr letency set to " + String(ldrctrl));
  } else if (String("hi") == command) {
    if (screen == 0) {
      terminal.println("Good morning :) ");
      screen = 1;
    } else {
      terminal.println("hello... there !!");
    }
  } else if (String("ldrsts") == command) {
    terminal.println(ldrsts);
  } else if (String("help") == command) {
    terminal.println("clear\nSLEEP->screen of\nhi->screen on\nRST->restart");
  } else if (String("RST") == command) {
    terminal.println("Restarting...");
    Serial.print("Initiating restart H.W...");
    ESP.reset();
  } else if (String("SLEEP") == command || String("Sleep") == command || String("sleep") == command) {
    terminal.println("GOODNIGHT :) ");
    screen = 0;
  } else if (String("clear") == command) {
    terminal.clear();
  } else if (String("auto") == command) {
    if (autoc == 0) {
      autoc = 1;
      terminal.println("Auto MODE ON");
    } else {
      autoc = 0;
      terminal.println("Auto MODE OFF");
    }
  } else if (String("ctrl") == command) {
    terminal.print("For LDR param type : 'ctrl ldr #'(0.00 to 3.00)\nFor filter time type : 'ctrl fltr #'(1,2)\n");
  } else if (String("ctrl fltr 1") == command) {
    fltrdlyctrl = 1;
  } else if (String("ctrl fltr 2") == command) {
    fltrdlyctrl = 2;
  } else if (String("wled") == command) {
    autoc = 1;
    terminal.println("Auto MODE OFF");
    if (wledc == 0) {
      wledc = 1;
      terminal.println("White LED ON");
    } else {
      wledc = 0;
      terminal.println("White LED OFF");
    }
  } else if (String("bled") == command) {
    autoc = 1;
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
  } else if (String("fltr") == command) {
    autoc = 1;
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
    terminal.write(param.getBuffer(), param.getLength());
    terminal.println();
  }
  // Ensure everything is sent
  terminal.flush();
}
void setup() {
  // serial strat
  Serial.begin(9600);
  // relay
  pinMode(wledr, OUTPUT);
  pinMode(bledr, OUTPUT);
  pinMode(filter, OUTPUT);
  // display
  if (!display.begin(SSD1306_SWITCHCAPVCC, 0x3C)) {
    Serial.println(F("SSD1306 allocation failed"));
    for (;;)
      delay(1000);  // Don't proceed, loop forever
  }
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
  display.setTextSize(2);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
  display.println("ROOMMY");
  display.display();
  delay(2000);
  display.clearDisplay();
  display.display();
  display.setTextSize(1);
  display.setTextColor(WHITE);
  display.setCursor(0, 0);
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
  display.clearDisplay();
  display.display();
  display.setCursor(0, 0);
  // wifi start
  display.println("Wifi Connected!");
  display.print("IP:");
  display.println(WiFi.localIP());
  display.display();
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
  // Blynk
  Blynk.begin(auth, ssid, pass);
  dht.begin();
  terminal.clear();
  terminal.println(F("Blynk v" BLYNK_VERSION ": Device started"));
  terminal.println(F("-------------"));
  terminal.println(F("Say 'hi' , or type help"));
  // Ensure everything is sent
  terminal.flush();
}
void loop() {
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
  float t = dht.readTemperature();
  int h = dht.readHumidity();
  int hr = 1, sec = 60, mint = 1, currentHour = 1;
  int m = 0;
  Blynk.connected();
  if (Blynk.connected()) {
    wifidisconct = 0;
    timeClient.update();
    if (isnan(h) || isnan(t)) {
      if (dc == 0) {
        Serial.println("Failed to read from DHT sensor!");
        dc = 1;
      }
    } else {
      dc = 0;
    }
    time_t now = time(nullptr);
    struct tm *p_tm = localtime(&now);
    hr = p_tm->tm_hour;
    currentHour = p_tm->tm_hour;
    mint = p_tm->tm_min;
    sec = p_tm->tm_sec;
    if (hr > 12) {
      hr = hr - 12;
      m = 1;
    }
    if (hr < 1) {
      hr = 12;
      m = 0;
    }
    display.clearDisplay();
    if (screen == 1) {
      display.setTextSize(3);
      display.setTextColor(WHITE);
      display.setCursor(0, 0);
      if (hr < 10) {
        display.print(" ");
      }
      display.print(hr);
      display.print(":");
      if (p_tm->tm_min < 10)
        display.print(" ");
      display.print(p_tm->tm_min);
      display.setTextSize(2);
      display.setCursor(90, 5);
      display.print(".");
      if (p_tm->tm_sec < 10)
        display.print(" ");
      display.print(p_tm->tm_sec);
      display.setTextSize(2);
      display.setCursor(0, 30);
      if ((p_tm->tm_mday) < 10) {
        display.print("0");
      }
      display.print(p_tm->tm_mday);
      display.setTextSize(1);
      display.print(".");
      display.setTextSize(2);
      if ((p_tm->tm_mon + 1) < 10) {
        display.print("0");
      }
      display.print(p_tm->tm_mon + 1);
      display.setCursor(54, 25);
      display.setTextSize(1);
      display.print(".");
      display.print(p_tm->tm_year + 1900);
      display.setTextSize(2);
      display.setCursor(54, 33);
      display.print(arr_days[timeClient.getDay()]);
      display.setCursor(90, 25);
      display.setTextSize(3);
      if (m) {
        display.print("PM");
      } else {
        display.print("AM");
      }
      display.setCursor(0, 50);
      display.setTextSize(2);
      display.print(t);
      display.setTextSize(1);
      display.cp437(true);
      display.write(167);
      display.setTextSize(2);
      display.print("C,");
      display.print(h);
      display.print("%");
      display.display();
    } else {
      display.setCursor(0, 0);
      display.setTextSize(6);
      display.setTextColor(BLACK);
      display.println("00");
      display.display();
    }
  }
  float ldrsts = analogRead(A0) * (5.0 / 1023.0);  // ldr sensor
  // aquarium light
  if (randomled != mint % 3)
    randomled = mint % 3;
  if (autoc) {
    if (currentHour >= 7 && currentHour < 23) {
      if (ldrsts < ldrctrl) {
        if (randomled == 0) {
          if (!wledc) {
            digitalWrite(wledr, HIGH);
            Blynk.virtualWrite(V12, HIGH);
            wledc = 1;
          }
          if (!bledc) {
            digitalWrite(bledr, HIGH);
            Blynk.virtualWrite(V14, HIGH);
            bledc = 1;
          }
        }
        if (randomled == 1) {
          if (!wledc) {
            digitalWrite(wledr, HIGH);
            Blynk.virtualWrite(V12, HIGH);
            wledc = 1;
          }
          if (bledc) {
            digitalWrite(bledr, LOW);
            Blynk.virtualWrite(V14, LOW);
            bledc = 0;
          }
        }
        if (randomled == 2) {
          if (!bledc) {
            digitalWrite(bledr, HIGH);
            Blynk.virtualWrite(V14, HIGH);
            bledc = 1;
          }
          if (wledc) {
            digitalWrite(wledr, LOW);
            Blynk.virtualWrite(V12, LOW);
            wledc = 0;
          }
        }
        if (sec + 5 > 60) {
          ledlay = sec + 5 - 60;
        } else {
          ledlay = sec + 5;
        }
      }
      if (sec == ledlay && ldrsts > ldrctrl) {
        ledlay = 60;
        if (wledc) {
          digitalWrite(wledr, LOW);
          Blynk.virtualWrite(V12, LOW);
          wledc = 0;
        }
        if (bledc) {
          digitalWrite(bledr, LOW);
          Blynk.virtualWrite(V14, LOW);
          bledc = 0;
        }
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
  } else {
    Serial.println("auto off");
    if (wledc == 0) {
      digitalWrite(wledr, LOW);
    }
    if (wledc == 1) {
      digitalWrite(wledr, HIGH);
    }
    if (bledc == 0) {
      digitalWrite(bledr, LOW);
    }
    if (bledc == 1) {
      digitalWrite(bledr, HIGH);
    }
    if (fltrc == 0) {
      digitalWrite(filter, LOW);
    }
    if (fltrc == 1) {
      digitalWrite(filter, HIGH);
    }
    if (sec + 5 > 60) {
      ledlay = sec + 5 - 60;
    } else {
      ledlay = sec + 5;
    }
  }
  Blynk.run();
  timer.run();
}
