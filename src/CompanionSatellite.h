#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <string>
// #include <array>
// #include <algorithm>
// #include <iterator>
// #include <map>

class CompanionSatellite
{
private:
    unsigned long _lastReceivedAt;
    std::string receiveBuffer;

public:
    void _handleReceivedData(char *data);
    bool connected();
};

void CompanionSatellite::_handleReceivedData(char *data)
{
    this->_lastReceivedAt = millis();
    this->receiveBuffer += std::string(data);
    Serial.printf("data >%s<\n", this->receiveBuffer.data());

    size_t i;
    int offset = 0;
    while ((i = this->receiveBuffer.find_first_of('\n', offset)) != std::string::npos)
    {
        std::string line = this->receiveBuffer.substr(offset, i - offset);
        offset = i + 1;
        Serial.printf("LINE >%s<\n", line.data());
        // this->handleCommand(line.toString().replace(/\r /, ''));
    }
    this->receiveBuffer.erase(0, offset);
}

bool CompanionSatellite::connected()
{
    return false;
}

#endif