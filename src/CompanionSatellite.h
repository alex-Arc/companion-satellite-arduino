#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <string>
#include <vector>
#include <list>
#include <algorithm>
// #include <iterator>
// #include <map>

#include <string_view>
#include <charconv>

#include <B64.h>

class CompanionSatellite
{
private:
    struct DeviceDrawProps
    {
        std::string deviceId;
        int keyIndex;
        std::string image;
        std::string color;
        std::string text;
    };

    struct DeviceRegisterProps
    {
        int keysTotal = 2;
        int keysPerRow = 2;
        bool bitmaps = false;
        bool color = false;
        bool text = true;
    };

    DeviceRegisterProps prop;

    unsigned long _lastReceivedAt;
    std::string_view receiveBuffer;

    struct parm
    {
        std::string_view key;
        std::string_view val;
    };

    std::vector<parm> parseLineParameters(std::string_view line);
    void handleCommand(std::string_view line);

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
    void handleState(std::vector<parm> params);
    void handleBrightness(std::vector<parm> params);

public:
    bool _connected = false;
    bool _deviceStatus = false;
    std::string transmitBuffer;
    std::list<DeviceDrawProps> drawQueue;

    void _handleReceivedData(char *data);
    bool connected();

    void keyDown(std::string deviceId, int keyIndex);
    void keyUp(std::string deviceId, int keyIndex);
};

std::vector<CompanionSatellite::parm> CompanionSatellite::parseLineParameters(std::string_view line)
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

    std::vector<std::string_view> fragments;
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

        // Serial.printf("KEY: >%s< VAL: >%s<\n", p.key.data(), p.val.data());
    }
    return res;
}

void CompanionSatellite::_handleReceivedData(char *data)
{
    this->_lastReceivedAt = millis();
    this->receiveBuffer = std::string_view(data);
    // Serial.printf("data >%s<\n", this->receiveBuffer.data());

    size_t i;
    int offset = 0;
    while ((i = this->receiveBuffer.find_first_of('\n', offset)) != std::string::npos)
    {
        std::string_view line = this->receiveBuffer.substr(offset, i - offset);
        offset = i + 1;
        // Serial.printf("LINE >%s<\n", line.data());
        this->handleCommand(line); // TODO: remove potential \r
    }
    // this->receiveBuffer.erase(0, offset); //FIX
}

void CompanionSatellite::handleCommand(std::string_view line)
{
    size_t i = line.find_first_of(' ');
    std::string_view cmd = (i == std::string::npos) ? line : line.substr(0, i);
    std::string_view body = (i == std::string::npos) ? "" : line.substr(i + 1);

    // Serial.printf("CMD: >%s<\tBODY: >%s<\n", cmd.data(), body.data());

    std::vector<parm> params = parseLineParameters(body);

    auto cmd_index = std::find(commandList.begin(), commandList.end(), cmd);

    switch (distance(commandList.begin(), cmd_index))
    {
    case 0: // PING
        Serial.printf("PING %s\n", body.data());
        // this.socket?.write(`PONG ${body}\n`)
        break;
    case 1: //'PONG':
        // console.log('Got pong')
        Serial.printf("PONG %s\n", body.data());
        // this._pingUnackedCount = 0
        break;
    case 2: //'KEY-STATE':
        // Serial.printf("KEY-STATE %s\n", body.data());
        this->handleState(params);
        break;
    case 3: //'KEYS-CLEAR':
        Serial.printf("KEYS-CLEAR %s\n", body.data());
        // this.handleClear(params)
        break;
    case 4: //'BRIGHTNESS':
        // Serial.printf("BRIGHTNESS %s\n", body.data());
        this->handleBrightness(params);
        break;
    case 5: //'ADD-DEVICE':
        // Serial.printf("ADD-DEVICE: %s\n", body.data());
        this->handleAddedDevice(params);
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
        // Serial.printf("KEY-PRESS: %s\n", body.data());
        // Ignore
        break;
    default:
        Serial.printf("Received unhandled command: %s %s\n", cmd.data(), body.data());
        // console.log(`Received unhandled command: ${cmd} ${body}`)
        break;
    }
}

void CompanionSatellite::handleState(std::vector<parm> params)
{
    if (params[0].key != "DEVICEID")
    {
        Serial.printf("Mising DEVICEID in KEY-DRAW response");
        return;
    }
    if (params[1].key != "KEY")
    {
        Serial.printf("Mising KEY in KEY-DRAW response");
        return;
    }

    int keyIndex{};

    auto [ptr, ec]{std::from_chars(params[1].val.data(), params[1].val.data() + params[1].val.size(), keyIndex)};

    if (ec == std::errc())
    {
        this->drawQueue.push_back(DeviceDrawProps());

        this->drawQueue.back().keyIndex = keyIndex;

        for (auto it = params.begin() + 2; it != params.end(); ++it)
        {
            if (this->prop.bitmaps && it->key == "BITMAP")
            {
                this->drawQueue.back().image = it->val;
            }
            else if (this->prop.color && it->key == "COLOR")
            {
                this->drawQueue.back().color = it->val;
            }
            else if (this->prop.text && it->key == "TEXT")
            {
                this->drawQueue.back().text = B64::decode(it->val);
            }
        }
    }
    else
    {
        Serial.printf("Bad KEY in KEY-DRAW response\n");
        return;
    }

    // const image = typeof params.BITMAP === 'string' ? Buffer.from(params.BITMAP, 'base64') : undefined
    // 	const text = typeof params.TEXT === 'string' ? Buffer.from(params.TEXT, 'base64').toString() : undefined
    // 	const color = typeof params.COLOR === 'string' ? params.COLOR : undefined

    // 	this.emit('draw', { deviceId: params.DEVICEID, keyIndex, image, text, color })
}

void CompanionSatellite::handleBrightness(std::vector<parm> params)
{
    // for (auto p : params)
    //     Serial.printf("KEY: >%s< VAL: >%s<\n", p.key.data(), p.val.data());

    if (params[0].key != "DEVICEID")
    {
        Serial.printf("Mising DEVICEID in BRIGHTNESS respons\ne");
        return;
    }
    if (params[1].key != "VALUE")
    {
        Serial.printf("Mising VALUE in BRIGHTNESS response\n");
        return;
    }
    int percent{};
    auto [ptr, ec]{std::from_chars(params[1].val.data(), params[1].val.data() + params[1].val.size(), percent)};

    if (ec == std::errc())
    {
        Serial.printf("BRIGHTNESS: %d\n", percent);
    }
    else
    {
        Serial.printf("Bad VALUE in BRIGHTNESS\n");
        return;
    }

    // this.emit('brightness', { deviceId: params.DEVICEID, percent })
}

void CompanionSatellite::keyDown(std::string deviceId, int keyIndex)
{
    if (this->_connected)
    {
        this->transmitBuffer.append("KEY-PRESS DEVICEID=" + deviceId +
                                    " KEY=" + std::to_string(keyIndex) +
                                    " PRESSED=1\n");
    }
}

void CompanionSatellite::keyUp(std::string deviceId, int keyIndex)
{
    if (this->_connected)
    {
        this->transmitBuffer.append("KEY-PRESS DEVICEID=" + deviceId +
                                    " KEY=" + std::to_string(keyIndex) +
                                    " PRESSED=0\n");
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
            " COLORS=" + ((props.color) ? "1" : "0") +
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
    if (params[1].key != "DEVICEID")
    {
        Serial.printf("Mising DEVICEID in ADD-DEVICE response");
        return;
    }

    _deviceStatus = true;

    // this.emit('newDevice', { deviceId: params.DEVICEID })
}

bool CompanionSatellite::connected()
{
    return false;
}

#endif