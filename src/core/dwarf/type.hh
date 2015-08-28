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
#ifndef INSIGHT_TYPE_HH
# define INSIGHT_TYPE_HH

# include "dwarf.hh"

namespace Insight {

    using Result = Dwarf::Die::TraversalResult;
    using Type = typename std::shared_ptr<TypeInfo>;

    struct TypeBuilder  {

        struct Visitor : public boost::static_visitor<std::shared_ptr<TypeInfo>> {

            Type operator()(Dwarf::TaggedDie<DW_TAG_base_type> &die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_unspecified_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_class_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_structure_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_union_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_enumeration_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_pointer_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_const_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_typedef>& die);

            template <typename T>
            Type operator()([[gnu::unused]] T& die) {
                return nullptr;
            }

            Visitor(TypeBuilder& tb, bool rp, BuildContext& c);

            TypeBuilder& tb;
            bool register_parent;
            BuildContext& ctx;
        };

        std::shared_ptr<TypeInfo> build_type(Dwarf::AnyDie& anydie, bool register_parent);
        std::shared_ptr<TypeInfo> get_type(Dwarf::AnyDie& die);
        std::shared_ptr<TypeInfo> get_type(Dwarf::Off offset);
        std::shared_ptr<TypeInfo> get_type_attr(Dwarf::Die &die);

        TypeBuilder(BuildContext& ctx);

        BuildContext& ctx;
    };

}

# include "struct.hh"
# include "enum.hh"
# include "union.hh"

#endif /* !INSIGHT_TYPE_HH */
