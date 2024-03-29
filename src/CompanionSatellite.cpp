#include <Arduino.h>
#include <CompanionSatellite.h>

CompanionSatellite::CompanionSatellite(std::string deviceId, std::string productName, int keysTotal, int keysPerRow, bool bitmaps, bool color, bool text)
{
    this->_deviceId = deviceId;
    this->_productName = productName;
    this->_props.keysTotal = keysTotal;
    this->_props.keysPerRow = keysPerRow;
    this->_props.bitmaps = bitmaps;
    this->_props.color = color;
    this->_props.text = text;

    this->_keyUpCmd.append("KEY-PRESS DEVICEID=" + _deviceId + " PRESSED=0 KEY=");
    this->_keyDownCmd.append("KEY-PRESS DEVICEID=" + _deviceId + " PRESSED=1 KEY=");

    DeviceDraw.resize(keysTotal);
}

void CompanionSatellite::maintain(bool clientStatus, char *data)
{
    if (clientStatus)
    {
        if (data != nullptr)
        {
            this->_lastReceivedAt = millis();
            this->_handleReceivedData(data);
        }
        if (_connectionActive)
        {
            if (_deviceStatus == 0)
            {
                this->addDevice();
                _addDeviceTimeout = millis();
            }
            else if (_deviceStatus == -1)
            {
                if (millis() - _addDeviceTimeout > 2000)
                {
                    Serial.printf("_addDeviceTimeout\n");
                    this->addDevice();
                }
            }
        }
    }
    else
    {
        _connectionActive = false;
        _deviceStatus = 0;
    }
}

std::vector<CompanionSatellite::parm> CompanionSatellite::parseLineParameters(std::string_view line)
{
    std::vector<std::string_view> fragments;

    bool inQuots = false;

    auto offset = line.begin();
    auto itr = line.begin();
    for (; itr < line.end(); itr++)
    {
        if (inQuots)
        {
            if (*itr == '"')
            {
                fragments.push_back(std::string_view(offset, itr - offset + 1));
                offset = itr + 2;
                inQuots = false;
            }
        }
        else
        {
            if (*itr == '"')
            {
                inQuots = true;
            }
            else if (*itr == ' ')
            {
                fragments.push_back(std::string_view(offset, itr - offset));
                offset = itr + 1;
            }
        }
    }
    if (itr - offset > 2)
    {
        fragments.push_back(std::string_view(offset, itr - offset));
    }

    std::vector<parm> res;
    for (auto fragment : fragments)
    {
        parm p;
        if (size_t equals = fragment.find_first_of('='); equals != std::string::npos)
        {
            p.key = fragment.substr(0, equals);
            p.val = fragment.substr(equals + 1);
            if (p.val.front() == '"')
            {
                p.val.remove_prefix(1);
                p.val.remove_suffix(1);
            }
            res.push_back(p);
        }
        else
        {
            p.key = fragment.substr(0, equals);
            p.val = "1";
            res.push_back(p);
        }

        // Serial.printf("KEY: >%.*s< VAL: >%.*s<\n", p.key.size(), p.key.data(), p.val.size(), p.val.data());
    }
    return res;
}

void CompanionSatellite::_handleReceivedData(char *data)
{
    this->receiveBuffer = std::string_view(data);

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
    if (this->_deviceStatus == -1)
    {
        this->addDevice();
    }
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
        // Serial.printf("REMOVE-DEVICE: %s\n", body.data());
        this->_deviceStatus = 0;
        break;
    case 7: //'BEGIN':
        Serial.printf("Connected to Companion: %s\n", body.data());
        this->transmitBuffer.clear();
        this->_connectionActive = true;
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
    // for (auto p : params)
    //     Serial.printf("KEY: >%.*s< VAL: >%.*s<\n", p.key.size(), p.key.data(), p.val.size(), p.val.data());

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

    if (params[0].val != this->_deviceId)
    {
        Serial.printf("Wrong DEVICEID in ADD-DEVICE response\n");
        return;
    }

    int keyIndex{};

    auto [ptr, ec]{std::from_chars(params[1].val.data(), params[1].val.data() + params[1].val.size(), keyIndex)};

    if (ec == std::errc())
    {
        for (auto it = params.begin() + 2; it != params.end(); ++it)
        {
            if (this->_props.bitmaps && it->key == "BITMAP")
            {
                this->DeviceDraw[keyIndex].image = it->val;
            }
            else if (this->_props.color && it->key == "COLOR")
            {
                this->DeviceDraw[keyIndex].color = it->val;
            }
            else if (this->_props.text && it->key == "TEXT")
            {
                this->DeviceDraw[keyIndex].text = B64::decode(it->val);
            }
            else if (it->key == "PRESSED")
            {
                this->DeviceDraw[keyIndex].pressed = (it->val.front() == '1' || it->val.front() == 't') ? true : false;
            }
        }
        this->update = true;
    }
    else
    {
        Serial.printf("Bad KEY in KEY-DRAW response\n");
        return;
    }
}

void CompanionSatellite::handleBrightness(std::vector<parm> params)
{
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

    if (params[0].val != this->_deviceId)
    {
        Serial.printf("Wrong DEVICEID in ADD-DEVICE response\n");
        return;
    }

    int percent{};
    auto [ptr, ec]{std::from_chars(params[1].val.data(), params[1].val.data() + params[1].val.size(), percent)};

    if (ec == std::errc())
    {
        // Serial.printf("BRIGHTNESS: %d\n", percent);
        _brightness = percent;
    }
    else
    {
        Serial.printf("Bad VALUE in BRIGHTNESS\n");
        return;
    }

    // this.emit('brightness', { deviceId: params.DEVICEID, percent })
}

void CompanionSatellite::keyDown(int keyIndex)
{
    if (this->_connectionActive)
    {
        this->transmitBuffer.append(_keyDownCmd + std::to_string(keyIndex) + "\n");
    }
}

void CompanionSatellite::keyUp(int keyIndex)
{
    if (this->_connectionActive)
    {
        this->transmitBuffer.append(_keyUpCmd + std::to_string(keyIndex) + "\n");
    }
}

void CompanionSatellite::addDevice()
{
    if (this->_connectionActive)
    {
        this->transmitBuffer.append(
            "ADD-DEVICE DEVICEID=" + this->_deviceId +
            " PRODUCT_NAME=\"" + this->_productName +
            "\" KEYS_TOTAL=" + std::to_string(this->_props.keysTotal) +
            " KEYS_PER_ROW=" + std::to_string(this->_props.keysPerRow) +
            " BITMAPS=" + ((this->_props.bitmaps) ? "1" : "0") +
            " COLORS=" + ((this->_props.color) ? "1" : "0") +
            " TEXT=" + ((this->_props.text) ? "1" : "0") + "\n");
    }
    this->_deviceStatus = -1;
}

void CompanionSatellite::handleAddedDevice(std::vector<parm> params)
{
    // for (auto p : params)
    //     Serial.printf("KEY: >%.*s< VAL: >%.*s<\n", p.key.size(), p.key.data(), p.val.size(), p.val.data());

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

    if (params[1].val != this->_deviceId)
    {
        Serial.printf("Wrong DEVICEID in ADD-DEVICE response\n");
        return;
    }

    _deviceStatus = 1;

    // this.emit('newDevice', { deviceId: params.DEVICEID })
}