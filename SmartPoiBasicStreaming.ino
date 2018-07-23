/*Basic Streaming Smart Poi version
 * Receives UDP RGB packets and displays them on LED strip 
 * Many thanks to Arduino, Processing teams
 * as well as Brett and Dylan Pillemer for inspiration 
 * All code is written or adapted by Tom Hastings
 * www.circusscientist.com
 * Only works in combination with SmartPoiBasicStreaming_UDP_Send Processing app. 
 * 
 * Arduino ide settings: 
 * Wemos D1 R2 & Mini
 * lWip v2.0 lower memory
 * 4m, 1m Spiffs
 * 80mhz
 * 
 * 
 */


#include <ESP8266WiFi.h>
#include <DNSServer.h>
#include <WiFiUdp.h>



//////////////////////////////////////////FastLED code:////////////
#include "FastLED.h"
// How many leds in your strip?
#define NUM_LEDS 37
//#define NUM_LEDS 73

#define NUM_PX 36
//#define NUM_PX 72

int newBrightness = 1; //setting 220 for battery and so white is not too much! //20 for testing ok

#define DATA_PIN D2 //D2 for D1Mini, 2 for ESP-01
#define CLOCK_PIN D1 //D1 for D1Mini, 0 for ESP-01

// Define the array of leds
CRGB leds[NUM_LEDS];
///////////////////////////////////////////end FastLED code////////////////

////////////////////////////////////////////////////Mostly networking stuff: ////////////////////////////////////////////
const byte DNS_PORT = 53;
IPAddress apIP(192, 168, 1, 1);
IPAddress apIPSlave(192, 168, 1, 78);
DNSServer dnsServer;

int status = WL_IDLE_STATUS;
char apName[] = "Smart_Poi_2"; 
char apPass[] = "bluefire"; 
int apChannel = 1;

const unsigned int localPort = 2390;      // local port to listen on


byte packetBuffer[NUM_PX]; //buffer to hold incoming packet


WiFiUDP Udp;

void setup() {
//  Serial.begin(115200);
  FastLED.addLeds<APA102, DATA_PIN, CLOCK_PIN, BGR>(leds, NUM_LEDS); //DATA_RATE_MHZ(8)
  //  FastLED.addLeds<NEOPIXEL, DATA_PIN>(leds, NUM_LEDS); //not ideal!
  FastLED.setBrightness(  newBrightness ); //should be low figure here, for startup battery saving...
  FastLED.showColor( CRGB::Black );
  WiFi.mode(WIFI_AP);
  WiFi.softAPConfig(apIP, apIP, IPAddress(255, 255, 255, 0));
  WiFi.softAP(apName, apPass, apChannel); //use pre-set values here
  dnsServer.start(DNS_PORT, "*", apIP); //AP mode only
  Udp.begin(localPort);
  fastLEDIndicate();
  
}

void loop() {
  dnsServer.processNextRequest();
  int packetSize = Udp.parsePacket();
  if (packetSize) // if udp packet is received:
  {
    int len = Udp.read(packetBuffer, NUM_PX);
    if (len > 0) packetBuffer[len] = 0;
    //    //Serial.println("Contents:");
    //    //Serial.println(packetBuffer);
    ////////////////////////////////////FastLED Code://///////////
    for (int i = 0; i < NUM_PX; i++)
    {
      byte X;
      byte Y;

      ///////////////////////////////convert byte to signed byte:////
      X = packetBuffer[i] - 127;
      /////////////////////////////end convert byte to signed byte////

      byte R1 = (X & 0xE0);
      leds[i].r = R1; //
      byte G1 =  ((X << 3) & 0xE0);
      leds[i].g = G1;
      byte M1 = (X << 6);
      leds[i].b = M1;
      //FastLED.delay(1);
      //        //Serial.print(R1); //Serial.print(", "); //Serial.print(G1); //Serial.print(", "); //Serial.println(M1);
    }

    //FastLED.delay(2); //not just for emulator!
    LEDS.show();
    delay(1);
    ///////////////////////////////////end FastLed Code//////////////

    // send a reply, to the IP address and port that sent us the packet we received
    //    Udp.beginPacket(Udp.remoteIP(), Udp.remotePort());
    //    Udp.write(ReplyBuffer);
    //    Udp.endPacket();
  } else {
    //    //Serial.println("/");
    //nothing for <interval> seconds wait for signal
  }
}


//for startup:
void fastLEDIndicate(){
   //indicate wifi mode:
//  //Serial.println("FASTLED NOW");

    for (int i = 0; i < NUM_LEDS; i++) {
      // Set the i'th led to whatever
      leds[i] = CRGB::Magenta;
      // Show the leds
      FastLED.show();
      // now that we've shown the leds, reset the i'th led to black
      leds[i] = CRGB::Black;
      // Wait a little bit before we loop around and do it again
      delay(10);
    }
    for (int i = 0; i < NUM_LEDS; i++) {
      // Set the i'th led to whatever
      leds[i] = CRGB::Yellow;
      // Show the leds
      FastLED.show();
      // now that we've shown the leds, reset the i'th led to black
      leds[i] = CRGB::Black;
      // Wait a little bit before we loop around and do it again
      delay(10);
    }
    for (int i = 0; i < NUM_LEDS; i++) {
      // Set the i'th led to whatever
      leds[i] = CRGB::Cyan;
      // Show the leds
      FastLED.show();
      // now that we've shown the leds, reset the i'th led to black
      leds[i] = CRGB::Red;
      // Wait a little bit before we loop around and do it again
      delay(10);
    }

    delay(10);


 

  FastLED.showColor( CRGB::Black );
}


