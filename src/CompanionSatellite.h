#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <string>
#include <vector>
// #include <array>
// #include <algorithm>
// #include <iterator>
// #include <map>

class CompanionSatellite
{
private:
    unsigned long _lastReceivedAt;
    std::string receiveBuffer;

    struct parm
    {
        std::string key;
        std::string val;
    };

    std::vector<parm> parseLineParameters(std::string line);
    void handleCommand(std::string line);

public:
    void _handleReceivedData(char *data);
    bool connected();
};

std::vector<CompanionSatellite::parm> CompanionSatellite::parseLineParameters(std::string line)
{
    // https://newbedev.com/javascript-split-string-by-space-but-ignore-space-in-quotes-notice-not-to-split-by-the-colon-too
    // const match = line.match(/\\?.|^$/g);

    // if (size_t quote = line.find_first_of('"'); quote != std::string::npos)
    // {
    // TODO: quote parseing
    /*
    const fragments = match
        ? match.reduce(
                (p, c) => {
                    if (c === '"') {
                        p.quote ^= 1
                    } else if (!p.quote && c === ' ') {
                        p.a.push('')
                    } else {
                        p.a[p.a.length - 1] += c.replace(/\\(.)/, '$1')
                    }
                    return p
                },
                { a: [''], quote: 0 }
        ).a
        : []
        */
    // }

    std::vector<std::string> fragments;
    size_t offset = 0;
    for (size_t space = line.find_first_of(' ', offset); space != std::string::npos; space = line.find_first_of(' ', offset))
    {
        fragments.push_back(line.substr(offset, space - offset));
        offset = space + 1;
    }
    fragments.push_back(line.substr(offset));

    std::vector<parm> res;
    for (auto fragment : fragments)
    {
        parm p;
        if (size_t equals = fragment.find_first_of('='); equals != std::string::npos)
        {
            p.key = fragment.substr(0, equals);
            p.val = fragment.substr(equals+1);
            res.push_back(p);
        }
        else
        {
            p.key = fragment.substr(0, equals);
            p.val = "true";
            res.push_back(p);
        }

        Serial.printf("KEY: >%s< VAL: >%s<\n", p.key.data(), p.val.data());
    }

    // for (const fragment of fragments)
    // {
    //     const[key, value] = fragment.split('=')
    //                             res[key] = value == = undefined ? true : value
    // }
    return res;
}

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
        this->handleCommand(line); // TODO: remove potential \r
    }
    this->receiveBuffer.erase(0, offset);
}

void CompanionSatellite::handleCommand(std::string line)
{
    size_t i = line.find_first_of(' ');
    std::string cmd = (i == std::string::npos) ? line : line.substr(0, i);
    std::string body = (i == std::string::npos) ? "" : line.substr(i + 1);

    Serial.printf("CMD: >%s<\tBODY: >%s<\n", cmd.data(), body.data());

    parseLineParameters(body);
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