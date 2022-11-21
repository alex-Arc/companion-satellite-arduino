#include "Satellite.hpp"
#include "SatelliteState.hpp"

#include <string>
#include <queue>

namespace CompanionSatelliteAPI
{

    void SatelliteState::removeDevice(Satellite *sat)
    {

        while (!sat->cmd_buffer.empty())
            sat->cmd_buffer.pop();
        sat->timeout = 0;
        sat->txBuffer.assign("REMOVE-DEVICE DEVICEID=" + sat->settings.id);
    }
} // namespace CompanionSatelliteAPI