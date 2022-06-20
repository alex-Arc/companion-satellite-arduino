#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <string>
#include <vector>
// #include <array>
#include <algorithm>
// #include <iterator>
// #include <map>

class CompanionSatellite
{
private:
    struct DeviceDrawProps
    {
        std::string deviceId = "1234";
        int keyIndex = 2;
        bool image = false;
        bool color = false;
        bool text = false;
    };

    struct DeviceRegisterProps
    {
        int keysTotal = 2;
        int keysPerRow = 8;
        bool bitmaps = false;
        bool colours = false;
        bool text = false;
    };

    DeviceRegisterProps prop;

    unsigned long _lastReceivedAt;
    std::string receiveBuffer;

    struct parm
    {
        std::string key;
        std::string val;
    };

    std::vector<parm> parseLineParameters(std::string line);
    void handleCommand(std::string line);

    const std::vector<std::string> commandList = {
        "PING",
        "PONG",
        "KEY-STATE",
        "KEYS-CLEAR",
        "BRIGHTNESS",
        "ADD-DEVICE",
        "REMOVE-DEVICE",
        "BEGIN",
        "KEY-PRESS"};

    void addDevice(std::string deviceId, std::string productName, CompanionSatellite::DeviceRegisterProps props);
    void handleAddedDevice(std::vector<parm> params);

public:
    bool _connected = false;
    std::string transmitBuffer;

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
            p.val = fragment.substr(equals + 1);
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
    return res;
}

void CompanionSatellite::_handleReceivedData(char *data)
{
    this->_lastReceivedAt = millis();
    this->receiveBuffer += std::string(data);
    // Serial.printf("data >%s<\n", this->receiveBuffer.data());

    size_t i;
    int offset = 0;
    while ((i = this->receiveBuffer.find_first_of('\n', offset)) != std::string::npos)
    {
        std::string line = this->receiveBuffer.substr(offset, i - offset);
        offset = i + 1;
        // Serial.printf("LINE >%s<\n", line.data());
        this->handleCommand(line); // TODO: remove potential \r
    }
    this->receiveBuffer.erase(0, offset);
}

void CompanionSatellite::handleCommand(std::string line)
{
    size_t i = line.find_first_of(' ');
    std::string cmd = (i == std::string::npos) ? line : line.substr(0, i);
    std::string body = (i == std::string::npos) ? "" : line.substr(i + 1);

    // Serial.printf("CMD: >%s<\tBODY: >%s<\n", cmd.data(), body.data());

    std::vector<parm> params = parseLineParameters(body);

    auto cmd_index = std::find(commandList.begin(), commandList.end(), cmd);

    switch (distance(commandList.begin(), cmd_index))
    {
    case 0: // PING
        Serial.printf("PING device: %s\n", body.data());
        // this.socket?.write(`PONG ${body}\n`)
        break;
    case 1: //'PONG':
        // console.log('Got pong')
        Serial.printf("PONG device: %s\n", body.data());
        // this._pingUnackedCount = 0
        break;
    case 2: //'KEY-STATE':
        Serial.printf("KEY-STATE device: %s\n", body.data());
        // this.handleState(params)
        break;
    case 3: //'KEYS-CLEAR':
        Serial.printf("KEYS-CLEAR device: %s\n", body.data());
        // this.handleClear(params)
        break;
    case 4: //'BRIGHTNESS':
        Serial.printf("BRIGHTNESS device: %s\n", body.data());
        // this.handleBrightness(params)
        break;
    case 5: //'ADD-DEVICE':
        Serial.printf("ADD-DEVICE: %s\n", body.data());
        // this.handleAddedDevice(params)
        break;
    case 6: //'REMOVE-DEVICE':
        Serial.printf("REMOVE-DEVICE: %s\n", body.data());

        break;
    case 7: //'BEGIN':
        Serial.printf("Connected to Companion: %s\n", body.data());
        this->transmitBuffer.clear();
        this->_connected = true;
        this->addDevice("1234", "ESP32 test", this->prop);
        break;
    case 8: //'KEY-PRESS':
        Serial.printf("KEY-PRESS: %s\n", body.data());
        // Ignore
        break;
    default:
        Serial.printf("Received unhandled command: %s %s\n", cmd.data(), body.data());
        // console.log(`Received unhandled command: ${cmd} ${body}`)
        break;
    }
}

void CompanionSatellite::addDevice(std::string deviceId, std::string productName, CompanionSatellite::DeviceRegisterProps props)
{
    if (this->_connected)
    {
        this->transmitBuffer.append(
            "ADD-DEVICE DEVICEID=" + deviceId +
            " PRODUCT_NAME=\"" + productName +
            "\" KEYS_TOTAL=" + std::to_string(props.keysTotal) +
            " KEYS_PER_ROW=" + std::to_string(props.keysPerRow) +
            " BITMAPS=" + ((props.bitmaps) ? "1" : "0") +
            " COLORS=" + ((props.colours) ? "1" : "0") +
            " TEXT=" + ((props.text) ? "1" : "0") + "\n");
    }
}

void CompanionSatellite::handleAddedDevice(std::vector<parm> params)
{
    if (params[0].key != "OK" || params[0].key == "ERROR")
    {
        if (params[1].key == "MESSAGE")
        {
            Serial.printf("Add device failed: %s\n", params[1].val.data());
        }
        else
        {
            Serial.printf("Add device failed: no message\n");
        }
        // if (typeof params.DEVICEID === 'string') {
        // 	this.emit('deviceErrored', {
        // 		deviceId: params.DEVICEID,
        // 		message: `${params.MESSAGE || 'Unknown Error'}`,
        // 	})
        // }
        return;
    }
    if (typeof params.DEVICEID != = 'string')
    {
        console.log('Mising DEVICEID in ADD-DEVICE response') return;
    }

    // this.emit('newDevice', { deviceId: params.DEVICEID })
}

bool CompanionSatellite::connected()
{
    return false;
}

#endif