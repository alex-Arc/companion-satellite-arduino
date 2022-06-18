#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
// #include <array>
#include <string>
// #include <algorithm>
#include <iterator>
#include <map>

const int MAX_ARGS = 8;

namespace BEGIN
{
    enum ARG
    {
        NONE = -1,
        CompanionVersion,
        ApiVersion
    };
} // namespace BEGIN

namespace ADD_DEVICE
{
    enum ARG
    {
        NONE = -1,
        OK,
        DEVICEID
    };
} // namespace BEGIN

namespace QUIT
{
    enum
    {
        NONE = -1,
        CompanionVersion,
        ApiVersion
    };
} // namespace BEGIN

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
        NONE = -1,
        QUIT,
        PING,
        PONG,
        BEGIN,
        ADD_DEVICE,
        BOP
    };

    const std::map<std::string, int> stringToCommand = {
        {"BEGIN", CMD::BEGIN},
        {"QUIT", CMD::QUIT},
        {"PING", CMD::PING},
        {"PONG", CMD::PONG},
        {"ADD-DEVICE", CMD::ADD_DEVICE},
        {"BOP", CMD::BOP}};

    const std::map<std::string, int> stringToBeginArg = {
        {"CompanionVersion", BEGIN::ARG::CompanionVersion},
        {"ApiVersion", BEGIN::ARG::ApiVersion}};

    const std::map<std::string, int> stringToAddArg = {
        {"OK", ADD_DEVICE::ARG::OK},
        {"DEVICEID", ADD_DEVICE::ARG::DEVICEID}};

    ConnectionState _state = ConnectionState::CLOSED;

    std::string *_buffer;
    std::string::iterator _offset;

    std::string apiVersion;
    std::string companionVersion;

    int device_id = 10;

    std::string out_buffer;

    int findElement(char key, std::map<std::string, int> map, bool allowEnd = false);

public:
    const char *initialize(const char *buf, uint8_t len);
    bool connected();
};

const char *CompanionSatellite::initialize(const char *buf, uint8_t len)
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
    {
        BEGIN::ARG arg = (BEGIN::ARG)findElement('=', stringToBeginArg, true);

        while (arg != -1)
        {
            switch (arg)
            {

            case BEGIN::ARG::CompanionVersion:
            {
                Serial.printf("BEGIN::ARG::CompanionVersion ");
                auto key_itr = std::find(_offset, _buffer->end(), ' ');
                companionVersion = std::string(_offset, key_itr);
                _offset = (key_itr == _buffer->end()) ? key_itr : std::next(key_itr);
                Serial.printf(">%s<\n", companionVersion.data());
                break;
            } // BEGIN::ARG::CompanionVersion

            case BEGIN::ARG::ApiVersion:
            {
                Serial.printf("BEGIN::ARG::ApiVersion ");
                auto key_itr = std::find(_offset, _buffer->end(), ' ');
                _buffer->find_first_of(" \n", distance(_buffer->begin(), _offset));

                if (key_itr == _buffer->end())
                {
                    apiVersion = std::string(_offset, std::prev(key_itr));
                    _offset = key_itr;
                }
                else
                {
                    apiVersion = std::string(_offset, key_itr);
                    _offset = std::next(key_itr);
                }
                Serial.printf(">%s<\n", apiVersion.data());

                if (apiVersion == std::string("1.2.0"))
                {
                    out_buffer.clear();
                    // ADD-DEVICE DEVICEID=00000 PRODUCT_NAME="Satellite Streamdeck"
                    out_buffer = "ADD-DEVICE DEVICEID=" + std::to_string(device_id) + " PRODUCT_NAME=\"ESP32 Streamdeck\" KEYS_TOTAL=2 BITMAPS=FALSE COLORS=FALSE TEXT=FALSE\n";
                    Serial.printf("OUT: >%s<\n", out_buffer.data());
                    _state = ConnectionState::INITIALIZING;
                    return out_buffer.data();
                }

                break;
            } // BEGIN::ARG::ApiVersion
            case BEGIN::ARG::NONE:
            {
                Serial.printf("BEGIN::ARG::NONE\n");
                break;
            } // BEGIN::ARG::NONE_ARG

            default:
            {
                Serial.printf("default %d\n", arg);
                break;
            }
            } // switch (arg)

            arg = (BEGIN::ARG)findElement('=', stringToBeginArg, true);
        } // while (arg != -1)
        break;
    }
    case CMD::ADD_DEVICE:
    {
        Serial.printf("CMD::ADD-DEVICE\n");
        break;
    }
    case CMD::NONE:
    {
        Serial.printf("CMD::NONE\n");
        break;
    }
    default:
    {
        Serial.printf("CMD: default");
        Serial.printf("Remainder: >%.*s<\n", distance(_offset, _buffer->end()), _offset);
        break;
    }
    }
    Serial.printf("DONE\n");

    return nullptr;
}

bool CompanionSatellite::connected()
{
    if (_state != ConnectionState::CLOSED)
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
    std::string::iterator key_itr;
    if (allowEnd)
    {
        char keys[2];
        keys[0] = key;
        keys[1] = '\n';

        size_t index = _buffer->find_first_of(keys, distance(_buffer->begin(), _offset), 2);
        if (index == std::string::npos)
        {
            _offset = _buffer->end();
            return -1;
        }
        key_itr = _buffer->begin() + index;
    }
    else
    {
        size_t index = _buffer->find_first_of(&key, distance(_buffer->begin(), _offset));
        if (index == std::string::npos)
        {
            _offset = _buffer->end();
            return -1;
        }
        key_itr = _buffer->begin() + index;
    }

    auto map_itr = map.find(std::string(_offset, key_itr));

    if (key_itr == _buffer->end())
    {
        _offset = key_itr;
    }
    else
    {
        _offset = std::next(key_itr);
    }

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