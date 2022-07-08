#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <string>
#include <vector>
#include <algorithm>

#include <utility>

#include <string_view>
#include <charconv>

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
        int keysTotal;
        int keysPerRow;
        bool bitmaps;
        bool color;
        bool text;
    };

    DeviceRegisterProps _props;

    unsigned long _lastReceivedAt;
    std::string_view receiveBuffer;

    struct parm
    {
        std::string_view key;
        std::string_view val;
    };

    std::vector<parm> parseLineParameters(std::string_view line);
    void handleCommand(std::pair<const char *, const char *> line);

    int findInCmdList(std::pair<const char*, const char *> data);

    const std::vector<std::string>cmd_list = {
        "ADD-DEVICE",
        "BEGIN",
        "BRIGHTNESS",
        "KEY-PRESS",
        "KEY-STATE",
        "KEYS-CLEAR",
        "PING",
        "PONG",
        "REMOVE-DEVICE"
        };

    const std::vector<std::string> commandList = {
        "PING",
        "PONG",
        "KEY-STATE",
        "KEYS-CLEAR",
        "BRIGHTNESS",
        "ADD-DEVICE",
        "REMOVE-DEVICE",
        "BEGIN",
        "KEY-PRESS"};

    void addDevice();
    void removeDevice();
    void handleAddedDevice(std::vector<parm> params);
    void handleState(std::vector<parm> params);
    void handleBrightness(std::vector<parm> params);

    int _brightness = 100;
    int _deviceStatus = 0;

    unsigned long _addDeviceTimeout = millis();
    void _handleReceivedData(char *data);
    bool _connectionActive = false;

    std::string _keyUpCmd;
    std::string _keyDownCmd;

public:
    CompanionSatellite(std::string deviceId, std::string productName, int keysTotal, int keysPerRow, bool bitmaps = false, bool color = false, bool text = false);

    std::string transmitBuffer;

    bool update = false;

    std::vector<DeviceDrawProps> DeviceDraw;

    void keyDown(int keyIndex);
    void keyUp(int keyIndex);

    void maintain(bool clientStatus, char *data = nullptr);
};

#endif