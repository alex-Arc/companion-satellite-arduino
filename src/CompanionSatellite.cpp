// #include <Arduino.h>
#include <CompanionSatellite.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>
#include <vector>

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

CompanionSatellite::CompanionSatellite(const char *deviceId, const char *productName, int keysTotal, int keysPerRow, bool bitmaps, bool color, bool text)
{
    this->_deviceId.assign(deviceId);
    this->_productName.assign(productName);
    this->_props.keysTotal = keysTotal;
    this->_props.keysPerRow = keysPerRow;
    this->_props.bitmaps = bitmaps;
    this->_props.color = color;
    this->_props.text = text;

    this->_keyUpCmd.append("KEY-PRESS DEVICEID=" + _deviceId + " PRESSED=0 KEY=");
    this->_keyDownCmd.append("KEY-PRESS DEVICEID=" + _deviceId + " PRESSED=1 KEY=");

    DeviceDraw.resize(keysTotal);
}

/**
 * Find the matching command.
 *
 * expects the first word i the string to be a valid command
 *
 * @param data string ptr to search.
 * @return CMD enum.
 */
CompanionSatellite::CMD CompanionSatellite::parseCmdType(const char *data)
{
    auto cmd = std::lower_bound(cmd_list.begin(), cmd_list.end(), data,
                                [](std::string from_list, const char *from_data)
                                { return (from_list.compare(0, std::string::npos, from_data, 0, from_list.size()) < 0); });

    if (cmd != cmd_list.end() && cmd->compare(0, std::string::npos, data, 0, cmd->size()) == 0)
    {
        return (CompanionSatellite::CMD)std::distance(cmd_list.begin(), cmd);
    }
    else
    {
        return CompanionSatellite::CMD::CMD_NONE;
    }
}

/**
 * Find Parameters.
 *
 *
 *
 * @param data string ptr to search.
 * @return Parm_t.
 */
CompanionSatellite::Parm_t CompanionSatellite::parseParameters(const char *data)
{
    auto arg = std::lower_bound(arg_list.begin(), arg_list.end(), data,
                                [](std::string from_list, const char *from_data)
                                { return (from_list.compare(0, std::string::npos, from_data, 0, from_list.size()) < 0); });

    if (arg != arg_list.end() && arg->compare(0, std::string::npos, data, 0, arg->size()) == 0)
    {
        switch (data[arg->size()])
        {
        case '\n':
        case ' ':
        {
            Parm_t parm = {
                .arg = (CompanionSatellite::ARG)std::distance(arg_list.begin(), arg),
                .val = "t"};
            return parm;
        }
        break;
        case '=':
        {
            const char *vs = data + arg->size() + 1;
            const char *ve;
            if (*vs == '\"')
            {
                ++vs;
                ve = std::strpbrk(vs, "\"");
            }
            else
            {
                ve = std::strpbrk(vs, " \n");
            }
            if (ve != nullptr)
            {
                Parm_t parm = {
                    .arg = (CompanionSatellite::ARG)std::distance(arg_list.begin(), arg),
                    .val = std::string(vs, ve - vs)};
                return parm;
            }
            else
            {
                Parm_t parm = {
                    .arg = CompanionSatellite::ARG::ARG_NONE,
                    .val = "f"};
                return parm;
            }
        }
        break;
        default:
        {
            Parm_t parm = {
                .arg = CompanionSatellite::ARG::ARG_NONE,
                .val = "f"};
            return parm;
        }
        break;
        }
    }
    else
    {
        Parm_t parm = {
            .arg = CompanionSatellite::ARG::ARG_NONE,
            .val = "none"};
        return parm;
    }
}

/**
 *
 * @param data null terminated char pointer
 */
int CompanionSatellite::parseData(const std::string data)
{
    return -1;
}

// void CompanionSatellite::maintain(bool clientStatus, char *data, size_t len)
// {
//     if (clientStatus)
//     {
//         if (data != nullptr)
//         {
//             this->_lastReceivedAt = 0; // millis();
//             this->_handleReceivedData(data, len);
//         }
//         if (this->_connectionActive)
//         {
//             if (this->_deviceStatus == 0)
//             {
//                 this->addDevice();
//             }
//             else if (this->_deviceStatus == -1)
//             {
//                 // if (millis() - this->_addDeviceTimeout > 10000)
//                 // {
//                 //     //Serial.printf("_addDeviceTimeout\n");
//                 //     this->addDevice();
//                 // }
//             }
//         }
//     }
//     else
//     {
//         this->_connectionActive = false;
//         this->_deviceStatus = 0;
//     }
// }

// /**
//  * Find first match in string list.
//  * @param data
//  * @return -1 if no match, otherwise index of match.
//  */
// int CompanionSatellite::findInCmdList(char *data)
// {
//     for (uint8_t i = 0; i < this->cmd_list.size(); i++)
//     {

//         const char *first2 = this->cmd_list[i];
//         // //Serial.printf(">%s< ? >%s<\n", data, this->cmd_list[i]);
//         if (std::strcmp(data, first2) == 0) // TODO: this can be optimized
//         {
//             return i;
//         }
//     }

//     return -1;
// }

// std::vector<CompanionSatellite::parm> CompanionSatellite::parseLineParameters(char *line)
// {
//     std::vector<char *> fragments;

//     bool inQuots = false;

//     char *offset = line;
//     char *itr = line;
//     const char *line_end = line + std::strlen(line);
//     for (; itr < line_end; itr++)
//     {
//         if (inQuots)
//         {
//             if (*itr == '"')
//             {
//                 *itr = 0;
//                 fragments.push_back(offset);
//                 offset = ++itr;
//                 inQuots = false;
//             }
//         }
//         else
//         {
//             if (*itr == '"')
//             {
//                 inQuots = true;
//             }
//             else if (*itr == ' ')
//             {
//                 if (offset != itr)
//                 {
//                     if (*offset == ' ')
//                         offset++;

//                     *itr = 0;
//                     fragments.push_back(offset);
//                     offset = itr + 1;
//                 }
//             }
//         }
//     }
//     if (itr - offset > 3)
//     {
//         *itr = 0;
//         fragments.push_back(offset);
//     }

//     std::vector<parm> res;
//     for (auto fragment : fragments)
//     {
//         // //Serial.printf("fragment: >%s<\n", fragment);
//         parm p;
//         char *equals = std::strchr(fragment, '=');
//         if (equals != nullptr)
//         {
//             *equals = 0;
//             p.key = fragment; // fragment.substr(0, equals);
//             p.val = ++equals; // fragment.substr(equals + 1);
//             if (*p.val == '"')
//             {
//                 p.val++;
//             }
//             res.push_back(p);
//         }
//         else
//         {
//             p.key = fragment; // fragment.substr(0, equals);
//             p.val = true_val;
//             res.push_back(p);
//         }

//         // //Serial.printf("KEY: >%s< VAL: >%s<\n", p.key, p.val);
//     }
//     return res;
// }

// void CompanionSatellite::_handleReceivedData(char *data, size_t len)
// {
//     char *i;
//     while ((i = strchr(data, '\n')) != nullptr)
//     {
//         *i = 0;
//         this->handleCommand(data); // TODO: remove potential \r
//         data = ++i;
//         // //Serial.printf("LINE >%s<\n", line.data());
//         // this->handleCommand(line); // TODO: remove potential \r
//     }
//     // this->receiveBuffer.erase(0, offset); //FIX
// }

// void CompanionSatellite::handleCommand(char *line)
// {
//     char *cmd, *body;

//     char *i = strchr(line, ' ');

//     if (i != nullptr)
//         *i = 0;

//     cmd = line;
//     if (strlen(cmd) < 4)
//         return;

//     body = (i == nullptr) ? nullptr : ++i;

//     // Serial.printf("CMD: >%s<\tBODY:>%s<\n", cmd, body);

//     std::vector<parm> params = parseLineParameters(body);

//     switch (this->findInCmdList(cmd))
//     {
//     case 0: //'ADD-DEVICE':
//         // //Serial.printf("ADD-DEVICE: %s\n", body.data());
//         this->handleAddedDevice(params);
//         break;
//     case 1: //'BEGIN':
//         // Serial.printf("Connected to Companion: %s\n", body);
//         this->transmitBuffer.clear();
//         this->_connectionActive = true;
//         this->_deviceStatus = 0;
//         break;
//     case 2: //'BRIGHTNESS':
//         // //Serial.printf("BRIGHTNESS %s\n", body.data());
//         this->handleBrightness(params);
//         break;
//     case 3: //'KEY-PRESS':
//         // //Serial.printf("KEY-PRESS: %s\n", body.data());
//         // Ignore
//         break;
//     case 4: //'KEY-STATE':
//         // //Serial.printf("KEY-STATE %s\n", body.data());
//         this->handleState(params);
//         break;
//     case 5: //'KEYS-CLEAR':
//         // Serial.printf("KEYS-CLEAR %s\n", body);
//         //  this.handleClear(params)
//         break;
//     case 6: // PING
//         // Serial.printf("PING %s\n", body);
//         //  this.socket?.write(`PONG ${body}\n`)
//         break;
//     case 7: //'PONG':
//         // console.log('Got pong')
//         // Serial.printf("PONG %s\n", body);
//         // this._pingUnackedCount = 0
//         break;
//     case 8: //'REMOVE-DEVICE':
//         // Serial.printf("REMOVE-DEVICE: %s\n", body);
//         this->_deviceStatus = 0;
//         break;
//     default:
//         // Serial.printf("Received unhandled CMD: >%s<\tBODY:>%s<\n", cmd, body);
//         //  console.log(`Received unhandled command: ${cmd} ${body}`)
//         break;
//     }
// }

// void CompanionSatellite::handleState(std::vector<parm> params)
// {
//     // for (auto p : params)
//     //     //Serial.printf("KEY: >%s< VAL: >%s<\n", p.key.size(), p.key.data(), p.val.size(), p.val.data());

//     // if (params[0].key != "DEVICEID")
//     if (*params[0].key != 'D')
//     {
//         // Serial.printf("Mising DEVICEID in KEY-DRAW response");
//         return;
//     }
//     // if (params[1].key != "KEY")
//     if (*params[1].key != 'K')
//     {
//         // Serial.printf("Mising KEY in KEY-DRAW response");
//         return;
//     }

//     if (*params[0].val != this->_deviceId[0])
//     {
//         // Serial.printf("Wrong DEVICEID in ADD-DEVICE response\n");
//         return;
//     }

//     int keyIndex = -1;

//     // auto [ptr, ec]{std::from_chars(params[1].val.first, params[1].val.second, keyIndex)};

//     // -----keyIndex = std::strtod(params[1].val, nullptr);---

//     if (keyIndex >= 0)
//     {
//         for (auto it = params.begin() + 2; it != params.end(); ++it)
//         {
//             if (this->_props.bitmaps && *it->key == 'B')
//             {
//                 // this->DeviceDraw[keyIndex].image = it->val;
//             }
//             else if (this->_props.color && *it->key == 'C')
//             {
//                 ++it->val;
//                 // std::from_chars(it->val.first, it->val.second, this->DeviceDraw[keyIndex].color, 16); c++17
//                 this->DeviceDraw[keyIndex].color = std::strtol(it->val, nullptr, 16);
//                 // Serial.printf("ALL: %d\t RED: %d\t GREEN: %d\t BLUE: %d\n", this->DeviceDraw[keyIndex].color,
//                 //    this->DeviceDraw[keyIndex].red,
//                 //    this->DeviceDraw[keyIndex].green,
//                 //    this->DeviceDraw[keyIndex].blue);
//             }
//             else if (this->_props.text && *it->key == 'T')
//             {
//                 // this->DeviceDraw[keyIndex].text = B64::decode(it->val, it->val.second - it->val.first);
//             }
//             else if (*it->key == 'P')
//             {
//                 this->DeviceDraw[keyIndex].pressed = (*it->val == '1' || *it->val == 't') ? true : false;
//             }
//         }
//         this->update = true;
//     }
//     else
//     {
//         // Serial.printf("Bad KEY in KEY-DRAW response\n");
//         return;
//     }
// }

// void CompanionSatellite::handleBrightness(std::vector<parm> params)
// {
//     // if (params[0].key != "DEVICEID")
//     // if (*params[0].key != 'D')
//     // {
//     //     //Serial.printf("Mising DEVICEID in BRIGHTNESS respons\n");
//     //     return;
//     // }
//     // if (*params[1].key != 'V')
//     // {
//     //     //Serial.printf("Mising VALUE in BRIGHTNESS response\n");
//     //     return;
//     // }

//     // if (*params[0].val != this->_deviceId[0])
//     // {
//     //     //Serial.printf("Wrong DEVICEID in ADD-DEVICE response\n");
//     //     return;
//     // }

//     int percent = -1;
//     // auto [ptr, ec]{std::from_chars(params[1].val.first, params[1].val.second, percent)};

//     percent = std::strtod(params[1].val, nullptr);

//     if (percent >= 0)
//     {
//         // //Serial.printf("BRIGHTNESS: %d\n", percent);
//         _brightness = percent;
//     }
//     else
//     {
//         // Serial.printf("Bad VALUE in BRIGHTNESS\n");
//         return;
//     }

//     // this.emit('brightness', { deviceId: params.DEVICEID, percent })
// }

// void CompanionSatellite::keyDown(int keyIndex)
// {
//     if (this->_connectionActive)
//     {
//         char ki[4];
//         snprintf(ki, 4, "%d", keyIndex);
//         this->transmitBuffer.append(_keyDownCmd + std::string(ki) + "\n");
//     }
// }

// void CompanionSatellite::keyUp(int keyIndex)
// {
//     // if (this->_connectionActive)
//     // {
//     char ki[4];
//     snprintf(ki, 4, "%d", keyIndex);
//     this->transmitBuffer.append(_keyUpCmd + std::string(ki) + "\n");
//     // }
// }

// void CompanionSatellite::addDevice()
// {
//     _addDeviceTimeout = 0; // millis();
//     // if (this->_connectionActive && this->_deviceStatus != 1)
//     // {
//     char kt[4];
//     char kpr[4];
//     snprintf(kt, 4, "%d", this->_props.keysTotal);
//     snprintf(kpr, 4, "%d", this->_props.keysPerRow);

//     this->transmitBuffer.append(
//         "ADD-DEVICE DEVICEID=" + this->_deviceId +
//         " PRODUCT_NAME=\"" + this->_productName +
//         "\" KEYS_TOTAL=" + std::string(kt) +
//         " KEYS_PER_ROW=" + std::string(kpr) +
//         " BITMAPS=" + ((this->_props.bitmaps) ? "1" : "0") +
//         " COLORS=" + ((this->_props.color) ? "1" : "0") +
//         " TEXT=" + ((this->_props.text) ? "1" : "0") + "\n");
//     // }
//     this->_deviceStatus = -1;
// }

// void CompanionSatellite::removeDevice()
// {
//     this->_addDeviceTimeout = 0; // millis();
//     if (this->_connectionActive)
//     {
//         this->transmitBuffer.append("REMOVE-DEVICE DEVICEID=" + this->_deviceId + "\n");
//         this->_deviceStatus = 0;
//     }
// }

// void CompanionSatellite::handleAddedDevice(std::vector<parm> params)
// {
//     if (strcmp(params[0].key, "OK") != 0 || strcmp(params[0].key, "ERROR") == 0)
//     {
//         if (strcmp(params[2].key, "MESSAGE") == 0)
//         {
//             // Serial.printf("Add device failed: %s\n", params[2].val - params[2].val, params[2].val);
//             //  if (params[2].val.compare("Device exists elsewhere") == 0)
//             //  {
//             //      this->removeDevice();
//             //  }
//         }
//         else
//         {
//             // Serial.printf("Add device failed: no message\n");
//         }
//         // if (typeof params.DEVICEID === 'string') {
//         // 	this.emit('deviceErrored', {
//         // 		deviceId: params.DEVICEID,
//         // 		message: `${params.MESSAGE || 'Unknown Error'}`,
//         // 	})
//         // }
//         this->_deviceStatus = -1;
//         return;
//     }
//     if (strcmp(params[1].key, "DEVICEID") != 0)
//     {
//         // Serial.printf("Mising DEVICEID in ADD-DEVICE response");
//         return;
//     }

//     if (strcmp(params[1].val, this->_deviceId.data()) != 0)
//     {
//         // Serial.printf("Wrong DEVICEID in ADD-DEVICE response\n");
//         return;
//     }

//     _deviceStatus = 1;

//     // this.emit('newDevice', { deviceId: params.DEVICEID })
// }