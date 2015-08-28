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
#include "struct.hh"
#include "annotation.hh"

namespace Insight {

    Result StructBuilder::operator()(Dwarf::TaggedDie<DW_TAG_member> &die) {
        std::string name(die.get_name() ?: "");

        auto type = tb.get_type_attr(die);
        if (!type)
            return Result::SKIP;

        std::string prefix("insight_annotation");
        if (name.substr(0, prefix.size()) == prefix) {
            std::unique_ptr<const Dwarf::Attribute> locattr = die.get_attribute(DW_AT_location);
            std::unique_ptr<const Dwarf::Attribute> constattr = die.get_attribute(DW_AT_const_value);

            std::shared_ptr<AnnotationInfoImpl> annotation;

            std::shared_ptr<ConstTypeInfo> constType = std::dynamic_pointer_cast<ConstTypeInfo>(type);
            std::string annotationName = constType ? constType->type().name() : type->name();

            if (locattr) {
                size_t loc = locattr->as<Dwarf::Off>();
                void *addr = reinterpret_cast<void *>(loc);

                annotation = std::make_shared<AnnotationInfoImpl>(annotationName, addr, type);
            } else if (constattr) {
                Dwarf::Block *block = constattr->as<Dwarf::Block *>();

                // we leak the block because we need it alive until the program ends
                void *addr = std::malloc(block->bl_len);
                std::memcpy(addr, block->bl_data, block->bl_len);
                std::shared_ptr<const Dwarf::Debug> dbg = die.get_debug();
                if (dbg)
                    dbg->dealloc(block);

                annotation = std::make_shared<AnnotationInfoImpl>(annotationName, addr, type);
            }

            if (!annotation)
                return Result::SKIP;

            size_t off = get_src_location_offset(die);
            if (off)
                tb.ctx.annotations[off] = annotation;
        } else {

            size_t offset = get_offset(die);
            if (offset == static_cast<size_t>(-1))
                return Result::SKIP;

            std::shared_ptr<FieldInfoImpl> finfo = std::make_shared<FieldInfoImpl>(die.get_name(), offset, type,
                                                                                   info);
            info->add_field(finfo);

            mark_element_line(tb.ctx, die, finfo);
        }

        return Result::SKIP;
    }

    Result StructBuilder::operator()(Dwarf::TaggedDie<DW_TAG_subprogram> &die) {
        if (!die.get_name()) // ignore unnamed methods
            return Result::SKIP;

        auto return_type = tb.get_type_attr(die);
        if (!return_type)
            return_type = VOID_TYPE;

        std::shared_ptr<MethodInfoImpl> method = std::make_shared<MethodInfoImpl>(die.get_name(), return_type, info);

        std::unique_ptr<const Dwarf::Attribute> vattr = die.get_attribute(DW_AT_virtuality);
        std::unique_ptr<const Dwarf::Attribute> vtabattr = die.get_attribute(DW_AT_vtable_elem_location);
        if (vattr && vtabattr) {
            Dwarf::Signed virtuality = vattr->as<Dwarf::Signed>();
            if (virtuality != DW_VIRTUALITY_none)
                method->set_vtable_index(vtabattr->as<uint64_t>());
        }

        Dwarf::Off off = die.get_offset();
        tb.ctx.methods.insert(std::make_pair(off, AnyMethod(method)));

        info->add_method(method);

        auto it = tb.ctx.method_addresses.find(die.get_offset());
        if (it != tb.ctx.method_addresses.end()) {
            method->address_ = it->second;
        }

        mark_element_line(tb.ctx, die, method);

        return Result::SKIP;
    }

    Result StructBuilder::operator()(Dwarf::TaggedDie<DW_TAG_inheritance> &die) {
        auto super_type = tb.get_type_attr(die);
        if (!super_type)
            return Result::SKIP;

        info->add_supertype(std::dynamic_pointer_cast<StructInfo>(super_type));

        return Result::SKIP;
    }

    StructBuilder::StructBuilder(std::shared_ptr<StructInfoImpl> info, TypeBuilder &tb)
        : info(info)
        , tb(tb)
    {}

    std::shared_ptr<TypeInfo> build_struct_type(Dwarf::Die &die, TypeBuilder& tb, bool register_parent) {
        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        size_t size = !attrsize ? 0 : attrsize->as<Dwarf::Off>();

        std::shared_ptr<Container> parent = get_parent(tb.ctx);

        std::string name = die.get_name() ?: ("anonymous#" + std::to_string(tb.ctx.anonymous_count++));
        std::shared_ptr<StructInfoImpl> info = std::make_shared<StructInfoImpl>(name, size);
        tb.ctx.types[die.get_offset()] = info;

        if (register_parent)
            info->set_parent(parent);

        StructBuilder builder(info, tb);

        tb.ctx.container_stack.push(AnyContainer(info));
        die.visit_headless(builder);
        tb.ctx.container_stack.pop();

        mark_element_line(tb.ctx, die, info);

        return info;
    }

}
