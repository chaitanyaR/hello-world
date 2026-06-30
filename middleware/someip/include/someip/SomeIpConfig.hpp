#pragma once

#include <string>

namespace sdv::someip {

// Loads the vsomeip JSON configuration for this node.
// The VSOMEIP_CONFIGURATION environment variable takes precedence;
// configPath is used as a fallback for tests and simulation mode.
class SomeIpConfig {
public:
    static void load(const std::string& configPath);
    static std::string activeConfigPath();
};

} // namespace sdv::someip
