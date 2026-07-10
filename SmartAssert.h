#pragma once

#include <string>
#include <sstream>
#include <iostream>
#include <cstdlib>

namespace smart_assert {

inline std::string format(const char* file, int line, const char* cond, const std::string& msg)
{
    std::ostringstream oss;
    oss << "SMART_ASSERT echoue: (" << cond << ") a " << file << ":" << line;
    if (!msg.empty()) oss << " -- " << msg;
    return oss.str();
}

[[noreturn]] inline void fail(const char* file, int line, const char* cond, const std::string& msg)
{
    std::cerr << format(file, line, cond, msg) << std::endl;
    std::abort();
}

} // namespace smart_assert

// Toujours actif : NON supprime par NDEBUG (contrairement a assert). Pour un
// vehicule, les invariants critiques doivent rester actifs meme en release.
#define SMART_ASSERT(cond, msg) \
    do { if (!(cond)) ::smart_assert::fail(__FILE__, __LINE__, #cond, (msg)); } while (0)
