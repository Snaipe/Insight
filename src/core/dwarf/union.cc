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
#include "union.hh"
#include "annotation.hh"

namespace Insight {

    Result UnionBuilder::operator()(Dwarf::TaggedDie<DW_TAG_member> &die) {
        auto type = tb.get_type_attr(die);
        if (!type)
            return Result::SKIP;

        std::shared_ptr<UnionFieldInfoImpl> finfo = std::make_shared<UnionFieldInfoImpl>(die.get_name(), type, info);
        info->add_field(finfo);

        mark_element_line(tb.ctx, die, finfo);

        return Result::SKIP;
    }

    Result UnionBuilder::operator()(Dwarf::TaggedDie<DW_TAG_subprogram> &die) {
        if (!die.get_name()) // ignore unnamed methods
            return Result::SKIP;

        auto return_type = tb.get_type_attr(die);
        if (!return_type)
            return_type = VOID_TYPE;

        std::shared_ptr<UnionMethodInfoImpl> method = std::make_shared<UnionMethodInfoImpl>(die.get_name(), return_type, info);

        Dwarf::Off off = die.get_offset();
        tb.ctx.methods.insert(std::make_pair(off, AnyMethod(method)));

        info->add_method(method);

        mark_element_line(tb.ctx, die, method);

        auto it = tb.ctx.method_addresses.find(die.get_offset());
        if (it != tb.ctx.method_addresses.end()) {
            method->address_ = it->second;
        }

        return Result::SKIP;
    }

    UnionBuilder::UnionBuilder(std::shared_ptr<UnionInfoImpl> info, TypeBuilder &tb)
        : info(info)
        , tb(tb)
    {}

    std::shared_ptr<TypeInfo> build_union_type(Dwarf::Die &die, TypeBuilder& tb, bool register_parent) {
        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        size_t size = !attrsize ? 0 : attrsize->as<Dwarf::Off>();

        std::shared_ptr<Container> parent = get_parent(tb.ctx);

        std::string name = die.get_name() ?: ("anonymous#" + std::to_string(tb.ctx.anonymous_count++));
        std::shared_ptr<UnionInfoImpl> info = std::make_shared<UnionInfoImpl>(name, size);
        tb.ctx.types[die.get_offset()] = info;

        if (register_parent)
            info->set_parent(parent);

        UnionBuilder builder(info, tb);

        tb.ctx.container_stack.push(AnyContainer(info));
        die.visit_headless(builder);
        tb.ctx.container_stack.pop();

        mark_element_line(tb.ctx, die, info);

        return info;
    }

}
