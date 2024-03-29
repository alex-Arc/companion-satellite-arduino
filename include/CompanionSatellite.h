#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <string>
#include <vector>
#include <algorithm>

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
        std::string color = "";
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
    void handleCommand(std::string_view line);

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
    void handleAddedDevice(std::vector<parm> params);
    void handleState(std::vector<parm> params);
    void handleBrightness(std::vector<parm> params);

    int _brightness = 100;
    int _deviceStatus = 0;

    unsigned long _addDeviceTimeout;
    void _handleReceivedData(char *data);
    bool _connectionActive = false;

    std::string _keyUpCmd;
    std::string _keyDownCmd;

public:

    CompanionSatellite(std::string deviceId, std::string productName, int keysTotal, int keysPerRow, bool bitmaps=false, bool color=false, bool text=false);

    std::string transmitBuffer;

    bool update = false;

    std::vector<DeviceDrawProps> DeviceDraw;


    void keyDown(int keyIndex);
    void keyUp(int keyIndex);

    void maintain(bool clientStatus, char *data = nullptr);
};


#endif