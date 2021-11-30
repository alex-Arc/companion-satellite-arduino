#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <string>
#include <vector>
#include <queue>

#define Use_NativeEthernet
#ifdef Use_NativeEthernet
#include <NativeEthernet.h>
#endif

const int BUFFER_SIZE = 254;
const int MAX_CHAR = 16;
const int RECONNECT_DELAY = 1000;
const int VERSION_MAJOR = 2;
const int VERSION_MINOR = 2;
const int VERSION_PATCH = 0;

class CompanionSatellite
{
private:
    const std::vector<std::string> CMDS = {
        "QUIT",
        "PING",
        "PONG",
        "ADD-DEVICE",
        "REMOVE-DEVICE",
        "KEY-PRESS",
        "KEY-STATE",
        "KEYS-CLEAR",
        "BRIGHTNESS"};

    enum CompanionSatelliteCommand
    {
        NONE = -1,
        QUIT,
        PING,
        PONG,
        ADD_DEVICE,
        REMOVE_DEVICE,
        KEY_PRESS,
        KEY_STATE,
        KEYS_CLEAR,
        BRIGHTNESS,
    };

    enum CompanionSatelliteStatus
    {
        NoConnection = -2,
        VersionMiss = -1,
        Starting = 1,
        ConnectionActive,
        AwaitingBegin,
        AddingDevice,
        Connected,
    };

    typedef struct
    {
        CompanionSatelliteCommand cmd;
        std::vector<std::string> args;
    } CompanionSatellitePacket_t;

    // typedef struct
    // {
    //     char key[MAX_CHAR];
    //     char val[MAX_CHAR];
    // } CompanionSatellitePacketArg_t;

    typedef struct
    {
        CompanionSatelliteCommand cmd_index;
        // char cmd[MAX_CHAR];
        std::string cmd;
        // CompanionSatellitePacketArg_t args[5];
        std::vector<std::string> key;
        std::vector<std::string> val;
    } CompanionSatellitePacket2_t;

    bool initSocket();
    bool sendCommand(CompanionSatellitePacket2_t);
    CompanionSatellitePacket2_t readPacket();
    void parseParameters();
    void sendPing();
    void addDevice();

    EthernetClient *_socket = nullptr;
    IPAddress _ip;
    uint16_t _port;

    std::queue<CompanionSatellitePacket_t> _cmdIn;
    CompanionSatellitePacket2_t _pack;
    elapsedMillis _timeout;

    char buf[BUFFER_SIZE];

    //TODO: maybe pass it as a parmater
    std::string _str;

    CompanionSatelliteStatus _status;

public:
    CompanionSatellite(EthernetClient *socket);
    ~CompanionSatellite();

    bool connected = false;

    void connect(IPAddress ip, uint16_t port = 16622);
    int maintain();
};

CompanionSatellite::CompanionSatellite(EthernetClient *socket)
{
    _socket = socket;
}

CompanionSatellite::~CompanionSatellite()
{
    if (_socket != nullptr)
        _socket->close();
}

CompanionSatellite::CompanionSatellitePacket2_t CompanionSatellite::readPacket()
{
    int len = _socket->available();
    CompanionSatellitePacket2_t pack;
    if (len)
    {
        if (len > BUFFER_SIZE)
            len = BUFFER_SIZE;
        _socket->read((uint8_t *)buf, len);

        int step = 0;
        int lastI = 0;
        for (int i = 0; i < len; i++)
        {
            lastI = i;
            switch (step)
            {
            case 0:
                while (buf[i] != ' ' && i < len)
                    i++;

                step = 1;
                pack.cmd.assign(&buf[lastI], i - lastI);
                Serial.printf("CMD: '%s'\n", pack.cmd.c_str());
                break;

            case 1:
                while (buf[i] != ' ' && buf[i] != '\n' && buf[i] != '=' && i < len)
                    i++;

                if (buf[i] == '=')
                {
                    step = 2;
                    pack.key.push_back(std::string().assign(&buf[lastI], i - lastI));
                }
                else
                {
                    (buf[i] == '\n') ? step = 0 : step = 1;

                    pack.key.push_back(std::string().assign(&buf[lastI], i - lastI));
                    pack.val.push_back(std::string().assign("true"));
                    Serial.printf("KEY:'%s' VAL:'%s'\n", pack.key.back().c_str(), pack.val.back().c_str());
                }

                break;

            case 2:
                while (buf[i] != ' ' && buf[i] != '\n' && buf[i] != '"' && i < len)
                    i++;
                //TODO: take care of spaces in ""
                if (buf[i] != '"')
                {
                    (buf[i] == '\n') ? step = 0 : step = 1;
                    pack.val.push_back(std::string().assign(&buf[lastI], i - lastI));
                    Serial.printf("KEY:'%s' VAL:'%s'\n", pack.key.back().c_str(), pack.val.back().c_str());
                }
                break;

            default:
                break;
            }
        }
    }
    return pack;
}

//TODO: move to read and do it in stream
void CompanionSatellite::parseParameters()
{
    Serial.println("parseParameters()");
}

bool CompanionSatellite::sendCommand(CompanionSatellitePacket2_t packet)
{
    Serial.println("sendCommand()");

    if (_status >= CompanionSatelliteStatus::AwaitingBegin)
    {
        Serial.print("SENDING: ");

        _socket->print(packet.cmd.c_str());
        Serial.print(packet.cmd.c_str());
        _socket->print(" ");
        Serial.print(" ");
        for (int i = 0; i < packet.key.size(); i++)
        {
            _socket->print(packet.key[i].c_str());
            Serial.print(packet.key[i].c_str());
            _socket->print("=");
            Serial.print("=");
            _socket->print(packet.val[i].c_str());
            Serial.print(packet.val[i].c_str());
            _socket->print(" ");
            Serial.print(" ");
        }
        _socket->print("\n");
        Serial.print("\n");
    }
    return 1;
}

void CompanionSatellite::addDevice()
{
    Serial.println("addDevice()");
    CompanionSatellitePacket2_t pack;
    pack.cmd = "ADD-DEVICE";
    pack.key.push_back(std::string("DEVICEID"));
    pack.val.push_back(std::string("00000"));

    pack.key.push_back(std::string("PRODUCT_NAME"));
    pack.val.push_back(std::string("\"Satellite Arduino\""));

    pack.key.push_back(std::string("KEYS_TOTAL"));
    pack.val.push_back(std::string("2"));

    pack.key.push_back(std::string("KEYS_PER_ROW"));
    pack.val.push_back(std::string("2"));

    pack.key.push_back(std::string("BITMAPS"));
    pack.val.push_back(std::string("false"));

    pack.key.push_back(std::string("COLORS"));
    pack.val.push_back(std::string("true"));

    pack.key.push_back(std::string("TEXT"));
    pack.val.push_back(std::string("false"));

    sendCommand(pack);
}

int CompanionSatellite::maintain()
{
    _pack = readPacket();

    switch (_status)
    {
    case CompanionSatelliteStatus::NoConnection:
        if (_timeout > RECONNECT_DELAY)
            _status = CompanionSatelliteStatus::Starting;
        break;
    case CompanionSatelliteStatus::Starting:
    {
        if (_socket->connect(_ip, _port))
        {
            _status = CompanionSatelliteStatus::AwaitingBegin;
            _timeout = 0;
        }
        else
        {
            _status = CompanionSatelliteStatus::NoConnection;
        }
        break;
    }
    case CompanionSatelliteStatus::AwaitingBegin:
    {
        if (_pack.cmd.compare("BEGIN") == 0 && _pack.key[0].compare("Companion") == 0 && _pack.key[1].compare("Version") == 0)
        {
            Serial.println("begin match");
            size_t dot1 = _pack.val[1].find_first_of('.');
            size_t dot2 = _pack.val[1].find_last_of('.');
            // size_t dash1 = _pack.val[1].find_first_of('-') + 1;
            int major = atoi(_pack.val[1].substr(dot1 - 1, 1).c_str());
            int minor = atoi(_pack.val[1].substr(dot1 - 1, 1).c_str());
            // std::string patch = _pack.val[1].substr(dot2+1, 1);
            Serial.printf("major=%d minor=%d patch=%s \n", major, minor);
            if (major == VERSION_MAJOR && minor >= VERSION_MINOR)
            {
                Serial.println("version match");
                _status = CompanionSatelliteStatus::AddingDevice;
                addDevice();
            }
        }

        if (_timeout > RECONNECT_DELAY)
            _status = CompanionSatelliteStatus::Starting;
        break;
    }
    case CompanionSatelliteStatus::AddingDevice:
    {
        if (_pack.cmd.compare("ADD-DEVICE"))
        {
            // parseParameters();
        }
        // if (_timeout > RECONNECT_DELAY)
        //     _status = CompanionSatelliteStatus::Starting;
    }

    default:
        break;
    }

    return _status;
}

void CompanionSatellite::connect(IPAddress ip, uint16_t port)
{
    if (_status > 0)
        _socket->close();

    _status = CompanionSatelliteStatus::Starting;

    _ip = ip;
    _port = port;

    maintain();
}

#endif