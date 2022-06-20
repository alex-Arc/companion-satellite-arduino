#include <Arduino.h>
#include <Bounce2.h>

// INSTANTIATE A Bounce OBJECT
Bounce bounce = Bounce();

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

  bounce.attach(34, INPUT);
  bounce.interval(5);
}

void loop()
{

  int n = client.available();
  if (n)
  {
    long int t1 = micros();

    buff = (char *)malloc(n + 1);
    client.read((uint8_t *)buff, n);
    // Serial.printf("%.*s \n", n, buff);

    compSat._handleReceivedData(buff);

    long int t2 = micros();
    Serial.printf("exec time: %d us\n", t2 - t1);
  }

  if (!compSat.transmitBuffer.empty())
  {
    Serial.printf("TX: >%s<\n", compSat.transmitBuffer.data());
    client.write(compSat.transmitBuffer.data());
    compSat.transmitBuffer.clear();
  }

  if (!compSat.drawQueue.empty())
  {
    while (!compSat.drawQueue.empty())
    {
      Serial.printf("draw id: %s index %d color %s image %s text %s\n", compSat.drawQueue.front().deviceId.data(), compSat.drawQueue.front().keyIndex, compSat.drawQueue.front().color.data(), compSat.drawQueue.front().image.data(), compSat.drawQueue.front().text.data());
      compSat.drawQueue.pop_front();
    }
  }

  bounce.update();
  if (bounce.changed())
  {
    int deboucedInput = bounce.read();

    if (deboucedInput == LOW)
    {
      compSat.keyDown("1234", 0);
    }
  }
}