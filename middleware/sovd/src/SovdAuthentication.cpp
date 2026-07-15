#include "sovd/SovdAuthentication.hpp"

namespace sdv::sovd {

void SovdAuthentication::registerToken(const std::string& token, SovdRole role)
{
    tokenTable_[token] = role;
}

SovdRole SovdAuthentication::authenticate(const std::string& bearerToken) const
{
    auto it = tokenTable_.find(bearerToken);
    return it != tokenTable_.end() ? it->second : SovdRole::Unauthenticated;
}

SovdRole SovdAuthentication::minimumRoleForPath(const std::string& path)
{
    // Faults and routines require at least Technician access.
    if (path.find("/faults") != std::string::npos ||
        path.find("/routines") != std::string::npos) {
        return SovdRole::Technician;
    }
    // Data reads are available to any authenticated caller.
    return SovdRole::Technician;
}

} // namespace sdv::sovd
