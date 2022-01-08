#ifndef CompanionSatellite_h
#define CompanionSatellite_h

#include <Arduino.h>
#include <string>
#include <vector>
#include <queue>
#include <map>

// #include <etl/to_string.h>

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

    std::map<std::string, CompanionSatelliteCommand> s_mapStringToCompanionSatelliteCommand = {
        {"QUIT", CompanionSatelliteCommand::QUIT},
        {"PING", CompanionSatelliteCommand::PING},
        {"PONG", CompanionSatelliteCommand::PONG},
        {"ADD-DEVICE", CompanionSatelliteCommand::ADD_DEVICE},
        {"REMOVE-DEVICE", CompanionSatelliteCommand::REMOVE_DEVICE},
        {"KEY-PRESS", CompanionSatelliteCommand::KEY_PRESS},
        {"KEY-STATE", CompanionSatelliteCommand::KEY_STATE},
        {"KEYS-CLEAR", CompanionSatelliteCommand::KEYS_CLEAR},
        {"BRIGHTNESS", CompanionSatelliteCommand::BRIGHTNESS}};

    std::map<CompanionSatelliteCommand, std::string> s_mapCompanionSatelliteCommandToString = {
        {CompanionSatelliteCommand::QUIT, "QUIT"},
        {CompanionSatelliteCommand::PING, "PING"},
        {CompanionSatelliteCommand::PONG, "PONG"},
        {CompanionSatelliteCommand::ADD_DEVICE, "ADD-DEVICE"},
        {CompanionSatelliteCommand::REMOVE_DEVICE, "REMOVE-DEVICE"},
        {CompanionSatelliteCommand::KEY_PRESS, "KEY-PRESS"},
        {CompanionSatelliteCommand::KEY_STATE, "KEY-STATE"},
        {CompanionSatelliteCommand::KEYS_CLEAR, "KEYS-CLEAR"},
        {CompanionSatelliteCommand::BRIGHTNESS, "BRIGHTNESS"}};

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
        CompanionSatelliteCommand cmd_index;
        // char cmd[MAX_CHAR];
        std::string cmd;
        // CompanionSatellitePacketArg_t args[5];
        std::vector<std::string> key;
        std::vector<std::string> val;
    } CompanionSatellitePacket_t;

    bool initSocket();
    bool sendCommand(CompanionSatellitePacket_t);
    void readPacket();
    void parseParameters();
    void sendPing();
    void addDevice();
    void quit(boolean reconnect = false);

    CompanionSatelliteCommand stringMatchCmd(std::string const &inString);

    EthernetClient *_socket = nullptr;
    IPAddress _ip;
    uint16_t _port;

    std::queue<CompanionSatellitePacket_t> _cmdIn;
    CompanionSatellitePacket_t _pack;
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
    void keyPress(uint8_t key, uint8_t state = 2);
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

void CompanionSatellite::readPacket()
{
    int len = _socket->available();
    CompanionSatellitePacket_t pack;
    if (len)
    {
        if (len > BUFFER_SIZE)
            len = BUFFER_SIZE;
        _socket->read((uint8_t *)buf, len);

        int step = 0;
        int lastI = 0;
        boolean inQuotes = false;
        for (int i = 0; i < len; i++)
        {
            lastI = i;
            switch (step)
            {
            case 0:
                while (buf[i] != ' ' && i < len)
                    i++;

                step = 1;
                _cmdIn.push(CompanionSatellitePacket_t());
                _cmdIn.back().cmd.assign(&buf[lastI], i - lastI);
                Serial.printf("CMD: '%s'\n", _cmdIn.back().cmd.c_str());
                break;

            case 1:
                while (buf[i] != ' ' && buf[i] != '\n' && buf[i] != '=' && i < len)
                    i++;

                if (buf[i] == '=')
                {
                    step = 2;
                    _cmdIn.back().key.push_back(std::string().assign(&buf[lastI], i - lastI));
                }
                else
                {
                    (buf[i] == '\n') ? step = 3 : step = 1;

                    _cmdIn.back().key.push_back(std::string().assign(&buf[lastI], i - lastI));
                    _cmdIn.back().val.push_back(std::string().assign("true"));
                    Serial.printf("KEY:'%s' VAL:'%s'\n", _cmdIn.back().key.back().c_str(), _cmdIn.back().val.back().c_str());
                }

                break;

            case 2:
                while (buf[i] != '\n' && i < len)
                {
                    if (buf[i] == '"')
                    {
                        inQuotes = !inQuotes;
                        Serial.println("in quotes");
                    }
                    if (buf[i] == ' ' && !inQuotes)
                    {
                        break;
                    }
                    i++;
                }

                (buf[i] == '\n') ? step = 3 : step = 1;
                _cmdIn.back().val.push_back(std::string().assign(&buf[lastI], i - lastI));
                Serial.printf("KEY:'%s' VAL:'%s'\n", _cmdIn.back().key.back().c_str(), _cmdIn.back().val.back().c_str());

                break;
            default:
                break;
            }
            if (step == 3)
            {
                Serial.printf("push cmd %s \n", _cmdIn.back().cmd.c_str());

                step = 0;
            }
        }
    }
}

bool CompanionSatellite::sendCommand(CompanionSatellitePacket_t packet)
{
    Serial.println("sendCommand()");

    if (_status >= CompanionSatelliteStatus::AwaitingBegin)
    {
        Serial.print("SENDING: ");
        Serial.println(packet.cmd.c_str());

        _socket->print(packet.cmd.c_str());
        _socket->print(" ");
        for (uint8_t i = 0; i < packet.key.size(); i++)
        {
            _socket->print(packet.key[i].c_str());
            _socket->print("=");
            _socket->print(packet.val[i].c_str());
            _socket->print(" ");
        }
        _socket->print("\n");
    }
    return 1;
}

void CompanionSatellite::addDevice()
{
    Serial.println("addDevice()");
    CompanionSatellitePacket_t pack;
    pack.cmd = "ADD-DEVICE";
    pack.key.push_back(std::string("DEVICEID"));
    pack.val.push_back(std::string("00000"));

    pack.key.push_back(std::string("PRODUCT_NAME"));
    pack.val.push_back(std::string("\"Satellite Arduino\""));

    pack.key.push_back(std::string("KEYS_TOTAL"));
    pack.val.push_back(std::string("4"));

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

void CompanionSatellite::quit(boolean reconnect)
{
    Serial.println("quit()");
    CompanionSatellitePacket_t pack;
    pack.cmd = "QUIT";
    sendCommand(pack);

    if (reconnect)
        addDevice();
}

void CompanionSatellite::keyPress(uint8_t key, uint8_t state)
{
    if (_status == CompanionSatelliteStatus::Connected)
    {
        CompanionSatellitePacket_t pack;
        pack.cmd = "KEY-PRESS";
        pack.key.push_back(std::string("DEVICEID"));
        pack.val.push_back(std::string("00000"));

        pack.key.push_back(std::string("KEY"));
        char buf[18];
        itoa(key, buf, 10);
        pack.val.push_back(std::string(buf));

        pack.key.push_back(std::string("PRESSED"));
        switch (state)
        {
        case 1:
            pack.val.push_back(std::string("true"));
            break;
        case 2:
        case 0:
            pack.val.push_back(std::string("false"));
            break;
        }
        sendCommand(pack);
    }
}

int CompanionSatellite::maintain()
{
    readPacket();

    if (!_cmdIn.empty())
    {
        Serial.print("cmd in que");
        Serial.println(_cmdIn.front().cmd.c_str());
    }

    switch (_status)
    {
    case CompanionSatelliteStatus::NoConnection:
        if (_timeout > RECONNECT_DELAY)
        {
            Serial.println("timeout");
            _status = CompanionSatelliteStatus::Starting;
            _timeout = 0;
        }
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
        if (!_cmdIn.empty())
        {
            if (_cmdIn.front().cmd.compare("BEGIN") == 0 && _cmdIn.front().key[0].compare("Companion") == 0 && _cmdIn.front().key[1].compare("Version") == 0)
            {
                Serial.println("begin match");
                size_t dot1 = _cmdIn.front().val[1].find_first_of('.');
                size_t dot2 = _cmdIn.front().val[1].find_last_of('.');
                int major = atoi(_cmdIn.front().val[1].substr(dot1 - 1, 1).c_str());
                int minor = atoi(_cmdIn.front().val[1].substr(dot2 - 1, 1).c_str());
                Serial.printf("major=%d minor=%d \n", major, minor);
                if (major == VERSION_MAJOR && minor >= VERSION_MINOR)
                {
                    Serial.println("version match");
                    _status = CompanionSatelliteStatus::AddingDevice;
                    addDevice();
                }
            }
            _cmdIn.pop();
        }

        if (_timeout > RECONNECT_DELAY)
        {
            Serial.println("timeout");
            _timeout = 0;
            _status = CompanionSatelliteStatus::Starting;
        }
        break;
    }
    case CompanionSatelliteStatus::AddingDevice:
    {
        if (!_cmdIn.empty())
        {
            if (_cmdIn.front().cmd.compare("ADD-DEVICE") == 0)
            {
                if (_cmdIn.front().key[0].compare("OK") == 0)
                    _status = CompanionSatelliteStatus::Connected;
                else if (_cmdIn.front().key[0].compare("ERROR") == 0)
                {
                    Serial.println(_cmdIn.front().val[1].c_str());
                    quit(true);
                }
            }
            _cmdIn.pop();
        }
        if (_timeout > RECONNECT_DELAY)
        {
            Serial.println("timeout");
            _timeout = 0;
            _status = CompanionSatelliteStatus::Starting;
        }
    }
    case CompanionSatelliteStatus::Connected:
        if (!_cmdIn.empty())
        {
            CompanionSatelliteCommand cmd = s_mapStringToCompanionSatelliteCommand[_cmdIn.front().cmd];
            //TODO: Check device id
            switch (cmd)
            {
            case CompanionSatelliteCommand::BRIGHTNESS:

                break;
            case CompanionSatelliteCommand::KEY_STATE:

                break;
            default:
                break;
            }
            _cmdIn.pop();
        }
        break;

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