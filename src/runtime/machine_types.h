#pragma once

#include <QMetaType>

namespace RuntimeInit {
inline constexpr int kTemperature = 25;
inline constexpr int kPressure = 80;
inline constexpr int kSpeed = 0;
} // namespace RuntimeInit

enum class MachineState { Idle, Starting, Running, Stopping, Fault };

struct TelemetryFrame
{
    double temperature{RuntimeInit::kTemperature};
    double pressure{RuntimeInit::kPressure};
    int speed{RuntimeInit::kSpeed};
};

Q_DECLARE_METATYPE(MachineState)
Q_DECLARE_METATYPE(TelemetryFrame)
