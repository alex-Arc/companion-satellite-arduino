#pragma once

#include "Satellite.hpp"

#include <string>
#include <queue>

// Forward declaration to resolve circular dependency/include
class Satellite;

namespace CompanionSatelliteAPI
{

    class SatelliteState
    {
    private:
        /* data */
    public:
        virtual void enter(Satellite *sat) = 0;
        virtual void exit(Satellite *sat) = 0;

        bool isConnected() { return false; };
        bool isActive() { return false; };

        virtual void addDevice() {}
        virtual void begin(Satellite *sat) {}
        virtual void brightness() {}
        virtual void keyPress() {}
        virtual void keystate() {}
        virtual void keysClear() {}
        virtual void ping() {}
        virtual void pong() {}
        virtual void removedevice() {}
        virtual void rotate() {}
    };

    enum CMD
    {
        CMD_NONE = -1,
        ADDDEVICE,
        BEGIN,
        BRIGHTNESS,
        KEYPRESS,
        KEYSTATE,
        KEYSCLEAR,
        PING,
        PONG,
        REMOVEDEVICE,
        ROTATE
    };

    const std::vector<std::string> cmd_list = {
        "ADD-DEVICE ",
        "BEGIN ",
        "BRIGHTNESS ",
        "KEY-PRESS ",
        "KEY-STATE ",
        "KEYS-CLEAR ",
        "PING ",
        "PONG ",
        "REMOVE-DEVICE ",
        "ROTATE "};

    enum ARG
    {
        ARG_NONE = -1,
        AV,
        BITMAP,
        COLOR,
        CV,
        DEVICEID,
        DIRECTION,
        ERROR,
        KEY,
        OK,
        PRESSED,
        TEXT,
        TYPE,
        VALUE
    };

    const std::vector<std::string> arg_list = {
        "ApiVersion",
        "BITMAP",
        "COLOR",
        "CompanionVersion",
        "DEVICEID",
        "DIRECTION",
        "ERROR",
        "KEY",
        "OK",
        "PRESSED",
        "TEXT",
        "TYPE",
        "VALUE"};

    struct
    {
        ARG arg;
        std::string val;
    } typedef parm_t;

    struct
    {
        const CMD cmd;
        std::queue<parm_t> parm;
    } typedef cmd_t;

    struct
    {
        uint8_t keysTotal;
        uint8_t keysPerRow;
        bool bitmaps;
        bool color;
        bool text;
        std::string id;
        std::string name;
    } typedef DeviceSettings_t;

} // namespace CompanionSatelliteAPI