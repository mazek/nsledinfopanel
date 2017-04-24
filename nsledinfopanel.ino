
#include <ArduinoJson.h>
#include <ArduinoHttpClient.h>
#include <ESP8266WiFi.h>
#include <WiFiClient.h>
#include <FastLED.h>
#include <LEDMatrix.h>

#include "memorysaver.h"
#if !(defined ESP8266 )
#error Please select the ArduCAM ESP8266 UNO board in the Tools/Board
#endif

//Station mode you should put your ssid and password
const char *ssid = "southofheaven"; // Put your SSID here
const char *password = "papaja29"; // Put your PASSWORD here

char serverAddress[] = "ns.rajewski.pl";  // server address
int port = 80;

int min = 80;
int max = 160;

WiFiClient wifi;
HttpClient client = HttpClient(wifi, serverAddress, port);


String response;
char response2[200];

int statusCode = 0;
//StaticJsonBuffer<3000> jsonBuffer;
DynamicJsonBuffer  jsonBuffer;

// LED panel initialisation START

#define LED_PIN        6
#define COLOR_ORDER    GRB
#define CHIPSET        WS2812B

#define MATRIX_WIDTH   32
#define MATRIX_HEIGHT  8
#define MATRIX_TYPE    VERTICAL_ZIGZAG_MATRIX

cLEDMatrix<MATRIX_WIDTH, MATRIX_HEIGHT, MATRIX_TYPE> leds;

// LED panel initialisation END.

void setup() {
  Serial.begin(115200);
  Serial.println("ArduCAM Start!");
  // put your setup code here, to run once:
   if(!strcmp(ssid,"SSID")){
       Serial.println("Please set your SSID");
       while(1);
    }
    if(!strcmp(password,"PASSWORD")){
       Serial.println("Please set your PASSWORD");
       while(1);
    }
    // Connect to WiFi network
    Serial.println();
    Serial.println();
    Serial.print("Connecting to ");
    Serial.println(ssid);
    
    WiFi.mode(WIFI_STA);
    WiFi.begin(ssid, password);
    while (WiFi.status() != WL_CONNECTED) {
      delay(500);
      Serial.print(".");
    }
    Serial.println("WiFi connected");
    Serial.println("");
    Serial.println(WiFi.localIP());

    FastLED.addLeds<CHIPSET, LED_PIN, COLOR_ORDER>(leds[0], leds.Size());
    FastLED.setBrightness(2);
    FastLED.clear(true);
    delay(500);
    
}

void loop() {
  
  Serial.println("making GET request");
  client.get("/pebble");

  // read the status code and body of the response
  statusCode = client.responseStatusCode();
  response = client.responseBody();
  
//  char response2[response.length()+1];
  
//  Serial.print("Status code: ");
//  Serial.println(statusCode);
//  Serial.print("Response: ");
//  Serial.println(response);
  response.remove(0, 3);
  response = response.substring(0, response.length()-5);
  response.toCharArray(response2,response.length()+1);
  
  Serial.print("Response2: -");
  Serial.print(response2);
  Serial.println("-");
    
  JsonObject& _data = jsonBuffer.parseObject(response2);

  if (!_data.success()) {
    Serial.println("parseObject() failed:( ");
    FastLED.showColor(CRGB::Pink);
    delay(5000);
    return;
  }
  else {
    Serial.println("parseObject() success! ");
  }
 
  String cur_time_s = _data["status"][0]["now"];
  String read_time_s = _data["bgs"][0]["datetime"];

  Serial.print("1 cur_time_s: ");
  Serial.println(cur_time_s);
  
  Serial.print("1 read_time_s: ");
  Serial.println(read_time_s);
  
  cur_time_s = cur_time_s.substring(0, cur_time_s.length()-3);
  read_time_s = read_time_s.substring(0, read_time_s.length()-3);

  Serial.print("2 cur_time_s: ");
  Serial.println(cur_time_s);
  
  Serial.print("2 read_time_s: ");
  Serial.println(read_time_s);
  
  unsigned long  cur_time = cur_time_s.toFloat();
  unsigned long  read_time = read_time_s.toFloat();
  
  Serial.print("cur_time: ");
  Serial.println(cur_time);
  
  Serial.print("read_time: ");
  Serial.println(read_time);

  unsigned long parakeet_last_seen = cur_time - read_time ;
  Serial.print("I seen parakeet more then ");
  Serial.print(parakeet_last_seen);
  Serial.println(" seconds.");
//  FastLED.clear(true);  
  if (parakeet_last_seen > 900) {
    // Lost parakeet signal after 900 seconds.
    
//   lost_time = strftime("%d %b %Y %H:%M",localtime(read_time/1000))
//  notify(title    = 'Nightscout read.',
//          subtitle = strftime("%d %b %H:%M",localtime(cur_time/1000)),
//          message  = 'I\'ve lost parakeet signal at: %s' % (lost_time))
    Serial.println("I lost parakeet signal :(");
    FastLED.showColor(CRGB::Lime);
  }
 else {
    // Parakeet operational.

    Serial.println("I got parakeet signal :)");
    unsigned long bwpo = _data["bgs"][0]["sgv"];
    Serial.print("BWPO: ");
    Serial.println(bwpo);
    long bgdelta = _data["bgs"][0]["bgdelta"];
    Serial.print("BGDELTA: ");
    Serial.println(bgdelta);

    
    if (bgdelta > 0) {
      // Sugar is growing.
//      bgdelta_s = "+%s" % bgdelta
    }
    else {
      // Sugar is dropping.
//      bgdelta_s = "%s" % bgdelta
    }
    
    if (bwpo < min) {
      // Sugar below minimum level.
 //     notify(title    = 'Nightscout read.',
 //            subtitle = '',
 //            message  = 'Sugar: %s, change: %s' % (bwpo, bgdelta_s))
      Serial.print("Sugar: ");
      Serial.print(bwpo);
      Serial.print(", change:  ");
      Serial.println(bgdelta);
      Serial.println("Sugar below minimum level.");
      FastLED.showColor(CRGB::Red);  
    }
    else if (bwpo > max) {
      // Sugar above maximum level.
      Serial.print("Sugar: ");
      Serial.print(bwpo);
      Serial.print(", change:  ");
      Serial.println(bgdelta);
      Serial.println("Sugar above maximum level.");
      FastLED.showColor(CRGB::Blue);  
    }
    else {
      // Sugar ok.
      FastLED.clear(true); 
      //FastLED.showColor(CRGB::Green);   
    }

  }

  Serial.println("Wait five seconds");
  delay(5000);
//  FastLED.show();

}
