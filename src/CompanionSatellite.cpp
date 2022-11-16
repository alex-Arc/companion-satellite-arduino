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
 * @return CMD_e enum.
 */
CompanionSatellite::CMD_e CompanionSatellite::parseCmdType(const char *data)
{
    while (*data == ' ' || *data == '\n')
                ++data;

    auto cmd = std::lower_bound(cmd_list.begin(), cmd_list.end(), data,
                                [](std::string from_list, const char *from_data)
                                { return (from_list.compare(0, std::string::npos, from_data, 0, from_list.size()) < 0); });

    if (cmd != cmd_list.end() && cmd->compare(0, std::string::npos, data, 0, cmd->size()) == 0)
    {
        this->_cursor = data + cmd->size();
        return (CompanionSatellite::CMD_e)std::distance(cmd_list.begin(), cmd);
    }
    else
    {
        return CompanionSatellite::CMD_e::CMD_NONE;
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
    while (*data == ' ' || *data == '\n')
                ++data;

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
                .arg = (ARG_e)std::distance(arg_list.begin(), arg),
                .val = "t"};
            this->_cursor = data + arg->size() + 1;
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
                    .arg = (ARG_e)std::distance(arg_list.begin(), arg),
                    .val = std::string(vs, ve - vs)};

                this->_cursor = ve + 1;
                return parm;
            }
            else
            {
                Parm_t parm = {
                    .arg = ARG_e::ARG_NONE,
                    .val = "f"};
                return parm;
            }
        }
        break;
        default:
        {
            Parm_t parm = {
                .arg = ARG_e::ARG_NONE,
                .val = "f"};
            return parm;
        }
        break;
        }
    }
    else
    {
        Parm_t parm = {
            .arg = ARG_e::ARG_NONE,
            .val = "none"};
        return parm;
    }
}

int CompanionSatellite::parseData(const std::string data)
{
    this->_cursor = data.data();
    int ret = 0;
    CMD_e cmd = parseCmdType(_cursor);
    while (cmd != CMD_e::CMD_NONE)
    {
        cmd_t command = {.cmd = cmd};
        Parm_t parm = parseParameters(_cursor);
        while (parm.arg != ARG_e::ARG_NONE)
        {
            command.parm.push_back(std::move(parm));
            parm = parseParameters(_cursor);
        }

        this->_cmd_buffer.push_back(std::move(command));
        ret++;

        cmd = parseCmdType(_cursor);
    }

    return ret;
}

/**
 * Craft ADD-DEVICE message.
 * @param
 * @return
 */
void CompanionSatellite::addDevice()
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
