#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <array>
#include <string>
#include <algorithm>
#include <iterator>

const int MAX_ARGS = 8;

class CompanionSatellite
{
private:
    typedef enum
    {
        CLOSED,
        INITIALIZING,
        OPEN
    } ConnectionState;

    typedef enum
    {
        BEGIN,
        QUIT,
        PING,
        PONG,
        NUMBER_OF_COMMANDS,
        NONE
    } CMD;
    std::array<std::string, CMD::NUMBER_OF_COMMANDS> commandList = {
        "BEGIN", "QUIT", "PING", "PONG"};

    typedef enum
    {
        CompanionVersion,
        ApiVersion,
        DEVICEID,
        PRODUCT_NAME,
        KEYS_TOTAL,
        BITMAPS,
        COLORS,
        TEXT,
        KEYS_PER_ROW,
        KEY,
        PRESSED,
        TYPE,
        VALUE,
        NUMBER_OF_ARGS,
        NONE_ARG
    } ARG;

    std::array<std::string, ARG::NUMBER_OF_ARGS> argList = {
        "CompanionVersion",
        "ApiVersion",
        "DEVICEID",
        "PRODUCT_NAME",
        "KEYS_TOTAL",
        "BITMAPS",
        "COLORS",
        "TEXT",
        "KEYS_PER_ROW",
        "KEY",
        "PRESSED",
        "TYPE",
        "VALUE"};

    ConnectionState _state = ConnectionState::CLOSED;

    char *_command;
    char *_values[MAX_ARGS];

    CMD _cmd;
    ARG _arg;
    ARG _val;

    std::string::iterator phraseCommand(std::string *input, std::string::iterator offset);
    std::string::iterator phraseArgVal(std::string *input, std::string::iterator offset);

public:
    const char *initialize(char *buf, uint8_t len);
    const char *maintain();
    bool connected();
};

const char *CompanionSatellite::initialize(char *buf, uint8_t len)
{
    if (_state != ConnectionState::CLOSED)
    {
        // TODO: close connection
    }

    std::string input(buf, len);
    Serial.printf("All >%s<\n", input.data());

    auto itr = phraseCommand(&input, input.begin());
    Serial.printf("COMMAND: %d\n", _cmd);
    itr = phraseArgVal(&input, itr);
    Serial.printf("ARG: %d\n", _arg);
    itr = phraseArgVal(&input, itr);
    Serial.printf("ARG: %d\n", _arg);

    return nullptr;
}

bool CompanionSatellite::connected()
{
    if (_state == ConnectionState::OPEN)
    {
        return true;
    }
    else
    {
        return false;
    }
}

std::string::iterator CompanionSatellite::phraseCommand(std::string *input, std::string::iterator offset)
{

    // CMD
    auto space = std::find(input->begin(), input->end(), ' ');
    if (space == input->end())
    {
        _cmd = CMD::NONE;
        return input->end();
    }

    *space = 0;
    auto cmd_itr = std::find(commandList.begin(), commandList.end(), input->data());
    // TODO: not end test nessesary
    if (cmd_itr != commandList.end())
    {
        _cmd = (CMD)(distance(commandList.begin(), cmd_itr));
        return std::next(space);
    }
    else
    {
        _cmd = CMD::NONE;
        return input->end();
    }
}

std::string::iterator CompanionSatellite::phraseArgVal(std::string *input, std::string::iterator offset)
{
    // ARG
    auto equal = std::find(offset, input->end(), '=');
    if (equal == input->end())
    {
        _arg = ARG::NONE_ARG;
        Serial.println("no arg");

        return input->end();
    }

    Serial.printf("Arg >%.*s<\n", distance(offset, equal), offset);

    auto arg_itr = std::find(argList.begin(), argList.end(), std::string(offset, equal));
    // TODO: not end test nessesary
    if (arg_itr != argList.end())
    {
        _arg = (ARG)(distance(argList.begin(), arg_itr));
    }
    else
    {
        _arg = ARG::NONE_ARG;
        Serial.println("no arg match");
        return input->end();
    }

    // VAL
    std::advance(equal, 1);
    auto space = std::find(equal, input->end(), ' ');
    if (space == input->end())
    {
        std::advance(space, -1);
    }
    Serial.printf("Val >%.*s<\n", distance(equal, space), equal);

    return std::next(space);
}

#endif