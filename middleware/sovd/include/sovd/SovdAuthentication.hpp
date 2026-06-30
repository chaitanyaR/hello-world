#pragma once

#include "SovdTypes.hpp"
#include <string>
#include <unordered_map>

namespace sdv::sovd {

// Bearer-token validation per ISO 17879 §8.
// In development, tokens are loaded from a static table.
// In production, replace with HSM-backed PKI certificate verification.
class SovdAuthentication {
public:
    void registerToken(const std::string& token, SovdRole role);

    // Returns the role for a valid token, or Unauthenticated.
    SovdRole authenticate(const std::string& bearerToken) const;

    static SovdRole minimumRoleForPath(const std::string& path);

private:
    std::unordered_map<std::string, SovdRole> tokenTable_;
};

} // namespace sdv::sovd
