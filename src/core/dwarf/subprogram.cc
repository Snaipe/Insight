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
#include "subprogram.hh"

using namespace Insight;

Result ParameterListBuilder::operator()(Dwarf::TaggedDie<DW_TAG_formal_parameter>& die) {
    // don't build info for artificial parameters such as "this"
    std::unique_ptr<const Dwarf::Attribute> artattr = die.get_attribute(DW_AT_artificial);
    if (artattr)
        return Result::SKIP;

    std::shared_ptr<TypeInfo> type = tb.get_type_attr(die);
    if (!type)
        return Result::SKIP;

    std::string name = die.get_name() ?: ("param" + std::to_string(paramIndex));

    std::shared_ptr<ParameterInfoImpl> param = std::make_shared<ParameterInfoImpl>(name, type);
    param->index_ = paramIndex;

    paramIndex++;

    map[name] = param;

    return Result::SKIP;
}

ParameterListBuilder::ParameterListBuilder(TypeBuilder& tb)
        : tb(tb)
        , paramIndex(0)
        , map()
{}
