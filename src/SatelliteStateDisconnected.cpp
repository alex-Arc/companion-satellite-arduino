#include "Satellite.hpp"
#include "SatelliteStateDisconnected.hpp"

#include <string>
#include <queue>


namespace CompanionSatelliteAPI
{

    void Disconnected::begin(Satellite *sat)
    {
        sat->txBuffer.append(

            "ADD-DEVICE DEVICEID=" + sat->settings.id +
            " PRODUCT_NAME=\"" + sat->settings.name +
            "\" KEYS_TOTAL=" + std::to_string(sat->settings.keysTotal) +
            " KEYS_PER_ROW=" + std::to_string(sat->settings.keysPerRow) +
            " BITMAPS=" + ((sat->settings.bitmaps) ? "1" : "0") +
            " COLORS=" + ((sat->settings.color) ? "1" : "0") +
            " TEXT=" + ((sat->settings.text) ? "1" : "0") + "\n");
    }

} // namespace CompanionSatelliteAPI