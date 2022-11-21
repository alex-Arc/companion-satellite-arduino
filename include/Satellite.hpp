#pragma once

#include "SatelliteState.hpp"
#include "SatelliteStateDisconnected.hpp"
#include "SatelliteStatePending.hpp"


#include <string>
#include <queue>

namespace api = CompanionSatelliteAPI;

// Forward declaration to resolve circular dependency/include
// class api::SatelliteState;
// class api::Disconnected;

class Satellite
{
private:
    api::SatelliteState *currentState;

    api::DeviceSettings_t settings;

    const char *cursor;

    void setState(api::SatelliteState &newState);
    inline api::SatelliteState *getCurrentState() const { return currentState; }

    api::CMD parseCmdType(const char *data);
    api::parm_t parseParameters(const char *data);

    std::queue<api::cmd_t> cmd_buffer;

    friend class api::SatelliteState;
    friend class api::Disconnected;
    friend class api::Pending;

public:
    std::string txBuffer;

    bool isConnected() { return currentState->isConnected(); };
    bool isActive() { return currentState->isActive(); };

    int parseData(const char *data);
    int maintainConnection(unsigned long elapsedTime, const char *data = nullptr);

    Satellite(std::string deviceId, std::string productName, int keysTotal, int keysPerRow, bool bitmaps = false, bool color = false, bool text = false);
};
