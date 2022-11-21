#include "Satellite.hpp"
#include "SatelliteStateDisconnected.hpp"

#include <string>
#include <queue>

namespace CompanionSatelliteAPI
{
    void Disconnected::enter(Satellite *sat)
    {
        sat->txBuffer.clear();
        sat->ping_pending = 0;
    }

    void Disconnected::begin(Satellite *sat)
    {
        bool version_match = false;

        while (!sat->cmd_buffer.front().parm.empty())
        {
            auto p = &sat->cmd_buffer.front().parm.front();
            if (p->arg == api::ARG::AV)
            {
                std::string::size_type idx;
                int major = std::stoi(p->val, &idx);
                int minor = std::stoi(p->val.substr(idx + 1), &idx);
                // FIXME: gets minor???
                //  int patch = std::stoi(p.val.substr(idx+1), &idx);
                if (major == 1 && minor >= 2)
                {
                    sat->txBuffer.append(
                        "ADD-DEVICE DEVICEID=" + sat->settings.id +
                        " PRODUCT_NAME=\"" + sat->settings.name +
                        "\" KEYS_TOTAL=" + std::to_string(sat->settings.keysTotal) +
                        " KEYS_PER_ROW=" + std::to_string(sat->settings.keysPerRow) +
                        " BITMAPS=" + ((sat->settings.bitmaps) ? "1" : "0") +
                        " COLORS=" + ((sat->settings.color) ? "1" : "0") +
                        " TEXT=" + ((sat->settings.text) ? "1" : "0") + "\n");
                    sat->setState(Pending::getInstance());
                }
            }
            sat->cmd_buffer.front().parm.pop();
        }

        while (!sat->cmd_buffer.empty())
            sat->cmd_buffer.pop();
    }

} // namespace CompanionSatelliteAPI