#pragma once

#include "SatelliteState.hpp"
#include "Satellite.hpp"

#include <string>
#include <queue>

namespace CompanionSatelliteAPI
{

    class Pending : public SatelliteState
    {
    private:
        Pending() {}
        Pending(const Pending &other);
        Pending &operator=(const Pending &other);

    public:
        void enter(Satellite *sat) {}
        void exit(Satellite *sat) {}

        bool isConnected() override { return true; };
        virtual std::string getStateName() { return std::string("Pending"); };


        // void begin(Satellite *sat) override;

        static Pending &getInstance()
        {
            static Pending singleton;
            return singleton;
        }
    };
} // namespace CompanionSatelliteAPI