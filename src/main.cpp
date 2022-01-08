#include <Arduino.h>
#include <NativeEthernet.h>
// #include <FastLED.h>
#include <Adafruit_NeoPixel.h>
#define Use_NativeEthernet
// #define Use_QNEthernet
// #define Use_Ethernet
#include <CompanionSatellite.h>
#include <Bounce.h>

EthernetClient client;
CompanionSatellite compSat(&client);

const uint8_t NUM_BTNS = 8;

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

Adafruit_NeoPixel leds = Adafruit_NeoPixel(NUM_BTNS, 39, NEO_GRB + NEO_KHZ800);

// Fill the dots one after the other with a color
void colorWipe(uint32_t c, uint8_t wait)
{
  for (uint16_t i = 0; i < leds.numPixels(); i++)
  {
    leds.setPixelColor(i, c);
    leds.show();
    delay(wait);
  }
}

int buttons[] = {0, 1, 2, 3, 4, 5, 6, 9};

Bounce b_btn[NUM_BTNS] = {
    Bounce(buttons[0], 5),
    Bounce(buttons[1], 5),
    Bounce(buttons[2], 5),
    Bounce(buttons[3], 5),
    Bounce(buttons[4], 5),
    Bounce(buttons[5], 5),
    Bounce(buttons[6], 5),
    Bounce(buttons[7], 5)};

bool buttonState[NUM_BTNS] = {0, 0, 0, 0, 0, 0, 0, 0};

bool change = false;

void setup()
{
  // Open serial communications and wait for port to open:
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }
  if (CrashReport)
  {
    Serial.print(CrashReport);
  }

  leds.begin();
  leds.setBrightness(255);
  leds.show();                           // Initialize all pixels to 'off'
  colorWipe(leds.Color(255, 0, 0), 100); // black
  colorWipe(leds.Color(0, 255, 0), 100); // black
  colorWipe(leds.Color(0, 0, 255), 100); // black

  for (int i = 0; i < NUM_BTNS; i++)
  {
    pinMode(buttons[i], INPUT_PULLUP);
  }

  // try to congifure using IP address instead of DHCP:
  // Ethernet.begin(mac); //, ip, myDns);
  // start the Ethernet connection:
  Serial.println("Initialize Ethernet with DHCP:");
  if (Ethernet.begin(mac) == 0)
  {
    Serial.println("Failed to configure Ethernet using DHCP");
    // Check for Ethernet hardware present
    if (Ethernet.hardwareStatus() == EthernetNoHardware)
    {
      Serial.println("Ethernet shield was not found.  Sorry, can't run without hardware. :(");
      while (true)
      {
        delay(1); // do nothing, no point running without Ethernet hardware
      }
    }
    if (Ethernet.linkStatus() == LinkOFF)
    {
      Serial.println("Ethernet cable is not connected.");
    }
    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);
  }
  else
  {
    Serial.print("  DHCP assigned IP ");
    Serial.println(Ethernet.localIP());
  }

  // give the Ethernet shield a second to initialize:
  delay(1000);

  compSat.connect(IPAddress(172, 16, 3, 48));

  colorWipe(leds.Color(0, 0, 0), 10); // black
  leds.setBrightness(255);
}

// comp.connect(IPAddress(192,168,0,2));
void loop()
{

  compSat.maintain();

  for (int i = 0; i < NUM_BTNS; i++)
  { // button bounce update and state
    b_btn[i].update();
    if (b_btn[i].fallingEdge())
    {
      compSat.keyPress(i, 1);
    }
    else if (b_btn[i].risingEdge())
    {
      compSat.keyPress(i, 0);
    }
  }
}