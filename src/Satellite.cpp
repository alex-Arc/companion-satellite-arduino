#include "Satellite.hpp"
#include "SatelliteStateDisconnected.hpp"
#include "SatelliteStatePending.hpp"

#include <algorithm>
#include <cstring>
#include <queue>
#include <string>

namespace api = CompanionSatelliteAPI;

Satellite::Satellite(std::string deviceId, std::string productName, int keysTotal, int keysPerRow, bool bitmaps, bool color, bool text)
{
    this->currentState = &api::Disconnected::getInstance();

    this->settings.id = deviceId;
    this->settings.name = productName;
    this->settings.keysTotal = keysTotal;
    this->settings.keysPerRow = keysPerRow;
    this->settings.bitmaps = bitmaps;
    this->settings.color = color;
    this->settings.text = text;
}

/**
 * Transition to a new state.
 *
 * @param s string ptr to search.
 */
void Satellite::setState(api::SatelliteState &newState)
{
    this->currentState->exit(this);
    currentState = &newState;
    this->currentState->enter(this);
}

/**
 * Find the matching command.
 *
 * expects the first word in the string to be a valid command
 *
 * @param data string ptr to search.
 * @return CMD_e enum.
 */
api::CMD Satellite::parseCmdType(const char *data)
{
    while (*data == ' ' || *data == '\n')
        ++data;

    auto cmd = std::lower_bound(api::cmd_list.begin(), api::cmd_list.end(), data,
                                [](std::string from_list, const char *from_data)
                                { return (from_list.compare(0, std::string::npos, from_data, 0, from_list.size()) < 0); });

    if (cmd != api::cmd_list.end() && cmd->compare(0, std::string::npos, data, 0, cmd->size()) == 0)
    {
        this->cursor = data + cmd->size();
        return (api::CMD)std::distance(api::cmd_list.begin(), cmd);
    }
    else
    {
        return api::CMD::CMD_NONE;
    }
}

/**
 * Find Parameters.
 * @param data string ptr to search.
 * @return Parm_t.
 */
api::parm_t Satellite::parseParameters(const char *data)
{
    while (*data == ' ' || *data == '\n')
        ++data;

    auto arg = std::lower_bound(api::arg_list.begin(), api::arg_list.end(), data,
                                [](std::string from_list, const char *from_data)
                                { return (from_list.compare(0, std::string::npos, from_data, 0, from_list.size()) < 0); });

    if (arg != api::arg_list.end() && arg->compare(0, std::string::npos, data, 0, arg->size()) == 0)
    {
        switch (data[arg->size()])
        {
        case '\n':
        case ' ':
        {
            this->cursor = data + arg->size() + 1;
            return api::parm_t{
                .arg = (api::ARG)std::distance(api::arg_list.begin(), arg),
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
                this->cursor = ve + 1;
                return api::parm_t{
                    .arg = (api::ARG)std::distance(api::arg_list.begin(), arg),
                    .val = std::string(vs, ve - vs)};
            }
            else
            {
                return api::parm_t{
                    .arg = api::ARG::ARG_NONE,
                    .val = "f"};
            }
        }
        break;
        default:
        {
            return api::parm_t{
                .arg = api::ARG::ARG_NONE,
                .val = "f"};
        }
        break;
        }
    }
    else
    {
        return api::parm_t{
            .arg = api::ARG::ARG_NONE,
            .val = "none"};
    }
}

/**
 * Find commands and arguments.
 * @param data string ptr to search.
 * @return num commands.
 */
int Satellite::parseData(const char *data)
{
    if (!data)
        return -1;

    this->cursor = data;
    int ret = 0;
    api::CMD cmd = parseCmdType(cursor);
    while (cmd != api::CMD::CMD_NONE)
    {
        api::cmd_t command = {.cmd = cmd};
        api::parm_t parm = parseParameters(this->cursor);
        while (parm.arg != api::ARG::ARG_NONE)
        {
            command.parm.push(std::move(parm));
            parm = parseParameters(this->cursor);
        }

        this->cmd_buffer.push(std::move(command));
        ret++;

        cmd = parseCmdType(this->cursor);
    }

    return ret;
}

/**
 * Starts and maintains the connection to Companion.
 * @param data from the establishd tcp connection
 * @return
 */
int Satellite::maintainConnection(uint32_t elapsedTime, const char *data)
{
    this->parseData(data);

    if (this->cmd_buffer.empty())
    {
        this->keepActive(elapsedTime);
    }
    else
    {
        this->timeout = 0;
        while (!this->cmd_buffer.empty())
        {
            switch (this->cmd_buffer.front().cmd)
            {
            case api::CMD::BEGIN:
                currentState->begin(this);
                break;

            case api::CMD::ADDDEVICE:
                currentState->addDevice(this);
                break;

            case api::CMD::PONG:
                this->ping_pending = false;
                break;

            default:
                break;
            }
        }
    }
    return 0;
}

int Satellite::keepActive(uint32_t elapsedTime)
{
    timeout += elapsedTime;

    if (this->isActive())
    {
        if (ping_pending && timeout > ping_timeout)
        {
            this->setState(api::Disconnected::getInstance());
            return -1;
        }
        else if (!ping_pending && timeout > ping_time)
        {
            this->sendPing();
            return 1;
        }
    }
    else
    {
        if (timeout > ping_time)
        {
            this->setState(api::Disconnected::getInstance());
            return -1;
        }
    }

    return 0;
}