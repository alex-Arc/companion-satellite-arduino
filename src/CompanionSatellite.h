#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <string>

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
    bool initSocket();
    int readPacket();
    void sendPing();
    void addDevice();

    EthernetClient *_socket = nullptr;
    IPAddress _ip;
    uint16_t _port;

    elapsedMillis _timeout;

    uint8_t buf[BUFFER_SIZE];
    std::string _str;

    enum CompanionSatelliteStatus
    {
        NoConnection = -2,
        VersionMiss = -1,
        Starting = 1,
        ConnectionActive,
        AwaitingBegin,
        Connected,
    };

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

                Serial.printf("major=%s minor=%s patch=%s", major.c_str(), minor.c_str(), patch.c_str());

                if (atoi(major.c_str()) == VERSION_MAJOR && atoi(minor.c_str()) >= VERSION_MINOR)
                {
                    _status = CompanionSatelliteStatus::Connected;

                }
            }
        }
        if (_timeout > RECONNECT_DELAY)
            _status = CompanionSatelliteStatus::Starting;
        break;
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