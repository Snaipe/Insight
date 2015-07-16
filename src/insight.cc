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
#include <memory>
#include <stack>
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
    using MethodOffsetMap = OffsetMap<std::shared_ptr<MethodInfoImpl>>;

    NamespaceInfoImpl ROOT_NAMESPACE("");

    std::shared_ptr<TypeInfo> VOID_TYPE = std::make_shared<PrimitiveTypeInfoImpl>("void", 0, PrimitiveKind::VOID, ROOT_NAMESPACE);

    struct BuildContext {
        BuildContext(const Dwarf::Debug& d)
            : dbg(d)
            , types()
            , methods()
            , anonymous_count(0)
            , namespace_stack()
            , struct_stack()
        {}

        const Dwarf::Debug& dbg;
        TypeOffsetMap types;
        MethodOffsetMap methods;
        int anonymous_count;
        std::stack<NamespaceInfoImpl*> namespace_stack;
        std::stack<StructInfoImpl*> struct_stack;
    };

    static void add_type_to_parent(BuildContext& ctx, std::shared_ptr<TypeInfo> ptr) {
        if (ctx.struct_stack.empty())
            ctx.namespace_stack.top()->add_type(ptr);
        else
            ctx.struct_stack.top()->add_type(ptr);
    }

    static void add_func_to_parent(BuildContext& ctx, std::shared_ptr<FunctionInfo> ptr) {
        if (ctx.struct_stack.empty())
            ctx.namespace_stack.top()->add_function(ptr);
        else
            ctx.struct_stack.top()->add_function(ptr);
    }

    static void add_var_to_parent(BuildContext& ctx, std::shared_ptr<VariableInfo> ptr) {
        if (ctx.struct_stack.empty())
            ctx.namespace_stack.top()->add_variable(ptr);
        else
            ctx.struct_stack.top()->add_variable(ptr);
    }

    static Container *get_parent(BuildContext& ctx) {
        if (ctx.struct_stack.empty())
            return ctx.namespace_stack.top();
        return ctx.struct_stack.top();
    }

    std::unordered_map<std::string, std::shared_ptr<TypeInfo>> type_registry;
    std::unordered_map<size_t, std::shared_ptr<TypeInfo>> inferred_type_registry;

    const TypeInfo& type_of_(void *dummy_addr) {
        return *inferred_type_registry.at(reinterpret_cast<size_t>(dummy_addr));
    }

    const TypeInfo& type_of_(std::string name) {
        return *type_registry.at(name);
    }

    const TypeInfo& type_of_(const std::type_info& info) {
        return type_of_(demangle(std::string(info.name())));
    }

    NamespaceInfo& root_namespace() {
        return ROOT_NAMESPACE;
    }

    static size_t get_offset(Dwarf::Die &die) {
        std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_data_member_location);
        if (attr) {
            return attr->as<Dwarf::Unsigned>();
        }
        return static_cast<size_t>(-1);
    }

    static std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Off offset);
    static std::shared_ptr<TypeInfo> build_struct_type(Dwarf::Die &die, BuildContext& ctx, bool register_parent);

    static std::shared_ptr<TypeInfo> build_type(Dwarf::Die& die, BuildContext& ctx, bool register_parent) {
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

        Container* parent = get_parent(ctx);

        std::shared_ptr<TypeInfo> type;
        const auto t = type_registry.find(die.get_name() ?: "");
        if (t != type_registry.end()) {
            type = t->second;
            if (register_parent) {
                Child *child = dynamic_cast<Child *>(&*type);
                child->set_parent(parent);
            }
        } else {
            const Dwarf::Tag &tag = die.get_tag();
            switch (tag.get_id()) {
                case DW_TAG_array_type: {

                } break;
                case DW_TAG_base_type: {
                    PrimitiveKind kind = primitiveKinds[die.get_name()];
                    std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
                    if (!attrsize) // ignore types with no size
                        break;

                    size_t size = attrsize->as<Dwarf::Off>();
                    auto t = std::make_shared<PrimitiveTypeInfoImpl>(die.get_name(), size, kind);

                    if (register_parent)
                        t->set_parent(parent);

                    ctx.types[die.get_offset()] = t;
                    type = t;
                } break;
                case DW_TAG_class_type: {
                    type = build_struct_type(die, ctx, register_parent);
                } break;
                case DW_TAG_structure_type: {
                    type = build_struct_type(die, ctx, register_parent);
                    type_registry["struct " + type->name()] = type;
                } break;
                case DW_TAG_pointer_type: {
                    std::shared_ptr<PointerTypeInfoImpl> t = std::make_shared<PointerTypeInfoImpl>();
                    ctx.types[die.get_offset()] = t;

                    std::unique_ptr<const Dwarf::Attribute> attrtype = die.get_attribute(DW_AT_type);
                    std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
                    if (!attrsize)
                        break;
                    if (!attrtype) {
                        t->set_type(VOID_TYPE);
                    } else {
                        std::shared_ptr<TypeInfo> subtype = get_type(ctx, attrtype->as<Dwarf::Off>());
                        if (!subtype)
                            break;

                        t->set_type(subtype);
                    }

                    if (register_parent)
                        t->set_parent(parent);

                    t->size_ = attrsize->as<Dwarf::Unsigned>();
                    type = t;
                } break;
                case DW_TAG_const_type: {
                    std::shared_ptr<ConstTypeInfoImpl> t = std::make_shared<ConstTypeInfoImpl>();
                    ctx.types[die.get_offset()] = t;

                    std::unique_ptr<const Dwarf::Attribute> attrtype = die.get_attribute(DW_AT_type);
                    if (!attrtype)
                        break;
                    std::shared_ptr<TypeInfo> subtype = get_type(ctx, attrtype->as<Dwarf::Off>());
                    if (!subtype)
                        break;

                    if (register_parent)
                        t->set_parent(parent);

                    t->set_type(subtype);
                    type = t;
                } break;
                case DW_TAG_typedef: {
                    std::shared_ptr<TypeDefInfoImpl> t = std::make_shared<TypeDefInfoImpl>(die.get_name());
                    ctx.types[die.get_offset()] = t;

                    std::unique_ptr<const Dwarf::Attribute> attrtype = die.get_attribute(DW_AT_type);
                    if (!attrtype)
                        break;
                    std::shared_ptr<TypeInfo> subtype = get_type(ctx, attrtype->as<Dwarf::Off>());
                    if (!subtype)
                        break;

                    if (register_parent)
                        t->set_parent(parent);

                    t->set_type(subtype);
                    type = t;
                } break;
                default: break;
            }
            if (type) {
                if (register_parent)
                    add_type_to_parent(ctx, type);
                type_registry[type->name()] = type;
            }
        }
        return type;
    }

    static std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Off offset) {
        auto it = ctx.types.find(offset);
        if (it == ctx.types.end()) {
            std::shared_ptr<Dwarf::Die> die = ctx.dbg.offdie(offset);
            return die ? build_type(*die, ctx, false) : nullptr;
        }
        return it->second;
    }

    static Dwarf::Die::TraversalResult handle_member(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        StructInfoImpl *structinfo = ctx.struct_stack.top();
        switch (tag.get_id()) {
            case DW_TAG_member: {
                size_t offset = get_offset(die);
                if (offset == static_cast<size_t>(-1))
                    break;
                std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_type);
                if (!attr)
                    break;

                std::weak_ptr<TypeInfo> weak(get_type(ctx, attr->as<Dwarf::Off>()));
                std::shared_ptr<FieldInfo> finfo = std::make_shared<FieldInfoImpl>(die.get_name(), offset, weak, *structinfo);
                structinfo->add_field(finfo);
            } break;
            case DW_TAG_subprogram: {
                if (!die.get_name()) // ignore unnamed methods
                    break;

                std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_type);
                if (!attr)
                    break;

                std::weak_ptr<TypeInfo> return_type(get_type(ctx, attr->as<Dwarf::Off>()));
                std::shared_ptr<MethodInfoImpl> method = std::make_shared<MethodInfoImpl>(die.get_name(), return_type, *structinfo);

                std::unique_ptr<const Dwarf::Attribute> vattr = die.get_attribute(DW_AT_virtuality);
                std::unique_ptr<const Dwarf::Attribute> vtabattr = die.get_attribute(DW_AT_vtable_elem_location);
                if (vattr && vtabattr) {
                    Dwarf::Signed virtuality = vattr->as<Dwarf::Signed>();
                    if (virtuality != DW_VIRTUALITY_none)
                        method->set_vtable_index(vtabattr->as<uint64_t>());
                }

                Dwarf::Off off = die.get_offset();
                ctx.methods[off] = method;

                structinfo->add_method(method);
            } break;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    static std::shared_ptr<TypeInfo> build_struct_type(Dwarf::Die &die, BuildContext& ctx, bool register_parent) {
        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        size_t size = !attrsize ? 0 : attrsize->as<Dwarf::Off>();

        Container* parent = get_parent(ctx);

        std::string name = die.get_name() ?: ("anonymous#" + std::to_string(ctx.anonymous_count++));
        std::shared_ptr<StructInfoImpl> structinfo = std::make_shared<StructInfoImpl>(name, size);
        ctx.types[die.get_offset()] = structinfo;

        if (register_parent)
            structinfo->set_parent(parent);

        ctx.struct_stack.push(&*structinfo);
        die.traverse_headless(handle_member, &ctx);
        ctx.struct_stack.pop();

        return structinfo;
    }

    static Dwarf::Die::TraversalResult infer_types(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        switch (tag.get_id()) {
            case DW_TAG_variable: {
                const char *name = die.get_name();
                if (!name)
                    break;

                if (std::string(name) != "insight_typeof_dummy")
                    break;

                std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_type);
                if (!attr)
                    break;

                std::unique_ptr<const Dwarf::Attribute> locattr = die.get_attribute(DW_AT_location);
                if (!locattr)
                    break;

                size_t loc = locattr->as<Dwarf::Off>();

                auto type = get_type(ctx, attr->as<Dwarf::Off>());
                auto inferred_type = std::dynamic_pointer_cast<PointerTypeInfoImpl>(type);
                inferred_type_registry[loc] = inferred_type->type_.lock();
            }
            case DW_TAG_lexical_block: return Dwarf::Die::TraversalResult::TRAVERSE;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    static Dwarf::Die::TraversalResult build_metadata(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        Container* parent = get_parent(ctx);

        switch (tag.get_id()) {
            case DW_TAG_namespace: {
                NamespaceInfoImpl* parentns = ctx.namespace_stack.top();
                std::shared_ptr<NamespaceInfoImpl> ns;
                auto it = parentns->nested_namespaces_.find(die.get_name());
                if (it != parentns->nested_namespaces_.end()) {
                    ns = std::dynamic_pointer_cast<NamespaceInfoImpl>(it->second);
                } else {
                    ns = std::make_shared<NamespaceInfoImpl>(die.get_name(), *parent);
                    parentns->add_nested_namespace(ns);
                }
                ctx.namespace_stack.push(&*ns);
                die.traverse_headless(build_metadata, data);
                ctx.namespace_stack.pop();
            } return Dwarf::Die::TraversalResult::SKIP;
            case DW_TAG_const_type:
            case DW_TAG_base_type:
            case DW_TAG_class_type:
            case DW_TAG_structure_type:
            case DW_TAG_pointer_type:
            case DW_TAG_typedef:
                build_type(die, ctx, true);
                break;
            case DW_TAG_subprogram: {
                std::unique_ptr<const Dwarf::Attribute> attrspec = die.get_attribute(DW_AT_specification);
                if (!attrspec) {
                    die.traverse_headless(infer_types, &ctx);

                    if (!die.get_name()) // ignore unnamed functions
                        break;

                    std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_type);
                    if (!attr)
                        break;

                    std::weak_ptr<TypeInfo> return_type(get_type(ctx, attr->as<Dwarf::Off>()));
                    std::shared_ptr<FunctionInfoImpl> func = std::make_shared<FunctionInfoImpl>(die.get_name(), return_type, *parent);

                    add_func_to_parent(ctx, func);

                    std::unique_ptr<const Dwarf::Attribute> attraddr = die.get_attribute(DW_AT_low_pc);
                    if (!attraddr)
                        break;
                    Dwarf::Addr addr = attraddr->as<Dwarf::Addr>();

                    func->address_ = reinterpret_cast<void*>(addr);
                } else {
                    Dwarf::Off off = attrspec->as<Dwarf::Off>();
                    auto it = ctx.methods.find(off);
                    if (it != ctx.methods.end()) {
                        std::shared_ptr<MethodInfoImpl>& method = it->second;
                        if (method->is_virtual())
                            break;

                        std::unique_ptr<const Dwarf::Attribute> attraddr = die.get_attribute(DW_AT_low_pc);
                        if (!attraddr)
                            break;
                        Dwarf::Addr addr = attraddr->as<Dwarf::Addr>();

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
        type_registry["void"] = VOID_TYPE;

        for (const Dwarf::CompilationUnit &cu : *dbg) {
            BuildContext ctx(*dbg);
            ctx.namespace_stack.push(&ROOT_NAMESPACE);

            cu.get_die()->traverse_headless(Insight::build_metadata, &ctx);
        }

    }
}

static class Init {
public:
    Init() { Insight::initialize(); }
} init;
