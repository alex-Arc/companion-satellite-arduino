#ifndef CompanionSatellite_h
#define CompanionSatellite_h

// #include <Arduino.h>
#include <string>
#include <queue>
// #include <algorithm>

// #include <queue> // std::queue, std::swap(queue)

// #include <utility>

#include <B64.h>

class CompanionSatellite
{
private:
    std::string _deviceId;
    std::string _productName;

    struct DeviceRegisterProps
    {
        uint8_t keysTotal;
        uint8_t keysPerRow;
        bool bitmaps;
        bool color;
        bool text;
    };

    DeviceRegisterProps _props;

    enum CMD_e
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

    enum ARG_e
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

    enum CON_e
    {
        RECONNECT,
        DISCONNECTED,
        CONNECTED,
        PENDINGADD,
        ACTIVE
    };

    CON_e state = CON_e::DISCONNECTED;

    std::string _keyUpCmd;
    std::string _keyDownCmd;

    struct
    {
        ARG_e arg;
        std::string val;
    } typedef Parm_t;

    struct
    {
        const CMD_e cmd;
        std::queue<Parm_t> parm;
    } typedef cmd_t;

    CMD_e parseCmdType(const char *data);
    Parm_t parseParameters(const char *data);

    unsigned long timeout = 0;

public:
    CompanionSatellite(std::string deviceId, std::string productName, int keysTotal, int keysPerRow, bool bitmaps = false, bool color = false, bool text = false);
    CompanionSatellite(const char *deviceId, const char *productName, int keysTotal, int keysPerRow, bool bitmaps = false, bool color = false, bool text = false);

    std::string transmitBuffer;

    void keyDown(int keyIndex);
    void keyUp(int keyIndex);

    const char *_cursor = nullptr;
    std::queue<cmd_t> _cmd_buffer;

    bool isConnected() { return state >= CON_e::CONNECTED; };
    bool isActive() { return state >= CON_e::ACTIVE; };

    int parseData(const char *data);

    int maintainConnection(unsigned long elapsedTime, const char *data = nullptr);
    int disconnect();
    void addDevice();
    void ping();
};
#endif