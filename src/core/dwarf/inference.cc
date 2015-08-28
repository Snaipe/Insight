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
#include "inference.hh"

namespace Insight {

    Result TypeInferer::operator()(Dwarf::TaggedDie<DW_TAG_variable>& die) {
        const char *name = die.get_name();
        if (!name)
            return Result::SKIP;

        if (std::string(name) != "insight_typeof_dummy")
            return Result::SKIP;

        auto type = tb.get_type_attr(die);
        if (!type)
            return Result::SKIP;

        std::unique_ptr<const Dwarf::Attribute> locattr = die.get_attribute(DW_AT_location);
        if (!locattr)
            return Result::SKIP;

        size_t loc = locattr->as<Dwarf::Off>();

        auto inferred_type = std::dynamic_pointer_cast<PointerTypeInfoImpl>(type);
        inferred_type_registry[loc] = inferred_type->type_.lock();

        return Result::SKIP;
    }

    Result TypeInferer::operator()([[gnu::unused]] Dwarf::TaggedDie<DW_TAG_lexical_block>& die) {
        return Result::TRAVERSE;
    }

    TypeInferer::TypeInferer(TypeBuilder& tb) : tb(tb) {}

}
