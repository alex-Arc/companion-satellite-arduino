#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <string>
#include <vector>

#define Use_NativeEthernet
#ifdef Use_NativeEthernet
#include <NativeEthernet.h>
#endif

const int BUFFER_SIZE = 64;
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

    bool initSocket();
    bool sendCommand(CompanionSatellitePacket_t);
    int readPacket();
    CompanionSatellitePacket_t parseParameters();
    void sendPing();
    void addDevice();

    EthernetClient *_socket = nullptr;
    IPAddress _ip;
    uint16_t _port;

    elapsedMillis _timeout;

    uint8_t buf[BUFFER_SIZE];

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

int CompanionSatellite::readPacket()
{
    int len = _socket->available();

    if (len)
    {
        if (len > BUFFER_SIZE)
            len = BUFFER_SIZE;
        _socket->read(buf, len);
        _str.assign((char *)buf, len);
        Serial.print(_str.c_str());
    }
    return len;
}

CompanionSatellite::CompanionSatellitePacket_t CompanionSatellite::parseParameters()
{
    CompanionSatellitePacket_t pack;
    pack.cmd = CompanionSatelliteCommand::NONE;

    int cmd_size = _str.find_first_not_of(" ");
    if (cmd_size){
        Serial.println(_str.substr(0, cmd_size).c_str());
        // switch (_str.substr(0, cmd_size))
        // {
        // case /* constant-expression */:
        //     /* code */
        //     break;
        
        // default:
        //     break;
        // }
    }

    return pack;
}

bool CompanionSatellite::sendCommand(CompanionSatellitePacket_t packet)
{
    Serial.println("sendCommand()");

    if (_status >= CompanionSatelliteStatus::AwaitingBegin)
    {
        Serial.print("SENDING: ");

        _socket->print(CMDS[packet.cmd].c_str());
        Serial.print(CMDS[packet.cmd].c_str());
        _socket->print(" ");
        Serial.print(" ");
        for (std::vector<std::string>::iterator it = packet.args.begin(); it != packet.args.end(); ++it)
        {
            _socket->print(it->c_str());
            Serial.print(it->c_str());

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
    CompanionSatellitePacket_t pack;
    pack.cmd = CompanionSatelliteCommand::ADD_DEVICE;
    pack.args.push_back("DEVICEID=00000");
    pack.args.push_back("PRODUCT_NAME=\"Satellite Arduino\"");
    pack.args.push_back("KEYS_TOTAL=2");
    pack.args.push_back("KEYS_PER_ROW=2");
    pack.args.push_back("BITMAPS=false");
    pack.args.push_back("COLORS=true");
    pack.args.push_back("TEXT=false");
    sendCommand(pack);
}

int CompanionSatellite::maintain()
{
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
        if (readPacket())
        {
            if (_str.substr(0, 23).compare("BEGIN Companion Version") == 0)
            {
                Serial.println("begin match");
                size_t dot1 = _str.find_first_of('.') + 1;
                size_t dot2 = _str.find_last_of('.') + 1;
                size_t dash1 = _str.find_first_of('-') + 1;
                std::string major = _str.substr(24, 1);
                std::string minor = _str.substr(dot1, 1);
                std::string patch = _str.substr(dot2, 1);

                Serial.printf("major=%s minor=%s patch=%s \n", major.c_str(), minor.c_str(), patch.c_str());

                if (atoi(major.c_str()) == VERSION_MAJOR && atoi(minor.c_str()) >= VERSION_MINOR)
                {
                    Serial.println("version match");
                    _status = CompanionSatelliteStatus::AddingDevice;
                    addDevice();
                }
            }
        }
        if (_timeout > RECONNECT_DELAY)
            _status = CompanionSatelliteStatus::Starting;
        break;
    }
    case CompanionSatelliteStatus::AddingDevice:
    {
        if (readPacket())
        {
            parseParameters();
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