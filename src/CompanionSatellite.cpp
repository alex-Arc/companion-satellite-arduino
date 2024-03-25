#include <Arduino.h>
#include <CompanionSatellite.h>

#include <algorithm>
#include <cstring>
#include <iterator>
#include <string>
#include <queue>
// #include <chrono>

CompanionSatellite::CompanionSatellite(std::string deviceId, std::string productName, int keysTotal, int keysPerRow, bool bitmaps, bool color, bool text)
{
    this->_deviceId = deviceId;
    this->_productName = productName;
    this->_props.keysTotal = keysTotal;
    this->_props.keysPerRow = keysPerRow;
    this->_props.bitmaps = bitmaps;
    this->_props.color = color;
    this->_props.text = text;

    this->keyState.resize(keysTotal);

    this->_keyUpCmd.append("KEY-PRESS DEVICEID=" + _deviceId + " PRESSED=0 KEY=");
    this->_keyDownCmd.append("KEY-PRESS DEVICEID=" + _deviceId + " PRESSED=1 KEY=");
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

    this->keyState.resize(keysTotal);

    this->_keyUpCmd.append("KEY-PRESS DEVICEID=" + _deviceId + " PRESSED=0 KEY=");
    this->_keyDownCmd.append("KEY-PRESS DEVICEID=" + _deviceId + " PRESSED=1 KEY=");
}

/**
 * Find the matching command.
 *
 * expects the first word in the string to be a valid command
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
            this->_cursor = data + arg->size() + 1;
            return Parm_t{
                .arg = (ARG_e)std::distance(arg_list.begin(), arg),
                .val = "t"};
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
                this->_cursor = ve + 1;
                return Parm_t{
                    .arg = (ARG_e)std::distance(arg_list.begin(), arg),
                    .val = std::string(vs, ve - vs)};
            }
            else
            {
                return Parm_t{
                    .arg = ARG_e::ARG_NONE,
                    .val = "f"};
            }
        }
        break;
        default:
        {
            return Parm_t{
                .arg = ARG_e::ARG_NONE,
                .val = "f"};
        }
        break;
        }
    }
    else
    {
        return Parm_t{
            .arg = ARG_e::ARG_NONE,
            .val = "none"};
    }
}

int CompanionSatellite::parseData(const char *data)
{
    if (!data)
        return -1;

    this->_cursor = data;
    int ret = 0;
    CMD_e cmd = parseCmdType(_cursor);
    while (cmd != CMD_e::CMD_NONE)
    {
        cmd_t command = {.cmd = cmd};
        Parm_t parm = parseParameters(_cursor);
        while (parm.arg != ARG_e::ARG_NONE)
        {
            command.parm.push(std::move(parm));
            parm = parseParameters(_cursor);
        }

        this->_cmd_buffer.push(std::move(command));
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

/**
 * Craft ping message.
 * @param
 * @return
 */
void CompanionSatellite::keepAlive(unsigned long timeDiff)
{
    this->pingTimeout += timeDiff;
    if (this->pingTimeout > 2100)
    {
        printf("Sending PING\n");
        this->transmitBuffer.append("PING 1234\n");
        this->pingTimeout = 0;
    }
}

/**
 * Starts and maintains the connection to Companion.
 * @param data from the establishd tcp connection
 * @return
 */
int CompanionSatellite::maintainConnection(const char *data)
{
    this->parseData(data);

    const unsigned long diff = millis() - lastmaintain;

    while (!this->_cmd_buffer.empty())
    {
        auto c = &this->_cmd_buffer.front();
        switch (this->state)
        {
        case CON_e::RECONNECT:
        case CON_e::DISCONNECTED:
            if (c->cmd != CMD_e::BEGIN)
            {
                this->state = CON_e::RECONNECT;
                this->transmitBuffer.clear();
                this->timeout = 0;
                this->pingTimeout = 0;
            }
            else
            {
                while (!c->parm.empty())
                {
                    auto p = &c->parm.front();
                    if (p->arg == ARG_e::AV)
                    {
                        std::string::size_type idx;
                        int major = std::stoi(p->val, &idx);
                        int minor = std::stoi(p->val.substr(idx + 1), &idx);
                        int patch = std::stoi(p->val.substr(idx + 1), &idx);
                        // FIXME: is this correct
                        printf("Connected to satellite version: %d %d %d\n", major, minor, patch);
                        this->state = CON_e::CONNECTED;
                        this->timeout = 0;
                        this->pingTimeout = 0;
                    }
                    c->parm.pop();
                }
            }
            break;
        case CON_e::CONNECTED:
            break;
        case CON_e::ACTIVE:
            switch (c->cmd)
            {
            case CMD_e::PONG:
                printf("PONG\n");
                this->timeout = 0;
                break;
            case CMD_e::BRIGHTNESS:
            {
                printf("BRIGHTNESS\n");
                break;
            }
            case CMD_e::KEYPRESS:
            {
                int index = -1;
                bool press = false;
                while (!c->parm.empty())
                {
                    auto p = &c->parm.front();
                    switch (p->arg)
                    {
                    case ARG_e::KEY:
                        index = std::stoi(p->val);
                        break;

                    case ARG_e::PRESSED:
                        press = (p->val == "t");
                        break;
                    }
                    c->parm.pop();
                }
                if (index > -1)
                {
                    this->keyState[index].pressed = press;
                    this->update = true;
                }
                break;
            }
            case CMD_e::KEYSTATE:
            {
                int index = -1;
                std::string color;
                std::string text;
                while (!c->parm.empty())
                {
                    auto p = &c->parm.front();
                    switch (p->arg)
                    {
                    case ARG_e::KEY:
                        index = std::stoi(p->val);
                        break;

                    case ARG_e::TYPE:
                        // TODO:
                        break;

                    case ARG_e::BITMAP:
                        // TODO:
                        break;

                    case ARG_e::COLOR:
                        color = p->val;
                        break;

                    case ARG_e::TEXT:
                        text = p->val;
                        break;
                    }
                    c->parm.pop();
                }
                if (index > -1)
                {
                    this->keyState[index].color = color;
                    this->keyState[index].text = text;
                    this->update = true;
                }
                break;
            }
            break;
            }
        case CON_e::PENDINGADD:
            if (c->cmd != CMD_e::ADDDEVICE)
            {
                this->state = CON_e::RECONNECT;
                this->transmitBuffer.clear();
            }
            else
            {
                auto p = &c->parm.front();
                if (c->cmd == CMD_e::ADDDEVICE && p->arg == ARG_e::OK)
                {
                    printf("device add OK\n");
                    this->state = CON_e::ACTIVE;
                }
                else
                {
                    printf("device add Fail\n");
                    this->state = CON_e::RECONNECT;
                    this->transmitBuffer.clear();
                }
                c->parm.pop();
            }
            break;
        }
        this->_cmd_buffer.pop();
    }

    switch (this->state)
    {
    case CON_e::CONNECTED:
        this->addDevice();
        printf("pending add\n");
        this->state = CON_e::PENDINGADD;
        this->timeout = 0;
        break;
    case CON_e::ACTIVE:
        this->keepAlive(diff);
        this->timeout += diff;
        break;
    case CON_e::PENDINGADD:
        this->timeout += diff;
        break;
    case CON_e::RECONNECT:
    case CON_e::DISCONNECTED:
        break;
    }

    if (this->timeout > 4000)
    {
        this->state = CON_e::RECONNECT;
        printf("timeout\n");
    }

    lastmaintain = millis();
    return 0;
}