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

    void handleCommand(std::string line);
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
        this->handleCommand(line); //TODO: remove potential \r
    }
    this->receiveBuffer.erase(0, offset);
}

void CompanionSatellite::handleCommand(std::string line) {
		size_t i = line.find_first_of(' ');
		std::string cmd = (i == std::string::npos) ? line : line.substr(0, i-1);
		std::string body = (i == std::string::npos) ? "" : line.substr(i + 1);

        Serial.printf("CMD: >%s<\tBODY: >%s<\n", cmd.data(), body.data());
		
        /*
        const params = parseLineParameters(body)

		switch (cmd.toUpperCase()) {
			case 'PING':
				this.socket?.write(`PONG ${body}\n`)
				break
			case 'PONG':
				// console.log('Got pong')
				this._pingUnackedCount = 0
				break
			case 'KEY-STATE':
				this.handleState(params)
				break
			case 'KEYS-CLEAR':
				this.handleClear(params)
				break
			case 'BRIGHTNESS':
				this.handleBrightness(params)
				break
			case 'ADD-DEVICE':
				this.handleAddedDevice(params)
				break
			case 'REMOVE-DEVICE':
				console.log('Removed device: ${body}')
				break
			case 'BEGIN':
				console.log(`Connected to Companion: ${body}`)
				break
			case 'KEY-PRESS':
				// Ignore
				break
			default:
				console.log(`Received unhandled command: ${cmd} ${body}`)
				break
		}
        */
	}

bool CompanionSatellite::connected()
{
    return false;
}

#endif