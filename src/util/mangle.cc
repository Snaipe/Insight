/*
 * This file is part of Insight.
 *
 * Copyright © 2015 Franklin "Snaipe" Mathieu <http://snaipe.me>
 *
 * Insight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Insight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Insight.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
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
