#include <Arduino.h>
//wroom devkit v1.0.6
#include <SPI.h>
#include <Ethernet.h>
#include <PubSubClient.h>
#include <WiFi.h>
#include <AsyncTCP.h>
#include <ESPAsyncWebServer.h>
#include <AsyncElegantOTA.h>
#include <TimeLib.h>
#include <Wire.h>  // Only needed for Arduino 1.6.5 and earlier
#include <SSD1306Wire.h> // legacy include: `#include "SSD1306.h"`
#include <OLEDDisplayUi.h>
#include <MD_Parola.h>
#include <MD_MAX72xx.h>
#include "images.h"

#define display_matrix    1
#define ser               1
#define BZ                13
#define GP2               2
#define GP3               4
#define Mat               14
#define RL               15
#define SPI_CUSTOM        1
#define TZ (+7*60*60) //Timezone

int BUILTIN_LED = 2;
const char* ssid = "orangepi";
const char* password = "112233445566";


SSD1306Wire display(0x3c, SDA, SCL);
OLEDDisplayUi ui ( &display );

int res = 0;
char Jam[9];
int jam, menit, detik, hari;
int hitung = 0;
int screenW = 128;
int screenH = 64;

int clockCenterX = screenW / 2;
int clockCenterY = ((screenH - 16) / 2) + 16; // top yellow part is 16 px height
int clockRadius = 23;
static const char* const wd[7] = {"Minggu", "Senin", "Selasa", "Rabu", "Kamis", "Jum'at", "Sabtu"};

String ipethernet;
AsyncWebServer server(80);
#define HARDWARE_TYPE MD_MAX72XX::FC16_HW
#define MAX_DEVICES 8
#define CLK_PIN   25
#define DATA_PIN  27
#define CS_PIN    26
MD_Parola P = MD_Parola(HARDWARE_TYPE, DATA_PIN, CLK_PIN, CS_PIN, MAX_DEVICES);
#define CHAR_SPACING  1 
const uint16_t WAIT_TIME = 1000;
byte mac[]    = {  0xDE, 0xED, 0xBA, 0xFE, 0xFE, 0xE2 };
IPAddress ip(192, 168, 100, 100);
const char* ssid2 = "HOTSPOT";
const char* password2 = "";
const char* mqtt_server = "broker.mqtt-dashboard.com";
EthernetClient ethClient;
PubSubClient client(ethClient);

unsigned long lastMsg = 0;
#define MSG_BUFFER_SIZE  (50)
char msg[MSG_BUFFER_SIZE];
char Led2[2];
float value = 0.0;
float tmp = 0.0;
int val_led;
bool oke = 0;

void callback(char* topic, byte* payload, unsigned int length) {
  String top;
  String pay;
  Serial.print("Message arrived [");
  Serial.print(topic);
  Serial.println("] ");
  for (int i = 0; i < length; i++) {
    //    Serial.print((char)topic[i]);
    top += topic[i];
  }
  for (int i = 0; i < length; i++) {
    //    Serial.print((char)payload[i]);
    pay += (char)payload[i];
  }
  Serial.print("topic ");
  Serial.print(topic);
  Serial.print("\t");
  Serial.print("top ");
  Serial.print(top);
  Serial.print("\t");
  Serial.print("payload ");
  Serial.println(pay);

  Serial.println();
  if (top == "L") {
    if (pay == "1") {
      val_led = 1;
      digitalWrite(RL, val_led);   // Turn the LED on (Note that LOW is the voltage level
      //      client.publish("Led2_", "1");
    } else if (pay == "0") {
      val_led = 0;
      digitalWrite(RL, val_led);  // Turn the LED off by making the voltage HIGH
      //      client.publish("Led2_", "0");
    }
    oke = 1;
//    bunyi();
  }
}
void setup_() {
  Ethernet.init(5);   // MKR ETH shield
  Ethernet.begin(mac);
  // Check for Ethernet hardware present
  if (Ethernet.hardwareStatus() == EthernetNoHardware) {
    Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
  }
  while (Ethernet.linkStatus() == LinkOFF) {
    Serial.println("Ethernet cable is not connected.");
    delay(500);
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);
  Serial.println("connecting...");
  client.setServer(mqtt_server, 1883);
  client.setCallback(callback);
}
String twoDigits(int digits) {
  if (digits < 10) {
    String i = '0' + String(digits);
    return i;
  }
  else {
    return String(digits);
  }
}

void clockOverlay(OLEDDisplay *display, OLEDDisplayUiState* state) {

}

void analogClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  //  ui.disableIndicator();

  // Draw the clock face
  //  display->drawCircle(clockCenterX + x, clockCenterY + y, clockRadius);
  display->drawCircle(clockCenterX + x, clockCenterY + y, 2);
  //
  //hour ticks
  for ( int z = 0; z < 360; z = z + 30 ) {
    //Begin at 0° and stop at 360°
    float angle = z ;
    angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
    int x2 = ( clockCenterX + ( sin(angle) * clockRadius ) );
    int y2 = ( clockCenterY - ( cos(angle) * clockRadius ) );
    int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 8 ) ) ) );
    display->drawLine( x2 + x , y2 + y , x3 + x , y3 + y);
  }

  // display second hand
  float angle = detik * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  int x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  int y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 5 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display minute hand
  angle = menit * 6 ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 4 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
  //
  // display hour hand
  angle = jam * 30 + int( ( menit / 12 ) * 6 )   ;
  angle = ( angle / 57.29577951 ) ; //Convert degrees to radians
  x3 = ( clockCenterX + ( sin(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  y3 = ( clockCenterY - ( cos(angle) * ( clockRadius - ( clockRadius / 2 ) ) ) );
  display->drawLine( clockCenterX + x , clockCenterY + y , x3 + x , y3 + y);
}

void digitalClockFrame(OLEDDisplay *display, OLEDDisplayUiState* state, int16_t x, int16_t y) {
  String timenow = String(jam) + ":" + twoDigits(menit) + ":" + twoDigits(detik);
  display->setTextAlignment(TEXT_ALIGN_CENTER);
  display->setFont(ArialMT_Plain_24);
  display->drawString(clockCenterX + x , clockCenterY + y, timenow );
}

FrameCallback frames[] = { analogClockFrame, digitalClockFrame };
int frameCount = 2;
OverlayCallback overlays[] = { clockOverlay };
int overlaysCount = 1;

void kedip() {
  //  digitalWrite(BZ, HIGH);
  digitalWrite(GP2, HIGH);
  delay(100);
  //  digitalWrite(BZ, LOW);
  digitalWrite(GP2, LOW);
  delay(50);
}

void setup() {
  #if ser
  Serial.begin(115200);
#endif
  pinMode(BZ, OUTPUT);
  pinMode(GP2, OUTPUT);
  pinMode(GP3, OUTPUT);
  pinMode(Mat, OUTPUT);
  pinMode(RL, OUTPUT);
  digitalWrite(GP2, LOW);
  digitalWrite(GP2, LOW);
  digitalWrite(Mat, HIGH);

  configTime(TZ, 0, "ntp.nict.jp", "ntp.jst.mfeed.ad.jp");

  ui.setTargetFPS(60);

  // Customize the active and inactive symbol
  ui.setActiveSymbol(activeSymbol);
  ui.setInactiveSymbol(inactiveSymbol);

  // You can change this to
  // TOP, LEFT, BOTTOM, RIGHT
  ui.setIndicatorPosition(TOP);

  // Defines where the first frame is located in the bar.
  ui.setIndicatorDirection(LEFT_RIGHT);

  // You can change the transition that is used
  // SLIDE_LEFT, SLIDE_RIGHT, SLIDE_UP, SLIDE_DOWN
  ui.setFrameAnimation(SLIDE_LEFT);

  // Add frames
  ui.setFrames(frames, frameCount);

  // Add overlays
  ui.setOverlays(overlays, overlaysCount);

  // Initialising the UI will init the display too.
  ui.init();

  display.flipScreenVertically();

  unsigned long secsSinceStart = millis();
  // Unix time starts on Jan 1 1970. In seconds, that's 2208988800:
  const unsigned long seventyYears = 2208988800UL;
  // subtract seventy years:
  //  unsigned long epoch = secsSinceStart - seventyYears * SECS_PER_HOUR;
  //  //  setTime(epoch);
  //  time_t t = now(); // store the current time in time variable t
  //  hour(t);          // returns the hour for the given time t
  //  minute(t);        // returns the minute for the given time t
  //  second(t);        // returns the second for the given time t
  //  day(t);           // the day for the given time t
  //  weekday(t);       // day of the week for the given time t
  //  month(t);         // the month for the given time t
  //  year(t);          // the year for the given time t
  //  setTime(t);



#if display_matrix
  P.begin();
  P.setIntensity(0);
#endif
  P.print(" CONNECT...");
  setup_();
  P.print("    OKE   ");
  //  P.print(Ethernet.localIP());

  WiFi.mode(WIFI_STA);
  WiFi.begin(ssid, password);
  Serial.println("");

  // Wait for connection
  while (WiFi.status() != WL_CONNECTED) {
    hitung++;
    kedip();
    delay(500);
    Serial.print(".");
    if (hitung > 10) {
      ESP.restart();
      hitung = 0;
    }
  }

  Serial.println("");
  Serial.print("Connected to ");
  Serial.println(ssid);
  Serial.print("IP address: ");
  Serial.println(WiFi.localIP());

  server.on("/", HTTP_GET, [](AsyncWebServerRequest * request) {
    request->send(200, "text/plain", "Hi! I am ESP32.");
  });

  AsyncElegantOTA.begin(&server);    // Start ElegantOTA
  server.begin();
  Serial.println("HTTP server started");

  if (client.connected() == 1) {
    Serial.println("lan connected");
    digitalWrite(GP3, HIGH);
  }
  if (WiFi.status() == 1) {
    Serial.println("wifi connected");
    digitalWrite(GP2, HIGH);
  }
  //  for (int i = o; i < Ethernet.localIP().length(); i++) {
  ipethernet = Ethernet.localIP().toString().c_str();
  //  }
}

void bunyi() {
  digitalWrite(BZ, HIGH);
  delay(100);
  digitalWrite(BZ, LOW);
}

void reconnect() {
  setup_();
  while (!client.connected()) {
    //    P.print(" CONNECT...");
    Serial.print("Attempting MQTT connection...");
    // Create a random client ID
    String clientId = "ESP32";
    clientId += String(random(0xffff), HEX);
    // Attempt to connect
    if (client.connect(clientId.c_str())) {
      Serial.println("connected");
      digitalWrite(GP3, HIGH);
      client.subscribe("Led2");
      //      client.subscribe("led");
    } else {
      Serial.print("failed, rc=");
      Serial.print(client.state());
      Serial.println(" try again in 5 seconds");
      // Wait 5 seconds before retrying
      hitung++;
      kedip();
      delay(1000);
      Serial.print(".");
      if (hitung > 10) {
        ESP.restart();
        hitung = 0;
      }
      setup_();
    }
  }
}

void loop() {
  time_t t;
  static time_t last_t;
  struct tm *tm;
  t = time(NULL);
  if (last_t == t) return;
  last_t = t;
  tm = localtime(&t);
  hari = ("%02d", tm->tm_wday);
  jam = ("%02d", tm->tm_hour);
  menit = ("%02d", tm->tm_min);
  detik = ("%02d", tm->tm_sec);
  sprintf(Jam, "%02d:%02d:%02d", jam, menit, detik);
  //  if (detik >= 59)tampil_hari();

  int remainingTimeBudget = ui.update();
  if (remainingTimeBudget > 0) {
    P.print(ipethernet);
    digitalWrite(GP3, HIGH);
    delay(remainingTimeBudget);
    digitalWrite(GP3, LOW);
  }
  AsyncElegantOTA.loop();
  //  P.displayZoneText(0, humString, PA_CENTER, 100, 0, PA_PRINT, PA_NO_EFFECT);
  //  P.displayAnimate();
  //  P.getZoneStatus(0);
  //  P.displayZoneText(1, tmpString, PA_CENTER, 100, 0, PA_PRINT, PA_NO_EFFECT);
  //  P.displayAnimate();
  //  P.getZoneStatus(1);
  if (!client.connected()) {
    digitalWrite(GP2, LOW);
    reconnect();
  } else {
    digitalWrite(GP2, HIGH);
  }
  client.loop();

  unsigned long now = millis();
  if (now - lastMsg > 5000) {
    char tmpString[8];
    char humString[8];

    lastMsg = now;
    value = random(1.0, 100.0);
    tmp = random(10, 100.0);
    dtostrf(tmp, 1, 1, tmpString);
    dtostrf(value, 1, 1, humString);

    //    snprintf (msg, MSG_BUFFER_SIZE, "hello world #%ld", value);
    //    Serial.print("Publish message: ");
    //    Serial.println(msg);
    String Ser = + "TEMP: " + String(tmpString) + "," + "HUM: " + String(humString) + "";
    String Ser2 = + "  " + String(tmpString) + "  " + String(humString) + "";

    Serial.print("Send: ");
    Serial.println(Ser);
    Ser.toCharArray(msg, 50);
    String led = String(val_led);
    led.toCharArray(Led2, 2);


    if (!client.connected()) {
      reconnect();
    } else {
      P.print(Ser2);
      client.publish("Ongki", msg);
      if (oke) {
        client.publish("out/led", Led2);
        oke = 0;
        //        Serial.print("oke: ");
        //        Serial.println(val_led);
        //        Serial.println(Led2);
      }
      client.publish("Ongki/tmp", tmpString);
      client.publish("Ongki/hum", humString);

    }
  }
}
