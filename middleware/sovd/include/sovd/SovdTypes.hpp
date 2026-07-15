#pragma once

#include <cstdint>
#include <string>
#include <vector>

namespace sdv::sovd {

// ISO 17879 §7.3 error model
struct SovdError {
    int         code;
    std::string message;
    std::string detail;
};

struct DtcEntry {
    uint32_t    dtcCode;
    std::string description;
    std::string severity;   // "info" | "warning" | "error" | "critical"
    uint64_t    timestampUs;
    float       odometer;
};

struct DataElement {
    std::string name;
    std::string value;
    std::string unit;
    uint64_t    timestampUs{0};
};

enum class RoutineState { Idle, Pending, Running, Completed, Failed };

struct RoutineStatus {
    std::string  routineId;
    RoutineState state{RoutineState::Idle};
    uint8_t      progressPercent{0};
    std::string  resultMessage;
};

// SOVD role for access control (ISO 17879 §8)
enum class SovdRole {
    Unauthenticated,
    Technician,
    OemSupport,
    Supplier
};

} // namespace sdv::sovd
