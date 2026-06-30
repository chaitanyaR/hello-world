#include "someip/SomeIpConfig.hpp"
#include <cstdlib>

namespace sdv::someip {

static std::string s_activePath;

void SomeIpConfig::load(const std::string& configPath)
{
    // VSOMEIP_CONFIGURATION env var overrides the provided path.
    const char* envPath = std::getenv("VSOMEIP_CONFIGURATION");
    s_activePath = envPath ? envPath : configPath;

#ifdef VSOMEIP_AVAILABLE
    // vsomeip reads VSOMEIP_CONFIGURATION automatically at application init.
    // Ensure the env var is set so it picks up the correct file.
    setenv("VSOMEIP_CONFIGURATION", s_activePath.c_str(), /*overwrite=*/0);
#endif
}

std::string SomeIpConfig::activeConfigPath()
{
    return s_activePath;
}

} // namespace sdv::someip
