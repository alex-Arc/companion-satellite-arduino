#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <string>
#include <queue>
#include <vector>
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
        "ADD-DEVICE",
        "BEGIN",
        "BRIGHTNESS",
        "KEY-PRESS",
        "KEY-STATE",
        "KEYS-CLEAR",
        "PING",
        "PONG",
        "REMOVE-DEVICE",
        "ROTATE"};

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
        VALUE,
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
        "VALUE",
    };

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

    struct
    {
        std::string image;
        std::string color;
        std::string text;
        bool pressed;
    } typedef keyStateProps_t;

    CMD_e parseCmdType();
    Parm_t parseParameters();

    void handleActiveConnection(cmd_t *c);
    void handleStartConnection(cmd_t *c);

    unsigned long timeout = 0;
    unsigned long pingTimeout = 0;
    unsigned long lastmaintain = millis();

    std::queue<cmd_t> _cmd_buffer;
    char *cursor = nullptr;
    int parseData();
    void addDevice();
    void keepAlive(unsigned long timeDiff);

public:
    CompanionSatellite(std::string deviceId, std::string productName, int keysTotal, int keysPerRow, bool bitmaps = false, bool color = false, bool text = false);
    CompanionSatellite(const char *deviceId, const char *productName, int keysTotal, int keysPerRow, bool bitmaps = false, bool color = false, bool text = false);

    std::string transmitBuffer;

    void keyDown(int keyIndex);
    void keyUp(int keyIndex);

    std::vector<keyStateProps_t> keyState;

    bool update = false;

    bool isConnected() { return state >= CON_e::CONNECTED; };
    bool isActive() { return state >= CON_e::ACTIVE; };

    int maintainConnection(char *data = nullptr);
    int disconnect();
};
#endif