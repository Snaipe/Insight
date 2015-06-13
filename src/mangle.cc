#include "mangle.hh"

#ifdef __GNUG__
#include <cxxabi.h>
#include <memory>

std::string Insight::demangle(std::string&& name) {
    int status;
    std::unique_ptr<char, void(*)(void*)> res(abi::__cxa_demangle(name.c_str(), 0, 0, &status), std::free);
    return status ? name : res.get();
};

#else
std::string Insight::demangle(std::string&& name) {
    return name;
}
#endif
