#include <Arduino.h>
#include <CompanionSatellite.h>

#include <cstring>
#include <string>
#include <queue>

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

void CompanionSatellite::keyDown(int keyIndex)
{
    if (this->state == CON_e::ACTIVE)
        this->transmitBuffer.append(this->_keyDownCmd + std::to_string(keyIndex) + '\n');

    printf("%s\n", this->transmitBuffer.c_str());
}

void CompanionSatellite::keyUp(int keyIndex)
{
    if (this->state == CON_e::ACTIVE)
        this->transmitBuffer.append(this->_keyUpCmd + std::to_string(keyIndex) + '\n');

    printf("%s\n", this->transmitBuffer.c_str());
}

/**
 * Find the matching command.
 *
 * expects the first word in the string to be a valid command
 *
 * @param data string ptr to search.
 * @return CMD_e enum.
 */
CompanionSatellite::CMD_e CompanionSatellite::parseCmdType()
{
    if (*this->cursor == '\0')
    {
        return CompanionSatellite::CMD_e::CMD_NONE;
    }

    const char *token = strtok_r(this->cursor, " ", &this->cursor);
    for (unsigned int i = 0; i < this->cmd_list.size(); i++)
    {
        if (strcmp(token, this->cmd_list[i].c_str()) == 0)
        {
            return CompanionSatellite::CMD_e(i);
        }
    }
    return CompanionSatellite::CMD_e::CMD_NONE;
}

/**
 * Find Parameters.
 * @param data string ptr to search.
 * @return Parm_t.
 */
CompanionSatellite::Parm_t CompanionSatellite::parseParameters()
{
    if (*this->cursor == '\0')
    {
        return Parm_t{.arg = ARG_e::ARG_NONE, .val = "none"};
    }

    auto breakPoint = std::strcspn(this->cursor, " \n=");

    for (unsigned int i = 0; i < this->arg_list.size(); i++)
    {
        if (strncmp(this->cursor, this->arg_list[i].c_str(), breakPoint) == 0)
        {
            this->cursor += breakPoint;
            switch (this->cursor[0])
            {
            case ' ':
            case '\n':
                ++this->cursor;
                return Parm_t{.arg = CompanionSatellite::ARG_e(i), .val = "true"};
            case '=':
            {
                this->cursor++;
                char delimiter[] = " \n";
                if (this->cursor[0] == '"')
                {
                    this->cursor++;
                    delimiter[0] = '\"';
                }
                const char *token = strtok_r(this->cursor, delimiter, &this->cursor);
                if (token == nullptr)
                {
                    this->cursor = nullptr;
                    return Parm_t{.arg = ARG_e::ARG_NONE, .val = "none"};
                }
                return Parm_t{
                    .arg = CompanionSatellite::ARG_e(i),
                    .val = token};
            }
            }
        }
    }

    return Parm_t{
        .arg = ARG_e::ARG_NONE,
        .val = "none"};
}

int CompanionSatellite::parseData()
{
    if (this->cursor == nullptr || *this->cursor == '\0')
        return -1;

    int ret = 0;
    CMD_e cmd = parseCmdType();
    while (cmd != CMD_e::CMD_NONE)
    {
        cmd_t command = {.cmd = cmd};
        Parm_t parm = parseParameters();
        while (parm.arg != ARG_e::ARG_NONE)
        {
            command.parm.push(std::move(parm));
            parm = parseParameters();
        }

        this->_cmd_buffer.push(std::move(command));
        ret++;

        cmd = parseCmdType();
    }

    this->cursor = nullptr;
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
void CompanionSatellite::handleStartConnection(CompanionSatellite::cmd_t *c)
{
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
}

void CompanionSatellite::handleActiveConnection(CompanionSatellite::cmd_t *c)
{
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
        bool press = false;
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
                text = B64::decode(p->val);
                break;

            case ARG_e::PRESSED:
                press = (p->val == "true");
                break;
            }
            c->parm.pop();
        }
        if (index > -1)
        {
            this->keyState[index].color = color;
            this->keyState[index].text = text;
            this->keyState[index].pressed = press;
            this->update = true;
        }
        break;
    }
    break;
    }
}

/**
 * Starts and maintains the connection to Companion.
 * @param data from the establishd tcp connection
 * @return
 */
int CompanionSatellite::maintainConnection(char *data)
{
    this->cursor = data;
    this->parseData();

    const unsigned long diff = millis() - lastmaintain;

    while (!this->_cmd_buffer.empty())
    {
        auto c = &this->_cmd_buffer.front();
        switch (this->state)
        {
        case CON_e::RECONNECT:
        case CON_e::DISCONNECTED:
            this->handleStartConnection(c);
            break;
        case CON_e::CONNECTED:
            break;
        case CON_e::ACTIVE:
            this->handleActiveConnection(c);
            break;
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
        if (this->state > CON_e::DISCONNECTED)
            printf("timeout\n");

        this->state = CON_e::RECONNECT;
    }

    lastmaintain = millis();
    return 0;
}