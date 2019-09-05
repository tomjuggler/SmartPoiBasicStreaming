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

//more send to processing stuff:
#define UPDATES_PER_SECOND 30000
boolean emulated = true;
//
/****Variables needed for sending to Processing. */
uint16_t sendDelay = 10;    // [Milliseconds] To slow stuff down if needed.
boolean testing = false;  // Default is false. [Ok to change for testing.]
// Can temporarily change testing to true to check output in serial monitor.
// Must set back to false to allow Processing to connect and receive data.

boolean linkedUp = true;  // Initially set linkup status false. [Do Not change.]
/****End of variables needed for sending Processing. */

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
  /****Stuff needed in setup() section for sending to Processing. */
  Serial.begin(115200);  // Allows serial output.
  if (emulated) {
    //  while (!Serial) {
    //    ;  // Wait for serial connection. Only needed for Leonardo board.
    //  }
    firstContact();  // Connect with Processing. Hello, is anyone out there?
    /****End of stuff to put in your setup() section for Processing. */
  }
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
    FastLED.delay(1);
//    delay(1);
    ///////////////////////////////////end FastLed Code//////////////
if (emulated) {
    SendToProcessing();
//    delay(1);
    FastLED.delay(1000 / UPDATES_PER_SECOND);
  }
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


/****The below two functions are needed for sending to Processing. */

// Waits for Processing to respond and then sends the number of pixels.
void firstContact() {
  uint16_t nLEDS = NUM_LEDS;  // Number to send to Processing.  (Allows up to 65,535 pixels)
  if (testing == true) {
    linkedUp = true;  // If testing, pretend we are already connected to Processing.
    Serial.print("NUM_LEDS: "); Serial.println(nLEDS);  // Number of pixels in our LED strip.
    Serial.print("  High Byte = "); Serial.print(highByte(nLEDS));  // The high byte.
    Serial.print(" x 256 = "); Serial.println(highByte(nLEDS) * 256);
    Serial.print("  Low Byte  = "); Serial.println(lowByte(nLEDS));  // The low byte.
    delay(3000);
  }
  else {
    while (Serial.available() <= 0) {  // Repeats until Processing responds. Hello?
      Serial.print('A');  // send an ASCII A (byte of value 65)
      delay(100);
    }
    // Once Processing responds send the number of pixels as two bytes.
    Serial.write(highByte(nLEDS));  // Send the high byte to Processing.
    Serial.write(lowByte(nLEDS));  // Send the low byte to Processing.
    Serial.print('#');  // send an ASCII # (byte of value 35) as a flag for Processing.
    linkedUp = true;  // Now that Processing knows number of pixels set linkedUp to true.
    delay(500);
  }
}

//--------------------
// This function sends ALL the pixel data to Processing.
void SendToProcessing() {
  if (testing == true) { // Print pixel data. If NUM_LEDS is large this will be a lot of data!
    Serial.println("-------------------------------------------------------");
    for (uint16_t d = 0; d < NUM_LEDS; d++) {
      Serial.print("p: "); Serial.print(d);
      Serial.print("\tr: "); Serial.print(leds[d].g);
      Serial.print("\tg: "); Serial.print(leds[d].b);
      Serial.print("\tb: "); Serial.println(leds[d].r);
    }
    Serial.println(" ");
    delay(500);  // Add some extra delay while testing.
  }
  else {  // Send ALL the pixel data to Processing!
    for (uint16_t d = 0; d < NUM_LEDS; d++) {
      Serial.write(d);          // Pixel number
      Serial.write(leds[d].g);  // Red channel data
      Serial.write(leds[d].b);  // Green channel data
      Serial.write(leds[d].r);  // Blue channel data
    }
    delay(sendDelay);  // Delay to slow things down if needed.
  }
}

//--------------------
/****End of the functions needed for sending to Processing.*/
