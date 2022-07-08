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
        if (this->_connectionActive)
        {
            if (this->_deviceStatus == 0)
            {
                this->addDevice();
            }
            else if (this->_deviceStatus == -1)
            {
                if (millis() - this->_addDeviceTimeout > 10000)
                {
                    Serial.printf("_addDeviceTimeout\n");
                    this->addDevice();
                }
            }
        }
    }
    else
    {
        this->_connectionActive = false;
        this->_deviceStatus = 0;
    }
}

/**
 * Find first match in string list.
 *
 * ........
 *
 * @param data pair with start and end of data.
 * @return -1 if no match otherwise index of match.
 */
int CompanionSatellite::findInCmdList(std::pair<const char *, const char *> data)
{
    int past = 0;
    for (int i = 0; i < this->cmd_list.size(); i++)
    {
        const char *first1 = data.first;
        const char *last1 = data.second;

        const char *first2 = this->cmd_list[i].data();
        const char *last2 = this->cmd_list[i].data() + this->cmd_list[i].size() - 1;
        // Serial.printf(">%.*s< ? >%.*s<\n", data.second-data.first, data.first, this->cmd_list[i].size(), this->cmd_list[i].data());

        while (first1 != last1)
        {
            if (*first2 > *first1)
            {
                // Serial.printf("%c > %c \n", *first2, *first1);
                past = -1;
                break;
            }
            else if (*first1 > *first2)
            {
                break;
            }
            else if (first2 == last2 && *first2 == *first1)
            {
                past = 1;
                break;
            }
            ++first1;
            ++first2;
        }

        if (past == 1)
        {
            // Serial.printf("MATCH %d\n", i);
            return i;
        }
    }

    return -1;
}

std::vector<CompanionSatellite::parm> CompanionSatellite::parseLineParameters(std::string_view line)
{
    std::vector<std::pair<const char *, const char *>> fragments;

    bool inQuots = false;

    const char *offset = line.begin();
    const char *itr = line.begin();
    const char *line_end = line.end();
    for (; itr < line_end; itr++)
    {
        if (inQuots)
        {
            if (*itr == '"')
            {
                fragments.push_back(std::make_pair(offset, itr++));
                offset = itr + 1;
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
                if (offset != itr)
                {
                    if (*offset == ' ')
                        offset++;

                    fragments.push_back(std::make_pair(offset, itr));
                    offset = itr + 1;
                }
            }
        }
    }
    if (itr - offset > 3)
    {
        fragments.push_back(std::make_pair(offset, itr));
    }

    std::vector<parm> res;
    for (auto fragment : fragments)
    {
        // Serial.printf("fragment: >%.*s<\n", fragment.second-fragment.first, fragment.first);
        parm p;
        if (const char *equals = std::find(fragment.first, fragment.second, '='); equals != fragment.second)
        {
            p.key = std::string_view(fragment.first, equals - fragment.first);  // fragment.substr(0, equals);
            p.val = std::string_view(equals + 1, fragment.second - equals - 1); // fragment.substr(equals + 1);
            if (p.val.front() == '"')
            {
                p.val.remove_prefix(1);
            }
            while (p.val.back() == '"' || p.val.back() == '\n' || p.val.back() == '\r')
            {
                p.val.remove_suffix(1);
            }
            res.push_back(p);
        }
        else
        {
            p.key = std::string_view(fragment.first, equals - fragment.first); // fragment.substr(0, equals);
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
        // this->handleCommand(line); // TODO: remove potential \r
        this->handleCommand(std::make_pair(line.begin(), line.end())); // TODO: remove potential \r
    }
    // this->receiveBuffer.erase(0, offset); //FIX
}

void CompanionSatellite::handleCommand(std::pair<const char *, const char *> line)
{
    std::pair<const char *, const char *> cmd;

    const char *i = std::find(line.first, line.second, ' ');

    cmd = (i == line.second) ? line : std::make_pair(line.first, i);
    std::string_view body = (i == line.second) ? "" : std::string_view(i+1, line.second - i-1);

    // Serial.printf("CMD: >%.*s<\tBODY:>%.*s<\n", cmd.second-cmd.first, cmd.first, body.size(), body.data());

    std::vector<parm> params = parseLineParameters(body);

    switch (this->findInCmdList(cmd))
    {
    case 0: //'ADD-DEVICE':
        // Serial.printf("ADD-DEVICE: %s\n", body.data());
        this->handleAddedDevice(params);
        break;
    case 1: //'BEGIN':
        Serial.printf("Connected to Companion: %s\n", body.data());
        this->transmitBuffer.clear();
        this->_connectionActive = true;
        this->_deviceStatus = 0;
        break;
    case 2: //'BRIGHTNESS':
        // Serial.printf("BRIGHTNESS %s\n", body.data());
        this->handleBrightness(params);
        break;
    case 3: //'KEY-PRESS':
        // Serial.printf("KEY-PRESS: %s\n", body.data());
        // Ignore
        break;
    case 4: //'KEY-STATE':
        // Serial.printf("KEY-STATE %s\n", body.data());
        this->handleState(params);
        break;
    case 5: //'KEYS-CLEAR':
        Serial.printf("KEYS-CLEAR %s\n", body.data());
        // this.handleClear(params)
        break;
    case 6: // PING
        Serial.printf("PING %s\n", body.data());
        // this.socket?.write(`PONG ${body}\n`)
        break;
    case 7: //'PONG':
        // console.log('Got pong')
        Serial.printf("PONG %s\n", body.data());
        // this._pingUnackedCount = 0
        break;
    case 8: //'REMOVE-DEVICE':
        Serial.printf("REMOVE-DEVICE: %s\n", body.data());
        this->_deviceStatus = 0;
        break;
    default:
        Serial.printf("Received unhandled CMD: >%.*s<\tBODY:>%.*s<\n", cmd.second-cmd.first, cmd.first, body.size(), body.data());
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
                it->val.remove_prefix(1);
                std::from_chars(it->val.data(), it->val.data() + it->val.size(), this->DeviceDraw[keyIndex].color, 16);
                Serial.printf("ALL: %d\t RED: %d\t GREEN: %d\t BLUE: %d\n", this->DeviceDraw[keyIndex].color,
                              this->DeviceDraw[keyIndex].red,
                              this->DeviceDraw[keyIndex].green,
                              this->DeviceDraw[keyIndex].blue);
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
    _addDeviceTimeout = millis();
    if (this->_connectionActive && this->_deviceStatus != 1)
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

void CompanionSatellite::removeDevice()
{
    this->_addDeviceTimeout = millis();
    if (this->_connectionActive)
    {
        this->transmitBuffer.append("REMOVE-DEVICE DEVICEID=" + this->_deviceId + "\n");
        this->_deviceStatus = 0;
    }
}

void CompanionSatellite::handleAddedDevice(std::vector<parm> params)
{
    // for (auto p : params)
    //     Serial.printf("KEY: >%.*s< VAL: >%.*s<\n", p.key.size(), p.key.data(), p.val.size(), p.val.data());

    if (params[0].key != "OK" || params[0].key == "ERROR")
    {
        if (params[2].key == "MESSAGE")
        {
            Serial.printf("Add device failed: %.*s\n", params[2].val.size(), params[2].val.data());
            if (params[2].val.compare("Device exists elsewhere") == 0)
            {
                this->removeDevice();
            }
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
        this->_deviceStatus = -1;
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