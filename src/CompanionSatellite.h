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

    ConnectionState _state = CLOSED;

    char *_command;
    char *_values[MAX_ARGS];
    bool phrasesCommand(char *buf, uint8_t len);

    // char *findArray();

    std::array<std::string, 4> commandList = {"BEGIN", "QUIT", "PING", "PONG"};

public:
    const char *initialize(char *buf, uint8_t len);
    bool connected();
};

const char *CompanionSatellite::initialize(char *buf, uint8_t len)
{
    if (_state != CompanionSatellite::ConnectionState::CLOSED)
    {
        // TODO: close connection
    }

    phrasesCommand(buf, len);

    return nullptr;
}

bool CompanionSatellite::connected()
{
    if (_state == CompanionSatellite::ConnectionState::OPEN)
    {
        return true;
    }
    else
    {
        return false;
    }
}

bool CompanionSatellite::phrasesCommand(char *buf, uint8_t len)
{
    std::string input(buf, len);
    Serial.printf("All >%s<\n", input.c_str());

    // CMD
    auto space = std::find(input.begin(), input.end(), ' ');
    if (space == input.end())
        return 0;

    Serial.printf("Command >%.*s<\n", distance(input.begin(), space), input.data());

    *space = 0;
    auto cmd_itr = std::find(std::begin(commandList), std::end(commandList), input.data());
    if (cmd_itr != std::end(commandList))
    {
        Serial.printf("Element %s at %d\n", input.c_str(), distance(std::begin(commandList), cmd_itr));
    }
    else
    {
        Serial.println("Element is not present in the given array");
        return 0;
    }

    // ARG
    auto equal = std::find(space, input.end(), '=');
    if (equal == input.end())
        return 0;

    Serial.printf("Arg >%.*s<\n", distance(space + 1, equal), space + 1);

    // VAL
    space = std::find(equal, input.end(), ' ');
    if (space == input.end())
        return 0;

    Serial.printf("Val >%.*s<\n", distance(equal + 1, space), equal + 1);

    // std::string arg = input.substr(space + 1, std::distance(space, equal));
    // Serial.printf("Arg >%s<\n", arg.c_str());

    /*
        Serial.printf("All >%.*s<\n", len, buf);
        char *lf, *equal, *space;

        lf = (char *)memchr(buf, '\n', len);

        if (lf == nullptr)
        {
            return false;
        }

        space = (char *)memchr(buf, ' ', len);

        Serial.printf("Command >%.*s<\n", space - buf, buf);

        *space = 0;

        auto itr = std::find(std::begin(commandList), std::end(commandList), buf);

        if (itr != std::end(commandList))
        {
            Serial.printf("Element %s at %d\n", buf, distance(std::begin(commandList), itr));
        }
        else
        {
            Serial.println("Element is not present in the given array");
            return 0;
        }

        for (int i = 0; i < MAX_ARGS; i++)
        {
            equal = (char *)memchr(space, '=', len - (space - buf));
            Serial.printf("Arg >%.*s< ", equal - space - 1, space + 1);

            space = (char *)memchr(equal, ' ', len - (equal - buf));
            if (space != nullptr)
            {
                Serial.printf("Val >%.*s<\n", space - equal - 1, equal + 1);
            }
            else
            {
                Serial.printf("Val >%.*s<\n", lf - equal - 1, equal + 1);
                break;
            }
        }
    */
    return 0;
}

#endif