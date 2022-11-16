#ifndef CompanionSatellite_h
#define CompanionSatellite_h

// #include <Arduino.h>
#include <string>
#include <vector>
// #include <algorithm>

// #include <queue> // std::queue, std::swap(queue)

// #include <utility>

#include <B64.h>

class CompanionSatellite
{
private:
    std::string _deviceId;
    std::string _productName;

    struct DeviceDrawProps
    {
        std::string image = "";
        union
        {
            uint32_t color;
            struct
            {
                uint8_t blue;
                uint8_t green;
                uint8_t red;
                uint8_t nan;
            };
        };
        std::string text = "";
        bool pressed = false;
    };

    struct DeviceRegisterProps
    {
        uint8_t keysTotal;
        uint8_t keysPerRow;
        bool bitmaps;
        bool color;
        bool text;
    };

    DeviceRegisterProps _props;

    unsigned long _lastReceivedAt;

    const char *true_val = "1";
    const char *DEVICEID_key = "DEVICEID";

    struct parm
    {
        const char *key;
        const char *val;
    };

    std::vector<parm> parseLineParameters(char *line);
    void handleCommand(char *line);

    int findInCmdList(char *data);

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
        REMOVEDEVICE
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
        "REMOVE-DEVICE "};

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
        TYPE
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
        "TYPE"};

    void removeDevice();
    void handleAddedDevice(std::vector<parm> params);
    void handleState(std::vector<parm> params);
    void handleBrightness(std::vector<parm> params);

    int _brightness = 100;
    int _deviceStatus = 0;

    unsigned long _addDeviceTimeout = 0; // millis();
    void _handleReceivedData(char *data, size_t len);
    bool _connectionActive = false;
    std::string _keyUpCmd;
    std::string _keyDownCmd;

public:
    CompanionSatellite(std::string deviceId, std::string productName, int keysTotal, int keysPerRow, bool bitmaps = false, bool color = false, bool text = false);
    CompanionSatellite(const char *deviceId, const char *productName, int keysTotal, int keysPerRow, bool bitmaps = false, bool color = false, bool text = false);

    std::string transmitBuffer;

    bool update = false;

    std::vector<DeviceDrawProps> DeviceDraw;

    void keyDown(int keyIndex);
    void keyUp(int keyIndex);

    void maintain(bool clientStatus, char *data = nullptr, size_t len = 0);

    struct
    {
        ARG_e arg;
        std::string val;
    } typedef Parm_t;

    struct
    {
        const CMD_e cmd;
        std::vector<Parm_t> parm;
    }typedef cmd_t;

    std::vector<cmd_t> _cmd_buffer;

    const char *_cursor = nullptr;

    CMD_e parseCmdType(const char *data);
    Parm_t parseParameters(const char *data);
    int parseData(const std::string data);

    void addDevice();
};
#endif