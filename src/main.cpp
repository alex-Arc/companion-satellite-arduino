#include <Arduino.h>
#include <CompanionSatellite.h>
CompanionSatellite compSat;

char *buff;

#define ETH_CLK_MODE ETH_CLOCK_GPIO17_OUT
#define ETH_PHY_POWER 12

#include <ETH.h>

static bool eth_connected = false;
WiFiClient client;

const uint8_t NUM_BTNS = 8;

IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

void WiFiEvent(WiFiEvent_t event)
{
  switch (event)
  {
  case SYSTEM_EVENT_ETH_START:
    Serial.println("ETH Started");
    // set eth hostname here
    ETH.setHostname("esp32-ethernet");
    break;
  case SYSTEM_EVENT_ETH_CONNECTED:
    Serial.println("ETH Connected");
    break;
  case SYSTEM_EVENT_ETH_GOT_IP:
    Serial.print("ETH MAC: ");
    Serial.print(ETH.macAddress());
    Serial.print(", IPv4: ");
    Serial.print(ETH.localIP());
    if (ETH.fullDuplex())
    {
      Serial.print(", FULL_DUPLEX");
    }
    Serial.print(", ");
    Serial.print(ETH.linkSpeed());
    Serial.println("Mbps");
    eth_connected = true;
    break;
  case SYSTEM_EVENT_ETH_DISCONNECTED:
    Serial.println("ETH Disconnected");
    eth_connected = false;
    break;
  case SYSTEM_EVENT_ETH_STOP:
    Serial.println("ETH Stopped");
    eth_connected = false;
    break;
  default:
    break;
  }
}

void setup()
{
  Serial.begin(9600);
  while (!Serial)
  {
    ; // wait for serial port to connect. Needed for native USB port only
  }

  WiFi.onEvent(WiFiEvent);
  ETH.begin();
  if (!ETH.config(IPAddress(192, 168, 0, 177), IPAddress(192, 168, 0, 1), IPAddress(255, 255, 255, 0)))
  {
    Serial.println("ETH: Error with setting static IP");
  }

  Serial.print("\nconnecting to ");
  Serial.println(IPAddress(192, 168, 0, 10));

  client.connect(IPAddress(192, 168, 0, 10), 16622);
  delay(5);
}

void loop()
{

  int n = client.available();
  if (n)
  {
    buff = (char *)malloc(n + 1);
    client.read((uint8_t *)buff, n);
    // Serial.printf("%.*s \n", n, buff);


  }

  delay(10);
}