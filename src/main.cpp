#include <Arduino.h>
#include <NativeEthernet.h>
#define Use_NativeEthernet
// #define Use_QNEthernet
// #define Use_Ethernet
#include <CompanionSatellite.h>

EthernetClient client;
CompanionSatellite compSat(&client);

byte mac[] = {0xDE, 0xAD, 0xBE, 0xEF, 0xFE, 0xED};
IPAddress ip(192, 168, 0, 177);
IPAddress myDns(192, 168, 0, 1);

void setup()
{
    // Open serial communications and wait for port to open:
    Serial.begin(9600);
    while (!Serial)
    {
        ; // wait for serial port to connect. Needed for native USB port only
    }

    // try to congifure using IP address instead of DHCP:
    Ethernet.begin(mac, ip, myDns);

    // give the Ethernet shield a second to initialize:
    delay(1000);

    compSat.connect(IPAddress(192, 168, 0, 2));
}

// comp.connect(IPAddress(192,168,0,2));
void loop()
{
    compSat.maintain();
    int len = client.available();
    if (len > 0)
    {
        byte buffer[80];
        if (len > 80)
            len = 80;

        client.read(buffer, len);
        Serial.write(buffer, len); // show in the serial monitor (slows some boards)
    }
    delay(10);
}