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
#include "type.hh"

namespace Insight {

    Type TypeBuilder::Visitor::operator()(Dwarf::TaggedDie<DW_TAG_base_type> &die) {
        static std::unordered_map<std::string, PrimitiveKind> primitiveKinds{
                {"char",                   PrimitiveKind::CHAR},
                {"signed char",            PrimitiveKind::CHAR},
                {"unsigned char",          PrimitiveKind::UNSIGNED_CHAR},
                {"int",                    PrimitiveKind::INT},
                {"unsigned int",           PrimitiveKind::UNSIGNED_INT},
                {"short int",              PrimitiveKind::SHORT_INT},
                {"short unsigned int",     PrimitiveKind::UNSIGNED_SHORT_INT},
                {"long int",               PrimitiveKind::LONG_INT},
                {"long unsigned int",      PrimitiveKind::UNSIGNED_LONG_INT},
                {"long long int",          PrimitiveKind::LONG_LONG_INT},
                {"long long unsigned int", PrimitiveKind::UNSIGNED_LONG_LONG_INT},
                {"float",                  PrimitiveKind::FLOAT},
                {"double",                 PrimitiveKind::DOUBLE},
                {"long double",            PrimitiveKind::LONG_DOUBLE},
                {"bool",                   PrimitiveKind::BOOL},
                {"_Bool",                  PrimitiveKind::BOOL},
                {"complex float",          PrimitiveKind::FLOAT_COMPLEX},
                {"complex double",         PrimitiveKind::DOUBLE_COMPLEX},
                {"complex long double",    PrimitiveKind::LONG_DOUBLE_COMPLEX},
        };

        std::shared_ptr<Container> parent = get_parent(ctx);

        PrimitiveKind kind = primitiveKinds[die.get_name()];
        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        if (!attrsize) // ignore types with no size
            return nullptr;

        size_t size = attrsize->as<Dwarf::Off>();
        auto t = std::make_shared<PrimitiveTypeInfoImpl>(die.get_name(), size, kind);

        if (register_parent)
            t->set_parent(parent);

        ctx.types[die.get_offset()] = t;
        return t;
    }

    Type TypeBuilder::Visitor::operator()(Dwarf::TaggedDie<DW_TAG_unspecified_type>& die) {
        std::shared_ptr<Container> parent = get_parent(ctx);

        auto t = std::make_shared<UnspecifiedTypeInfoImpl>(die.get_name());
        ctx.types[die.get_offset()] = t;

        // C++11 requires sizeof(nullptr_t) == sizeof(void*)
        if (std::string(die.get_name()) == "decltype(nullptr)")
            t->size_ = sizeof(void*);

        if (register_parent)
            t->set_parent(parent);

        return t;
    }

    Type TypeBuilder::Visitor::operator()(Dwarf::TaggedDie<DW_TAG_class_type>& die) {
        return build_struct_type(die, tb, register_parent);
    }

    Type TypeBuilder::Visitor::operator()(Dwarf::TaggedDie<DW_TAG_structure_type>& die) {
        return build_struct_type(die, tb, register_parent);
    }

    Type TypeBuilder::Visitor::operator()(Dwarf::TaggedDie<DW_TAG_union_type>& die) {
        return build_union_type(die, tb, register_parent);
    }

    Type TypeBuilder::Visitor::operator()(Dwarf::TaggedDie<DW_TAG_enumeration_type>& die) {
        return build_enum_type(die, tb, register_parent);
    }

    Type TypeBuilder::Visitor::operator()(Dwarf::TaggedDie<DW_TAG_pointer_type>& die) {
        std::shared_ptr<Container> parent = get_parent(ctx);

        std::shared_ptr<PointerTypeInfoImpl> t = std::make_shared<PointerTypeInfoImpl>();
        ctx.types[die.get_offset()] = t;

        std::unique_ptr<const Dwarf::Attribute> attrtype = die.get_attribute(DW_AT_type);
        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        if (!attrsize)
            return nullptr;

        if (!attrtype) {
            t->set_type(VOID_TYPE);
        } else {
            std::shared_ptr<TypeInfo> subtype = tb.get_type(attrtype->as<Dwarf::Off>());
            if (!subtype)
                return nullptr;

            t->set_type(subtype);
        }

        if (register_parent)
            t->set_parent(parent);

        t->size_ = attrsize->as<Dwarf::Unsigned>();
        return t;
    }

    Type TypeBuilder::Visitor::operator()(Dwarf::TaggedDie<DW_TAG_const_type>& die) {
        std::shared_ptr<Container> parent = get_parent(ctx);

        std::shared_ptr<ConstTypeInfoImpl> t = std::make_shared<ConstTypeInfoImpl>();
        ctx.types[die.get_offset()] = t;

        auto subtype = tb.get_type_attr(die);
        if (!subtype)
            return nullptr;

        if (register_parent)
            t->set_parent(parent);

        t->set_type(subtype);
        return t;
    }

    Type TypeBuilder::Visitor::operator()(Dwarf::TaggedDie<DW_TAG_typedef>& die) {
        std::shared_ptr<Container> parent = get_parent(ctx);

        const char *name = die.get_name();
        if (!name)
            return nullptr;

        std::shared_ptr<TypeDefInfoImpl> t = std::make_shared<TypeDefInfoImpl>(name);
        ctx.types[die.get_offset()] = t;

        auto subtype = tb.get_type_attr(die);
        if (!subtype)
            return nullptr;

        if (register_parent)
            t->set_parent(parent);

        t->set_type(subtype);
        return t;
    }

    TypeBuilder::Visitor::Visitor(TypeBuilder& tb, bool rp, BuildContext& c)
            : tb(tb), register_parent(rp), ctx(c)
    {}

    std::shared_ptr<TypeInfo> TypeBuilder::build_type(Dwarf::AnyDie& anydie, bool register_parent) {
        Dwarf::Die::visitor_to_die to_die;
        Dwarf::Die& die = anydie.apply_visitor(to_die);

        std::shared_ptr<Container> parent = get_parent(ctx);

        std::shared_ptr<TypeInfo> type;
        std::string name = die.get_name() ?: "";

        const auto it = ctx.types.find(die.get_offset());
        if (it != ctx.types.end()) {
            type = it->second;
            if (register_parent) {
                std::shared_ptr<MutableChild> child = std::dynamic_pointer_cast<MutableChild>(type);
                child->set_parent(parent);
            }
        } else {
            Visitor visitor(*this, register_parent, ctx);
            type = anydie.apply_visitor(visitor);
            if (type && register_parent)
                add_type_to_parent(ctx, type);
        }
        return type;
    }

    std::shared_ptr<TypeInfo> TypeBuilder::get_type(Dwarf::AnyDie& die) {
        Dwarf::Die::visitor_to_die to_die;
        auto it = ctx.types.find(die.apply_visitor(to_die).get_offset());
        if (it == ctx.types.end()) {
            return build_type(die, false);
        }
        return it->second;
    }

    std::shared_ptr<TypeInfo> TypeBuilder::get_type(Dwarf::Off offset) {
        auto it = ctx.types.find(offset);
        if (it == ctx.types.end()) {
            std::shared_ptr<Dwarf::AnyDie> die = ctx.dbg.offdie(offset);
            return die ? build_type(*die, false) : nullptr;
        }
        return it->second;
    }

    std::shared_ptr<TypeInfo> TypeBuilder::get_type_attr(Dwarf::Die &die) {
        std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_type);
        if (!attr)
            return nullptr;

        auto target = attr->as_die();
        return target ? get_type(*target) : nullptr;
    }

    TypeBuilder::TypeBuilder(BuildContext& ctx)
        : ctx(ctx)
    {}

}
