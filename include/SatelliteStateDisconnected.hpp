#pragma once

#include "SatelliteState.hpp"
#include "Satellite.hpp"

#include <string>
#include <queue>

class Satellite;

namespace CompanionSatelliteAPI
{
    class Disconnected : public SatelliteState
    {
    private:
        Disconnected() {}
        Disconnected(const Disconnected &other);
        Disconnected &operator=(const Disconnected &other);

    public:
        void enter(Satellite *sat) {}
        void exit(Satellite *sat) {}


        void begin(Satellite *sat) override;

        static Disconnected &getInstance()
        {
            static Disconnected singleton;
            return singleton;
        }
    };
} // namespace CompanionSatelliteAPI