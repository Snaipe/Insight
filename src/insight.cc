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
#include <vector>
#include <unordered_map>
#include <libdwarf++/dwarf.hh>
#include <libdwarf++/die.hh>
#include <libdwarf++/cu.hh>
#include "insight/insight"
#include "internal.hh"
#include "mangle.hh"

namespace Insight {

    template<typename T>
    using OffsetMap = std::unordered_map<size_t, T>;

    using TypeOffsetMap = OffsetMap<std::shared_ptr<TypeInfo>>;
    using MethodOffsetMap = OffsetMap<MethodInfoImpl*>;

    struct BuildContext {
        BuildContext(const Dwarf::Debug& d, TypeOffsetMap& tom, MethodOffsetMap& mom, int& ac)
            : dbg(d)
            , types(tom)
            , methods(mom)
            , anonymous_count(ac)
        {}

        const Dwarf::Debug& dbg;
        TypeOffsetMap& types;
        MethodOffsetMap& methods;
        int& anonymous_count;
        void* target;
    };

    std::unordered_map<std::string, std::shared_ptr<TypeInfo>> type_registry;

    const TypeInfo& type_of(std::string name) {
        return *type_registry.at(name);
    }

    const TypeInfo& type_of(const std::type_info& info) {
        return type_of(demangle(std::string(info.name())));
    }

    static size_t get_offset(Dwarf::Die &die) {
        std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_data_member_location);
        if (attr) {
            return attr->as<Dwarf::Unsigned>();
        }
        return static_cast<size_t>(-1);
    }

    static std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Off offset);
    static std::shared_ptr<TypeInfo> build_struct_type(Dwarf::Die &die, BuildContext& ctx);

    static std::shared_ptr<TypeInfo> build_type(Dwarf::Die& die, BuildContext& ctx) {
        static std::unordered_map<std::string, PrimitiveKind> primitiveKinds {
                {"char",                    PrimitiveKind::CHAR},
                {"signed char",             PrimitiveKind::CHAR},
                {"unsigned char",           PrimitiveKind::UNSIGNED_CHAR},
                {"int",                     PrimitiveKind::INT},
                {"unsigned int",            PrimitiveKind::UNSIGNED_INT},
                {"short int",               PrimitiveKind::SHORT_INT},
                {"short unsigned int",      PrimitiveKind::UNSIGNED_SHORT_INT},
                {"long int",                PrimitiveKind::LONG_INT},
                {"long unsigned int",       PrimitiveKind::UNSIGNED_LONG_INT},
                {"long long int",           PrimitiveKind::LONG_LONG_INT},
                {"long long unsigned int",  PrimitiveKind::UNSIGNED_LONG_LONG_INT},
                {"float",                   PrimitiveKind::FLOAT},
                {"double",                  PrimitiveKind::DOUBLE},
                {"long double",             PrimitiveKind::LONG_DOUBLE},
                {"bool",                    PrimitiveKind::BOOL},
                {"_Bool",                   PrimitiveKind::BOOL},
                {"complex float",           PrimitiveKind::FLOAT_COMPLEX},
                {"complex double",          PrimitiveKind::DOUBLE_COMPLEX},
                {"complex long double",     PrimitiveKind::LONG_DOUBLE_COMPLEX},
        };

        std::shared_ptr<TypeInfo> type;
        auto t = type_registry.find(die.get_name() ?: "");
        if (t != type_registry.end())
            type = t->second;
        else {
            const Dwarf::Tag &tag = die.get_tag();
            switch (tag.get_id()) {
                case DW_TAG_base_type: {
                    PrimitiveKind kind = primitiveKinds[die.get_name()];
                    std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
                    if (!attrsize) // ignore types with no size
                        break;

                    size_t size = attrsize->as<Dwarf::Off>();
                    type = std::make_shared<PrimitiveTypeInfoImpl>(die.get_name(), size, kind);
                }
                    break;
                case DW_TAG_structure_type:
                case DW_TAG_class_type: {
                    type = build_struct_type(die, ctx);
                }
                    break;
                case DW_TAG_pointer_type: {
                    std::unique_ptr<const Dwarf::Attribute> attrtype = die.get_attribute(DW_AT_type);
                    std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
                    if (!attrtype || !attrsize)
                        break;
                    std::shared_ptr<TypeInfo> subtype = get_type(ctx, attrtype->as<Dwarf::Off>());
                    if (!subtype)
                        break;

                    type = std::make_shared<PointerTypeInfoImpl>(subtype, attrsize->as<Dwarf::Unsigned>());
                }
                    break;
                case DW_TAG_const_type: {
                    std::unique_ptr<const Dwarf::Attribute> attrtype = die.get_attribute(DW_AT_type);
                    if (!attrtype)
                        break;
                    std::shared_ptr<TypeInfo> subtype = get_type(ctx, attrtype->as<Dwarf::Off>());
                    if (!subtype)
                        break;

                    type = std::make_shared<ConstTypeInfoImpl>(subtype);
                }
                    break;
                case DW_TAG_typedef: {
                    std::unique_ptr<const Dwarf::Attribute> attrtype = die.get_attribute(DW_AT_type);
                    if (!attrtype)
                        break;
                    std::shared_ptr<TypeInfo> subtype = get_type(ctx, attrtype->as<Dwarf::Off>());
                    if (!subtype)
                        break;

                    type = std::make_shared<TypeDefInfoImpl>(die.get_name(), subtype);
                }
                    break;
                default:
                    break;
            }
            if (type)
                type_registry[type->name()] = type;
        }
        if (type)
            ctx.types[die.get_offset()] = type;
        return type;
    }

    static std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Off offset) {
        auto it = ctx.types.find(offset);
        if (it == ctx.types.end())
            return nullptr;
        return it->second;
    }

    static Dwarf::Die::TraversalResult handle_member(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext *ctx = static_cast<BuildContext*>(data);

        StructInfoImpl *structinfo = static_cast<StructInfoImpl*>(ctx->target);
        switch (tag.get_id()) {
            case DW_TAG_member: {
                size_t offset = get_offset(die);
                if (offset == static_cast<size_t>(-1))
                    break;
                std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_type);
                if (!attr)
                    break;

                std::weak_ptr<TypeInfo> weak(get_type(*ctx, attr->as<Dwarf::Off>()));
                std::unique_ptr<FieldInfo> finfo = std::make_unique<FieldInfoImpl>(die.get_name(), offset, weak);
                structinfo->add_field(finfo);
            } break;
            case DW_TAG_subprogram: {
                if (!die.get_name()) // ignore unnamed methods
                    break;

                std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_type);
                if (!attr)
                    break;

                std::weak_ptr<TypeInfo> return_type(get_type(*ctx, attr->as<Dwarf::Off>()));
                std::unique_ptr<MethodInfo> method = std::make_unique<MethodInfoImpl>(die.get_name(), return_type);
                Dwarf::Off off = die.get_offset();
                ctx->methods[off] = static_cast<MethodInfoImpl*>(&*method);

                structinfo->add_method(method);
            } break;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    static std::shared_ptr<TypeInfo> build_struct_type(Dwarf::Die &die, BuildContext& ctx) {
        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        size_t size = !attrsize ? 0 : attrsize->as<Dwarf::Off>();

        std::string name = die.get_name() ?: ("anonymous#" + std::to_string(ctx.anonymous_count++));
        std::shared_ptr<StructInfo> structinfo = std::make_shared<StructInfoImpl>(name, size);

        BuildContext newctx = ctx;
        newctx.target = &*structinfo;

        die.traverse_headless(handle_member, &newctx);
        return structinfo;
    }

    static Dwarf::Die::TraversalResult build_metadata(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        switch (tag.get_id()) {
            case DW_TAG_const_type:
            case DW_TAG_base_type:
            case DW_TAG_class_type:
            case DW_TAG_structure_type:
            case DW_TAG_pointer_type:
            case DW_TAG_typedef:
                build_type(die, ctx);
                break;
            case DW_TAG_subprogram: {
                std::unique_ptr<const Dwarf::Attribute> attrspec = die.get_attribute(DW_AT_specification);
                if (!attrspec) {
                    // TODO: handle functions
                } else {
                    Dwarf::Off off = attrspec->as<Dwarf::Off>();
                    auto it = ctx.methods.find(off);
                    if (it != ctx.methods.end()) {
                        std::unique_ptr<const Dwarf::Attribute> attraddr = die.get_attribute(DW_AT_low_pc);
                        if (!attraddr)
                            break;
                        Dwarf::Addr addr = attraddr->as<Dwarf::Addr>();

                        MethodInfoImpl* method = it->second;
                        method->address_ = reinterpret_cast<void*>(addr);
                    }
                }
            } break;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    void initialize() {

        std::shared_ptr<const Dwarf::Debug> dbg = Dwarf::Debug::self();

        int anonymous_count = 0;
        TypeOffsetMap types;
        MethodOffsetMap methods;
        BuildContext ctx(*dbg, types, methods, anonymous_count);

        for (const Dwarf::CompilationUnit &cu : *dbg) {
            cu.get_die()->traverse_headless(Insight::build_metadata, &ctx);
        }

    }
}


