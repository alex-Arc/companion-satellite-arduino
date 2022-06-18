#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <array>
#include <string>
#include <algorithm>
#include <iterator>
#include <map>

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

    enum CMD
    {
        NONE_CMD = -1,
        QUIT,
        PING,
        PONG,
        BEGIN
    };

    typedef enum
    {
        NONE_ARG = -1,
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
    } ARG;

    std::map<std::string, int> const stringToCommand = {
        {"BEGIN", CMD::BEGIN},
        {"QUIT", CMD::QUIT},
        {"PING", CMD::PING},
        {"PONG", CMD::PONG},
    };

    std::map<std::string, ARG> const stringToArg = {
        {"CompanionVersion", ARG::CompanionVersion},
        {"ApiVersion", ARG::ApiVersion},
        {"DEVICEID", ARG::DEVICEID},
    };

    ConnectionState _state = ConnectionState::CLOSED;

    char *_command;
    char *_values[MAX_ARGS];



    std::string *_buffer;
    std::string::iterator _offset;

    int findElement(char key, std::map<std::string, int> map, bool allowEnd = false);


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
    _buffer = &input;
    Serial.printf("All >%s<\n", _buffer->data());
    _offset = _buffer->begin();

    switch ((CMD)findElement(' ', stringToCommand))
    {
    case CMD::BEGIN:
        Serial.printf("CMD: BEGIN");
        break;
    case CMD::NONE_CMD:
        Serial.printf("CMD: NONE");
        break;
    default:
        Serial.printf("CMD: default");
        break;
    }

    // CMD cmms = phraseCommand();
    // Serial.printf("COMMAND: %d\n", cmms);

    // itr = phraseArgVal(&input, itr);
    // Serial.printf("ARG: %d\n", _arg);
    // itr = phraseArgVal(&input, itr);
    // Serial.printf("ARG: %d\n", _arg);

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

int CompanionSatellite::findElement(char key, std::map<std::string, int> map, bool allowEnd)
{
    auto out_itr = std::find(_offset, _buffer->end(), key);
    auto map_itr = map.find(std::string(_offset, out_itr));
    _offset = out_itr;
    if (map_itr != map.end())
    {
        Serial.printf("map: %d\n", map_itr->second);
        return map_itr->second;
    }
    else
    {
        return -1;
    }
}


#endif