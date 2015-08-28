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
#ifndef INSIGHT_UNION_HH
# define INSIGHT_UNION_HH

# include "type.hh"

namespace Insight {

    struct UnionBuilder : public Dwarf::DefaultDieVisitor {

        Result operator()(Dwarf::TaggedDie<DW_TAG_member> &die);
        Result operator()(Dwarf::TaggedDie<DW_TAG_subprogram> &die);

        template <typename T>
        Result operator()([[gnu::unused]] T& t) {
            return Result::SKIP;
        }

        UnionBuilder(std::shared_ptr<UnionInfoImpl> info, TypeBuilder &tb);

    private:
        std::shared_ptr<UnionInfoImpl> info;
        TypeBuilder& tb;

    };

    std::shared_ptr<TypeInfo> build_union_type(Dwarf::Die &die, TypeBuilder& tb, bool register_parent);

}

#endif /* !INSIGHT_UNION_HH */
