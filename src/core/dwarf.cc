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
#include <map>
#include <memory>
#include <stack>
#include <boost/variant.hpp>
#include <libdwarf++/dwarf.hh>
#include <libdwarf++/die.hh>
#include <libdwarf++/cu.hh>
#include "insight/insight"
#include "data/internal.hh"
#include "util/mangle.hh"

#define STRUCT_OR_UNION_VISITOR(type, kind, operation) \
    struct add_ ## kind : public boost::static_visitor<void> { \
        void operator()(std::shared_ptr<StructInfoImpl>& info) const { operation; } \
        void operator()(std::shared_ptr<UnionInfoImpl>& info) const { operation; } \
        add_ ## kind (std::shared_ptr<type>& t) : ptr(t) {} \
        std::shared_ptr<type>& ptr; \
    }

namespace Insight {

    using StructOrUnion = boost::variant<std::shared_ptr<StructInfoImpl>, std::shared_ptr<UnionInfoImpl>>;
    using StructOrUnionMethod = boost::variant<std::shared_ptr<MethodInfoImpl>, std::shared_ptr<UnionMethodInfoImpl>>;

    using AnyAnnotated = boost::variant<
                std::shared_ptr<StructInfoImpl>,
                std::shared_ptr<UnionInfoImpl>,
                std::shared_ptr<VariableInfoImpl>,
                std::shared_ptr<FunctionInfoImpl>,
                std::shared_ptr<MethodInfoImpl>,
                std::shared_ptr<FieldInfoImpl>,
                std::shared_ptr<UnionMethodInfoImpl>,
                std::shared_ptr<UnionFieldInfoImpl>,
                std::shared_ptr<NamespaceInfoImpl>
            >;

    struct AddAnnotation : boost::static_visitor<void> {
        AddAnnotation(std::shared_ptr<AnnotationInfoImpl> annotation) : annotation_(annotation) {}

        void operator()(std::shared_ptr<StructInfoImpl>& type) {
            type->add_annotation(annotation_);
        }

        void operator()(std::shared_ptr<UnionInfoImpl>& type) {
            type->add_annotation(annotation_);
        }

        void operator()(std::shared_ptr<FieldInfoImpl>& field) {
            field->add_annotation(annotation_);
        }

        void operator()(std::shared_ptr<MethodInfoImpl>& method) {
            method->add_annotation(annotation_);
        }

        void operator()(std::shared_ptr<UnionFieldInfoImpl>& field) {
            field->add_annotation(annotation_);
        }

        void operator()(std::shared_ptr<UnionMethodInfoImpl>& method) {
            method->add_annotation(annotation_);
        }

        void operator()(std::shared_ptr<VariableInfoImpl>& var) {
            var->add_annotation(annotation_);
        }

        void operator()(std::shared_ptr<FunctionInfoImpl>& fun) {
            fun->add_annotation(annotation_);
        }

        void operator()(std::shared_ptr<NamespaceInfoImpl>& ns) {
            ns->add_annotation(annotation_);
        }

        std::shared_ptr<AnnotationInfoImpl> annotation_;
    };

    template<typename T>
    using OffsetMap = std::unordered_map<size_t, T>;

    using TypeOffsetMap = OffsetMap<std::shared_ptr<TypeInfo>>;
    using MethodOffsetMap = OffsetMap<StructOrUnionMethod>;

    NamespaceInfoImpl ROOT_NAMESPACE("");
    std::unordered_map<std::string, std::shared_ptr<NamespaceInfo>> namespaces;

    std::shared_ptr<TypeInfo> VOID_TYPE = std::make_shared<PrimitiveTypeInfoImpl>("void", 0, PrimitiveKind::VOID, ROOT_NAMESPACE);

    struct BuildContext {
        BuildContext(const Dwarf::Debug& d)
            : dbg(d)
            , types()
            , methods()
            , anonymous_count(0)
            , namespace_stack()
            , type_stack()
            , annotations()
        {}

        const Dwarf::Debug& dbg;
        TypeOffsetMap types;
        MethodOffsetMap methods;
        int anonymous_count;
        std::stack<NamespaceInfoImpl*> namespace_stack;
        std::stack<StructOrUnion> type_stack;
        std::map<size_t, std::shared_ptr<AnnotationInfoImpl>> annotations;
        std::map<size_t, AnyAnnotated> annotated;
    };

    static size_t get_src_location_offset(Dwarf::Die& die) {
        std::unique_ptr<const Dwarf::Attribute> lineattr = die.get_attribute(DW_AT_decl_line);
        std::unique_ptr<const Dwarf::Attribute> fileattr = die.get_attribute(DW_AT_decl_file);

        if (!lineattr || !fileattr)
            return 0;

        return (fileattr->as<Dwarf::Unsigned>() << ((sizeof (size_t) >> 1) * 8)) | lineattr->as<Dwarf::Unsigned>();
    }

    static void add_type_to_parent(BuildContext& ctx, std::shared_ptr<TypeInfo> ptr) {
        STRUCT_OR_UNION_VISITOR(TypeInfo, type, info->add_type(ptr));

        if (ctx.type_stack.empty())
            ctx.namespace_stack.top()->add_type(ptr);
        else {
            auto visitor = add_type(ptr);
            ctx.type_stack.top().apply_visitor(visitor);
        }
    }

    static void add_func_to_parent(BuildContext& ctx, std::shared_ptr<FunctionInfo> ptr) {
        STRUCT_OR_UNION_VISITOR(FunctionInfo, function, info->add_function(ptr));

        if (ctx.type_stack.empty())
            ctx.namespace_stack.top()->add_function(ptr);
        else {
            auto visitor = add_function(ptr);
            ctx.type_stack.top().apply_visitor(visitor);
        }
    }

    static void add_var_to_parent(BuildContext& ctx, std::shared_ptr<VariableInfo> ptr) {
        STRUCT_OR_UNION_VISITOR(VariableInfo, variable, info->add_variable(ptr));

        if (ctx.type_stack.empty())
            ctx.namespace_stack.top()->add_variable(ptr);
        else {
            auto visitor = add_variable(ptr);
            ctx.type_stack.top().apply_visitor(visitor);
        }
    }

    static Container *get_parent(BuildContext& ctx) {
        if (ctx.type_stack.empty())
            return ctx.namespace_stack.top();
        else {
            auto& t = ctx.type_stack.top();
            if (t.type() == typeid(std::shared_ptr<StructInfoImpl>)) {
                return boost::get<std::shared_ptr<StructInfoImpl>>(t).get();
            } else if (t.type() == typeid(std::shared_ptr<UnionInfoImpl>)) {
                return boost::get<std::shared_ptr<UnionInfoImpl>>(t).get();
            } else {
                throw std::runtime_error("unexpected variant value");
            }
        }
    }

    std::unordered_map<std::string, std::shared_ptr<TypeInfo>> type_registry;
    std::unordered_map<size_t, std::shared_ptr<TypeInfo>> inferred_type_registry;

    TypeInfo& type_of_(void *dummy_addr) {
        return *inferred_type_registry.at(reinterpret_cast<size_t>(dummy_addr));
    }

    TypeInfo& type_of_(std::string name) {
        return *type_registry.at(name);
    }

    TypeInfo& type_of_(const std::type_info& info) {
        return type_of_(demangle(std::string(info.name())));
    }

    NamespaceInfo& root_namespace() {
        return ROOT_NAMESPACE;
    }

    NamespaceInfo& namespace_of_(std::string name) {
        return *namespaces.at(name);
    }

    static size_t get_offset(Dwarf::Die &die) {
        std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_data_member_location);
        if (attr) {
            return attr->as<Dwarf::Unsigned>();
        }
        return static_cast<size_t>(-1);
    }

    static std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Off offset);
    static std::shared_ptr<TypeInfo> get_type_attr(BuildContext &ctx, Dwarf::Die &die);
    static std::shared_ptr<TypeInfo> build_struct_type(Dwarf::Die &die, BuildContext& ctx, bool register_parent);
    static std::shared_ptr<TypeInfo> build_union_type(Dwarf::Die &die, BuildContext& ctx, bool register_parent);

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
        std::string name = die.get_name() ?: "";

        const auto t = ctx.types.find(die.get_offset());
        if (t != ctx.types.end()) {
            type = t->second;
            if (register_parent) {
                MutableChild *child = dynamic_cast<MutableChild *>(&*type);
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
                case DW_TAG_unspecified_type: {
                    auto t = std::make_shared<UnspecifiedTypeInfoImpl>(die.get_name());
                    ctx.types[die.get_offset()] = t;

                    // C++11 requires sizeof(nullptr_t) == sizeof(void*)
                    if (name == "decltype(nullptr)")
                        t->size_ = sizeof(void*);

                    if (register_parent)
                        t->set_parent(parent);

                    type = t;
                } break;
                case DW_TAG_class_type: {
                    type = build_struct_type(die, ctx, register_parent);
                } break;
                case DW_TAG_structure_type: {
                    type = build_struct_type(die, ctx, register_parent);
                } break;
                case DW_TAG_union_type: {
                    type = build_union_type(die, ctx, register_parent);
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

                    auto subtype = get_type_attr(ctx, die);
                    if (!subtype)
                        break;

                    if (register_parent)
                        t->set_parent(parent);

                    t->set_type(subtype);
                    type = t;
                } break;
                case DW_TAG_typedef: {
                    const char *name = die.get_name();
                    if (!name)
                        break;

                    std::shared_ptr<TypeDefInfoImpl> t = std::make_shared<TypeDefInfoImpl>(name);
                    ctx.types[die.get_offset()] = t;

                    auto subtype = get_type_attr(ctx, die);
                    if (!subtype)
                        break;

                    if (register_parent)
                        t->set_parent(parent);

                    t->set_type(subtype);
                    type = t;
                } break;
                default: break;
            }
            if (type && register_parent)
                add_type_to_parent(ctx, type);
        }
        return type;
    }

    static std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Die& die) {
        auto it = ctx.types.find(die.get_offset());
        if (it == ctx.types.end()) {
            return build_type(die, ctx, false);
        }
        return it->second;
    }

    static std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Off offset) {
        auto it = ctx.types.find(offset);
        if (it == ctx.types.end()) {
            std::shared_ptr<Dwarf::Die> die = ctx.dbg.offdie(offset);
            return die ? build_type(*die, ctx, false) : nullptr;
        }
        return it->second;
    }

    static std::shared_ptr<TypeInfo> get_type_attr(BuildContext &ctx, Dwarf::Die &die) {
        std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_type);
        if (!attr)
            return nullptr;

        auto target = attr->as_die();
        return target ? get_type(ctx, *target) : nullptr;
    }

    static Dwarf::Die::TraversalResult handle_member(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        StructInfoImpl& structinfo = *boost::get<std::shared_ptr<StructInfoImpl>>(ctx.type_stack.top());
        switch (tag.get_id()) {
            case DW_TAG_member: {
                std::string name(die.get_name() ?: "");

                auto type = get_type_attr(ctx, die);
                if (!type)
                    break;

                std::string prefix("insight_annotation");
                if (name.substr(0, prefix.size()) == prefix) {
                    std::unique_ptr<const Dwarf::Attribute> locattr = die.get_attribute(DW_AT_location);
                    std::unique_ptr<const Dwarf::Attribute> constattr = die.get_attribute(DW_AT_const_value);

                    std::shared_ptr<AnnotationInfoImpl> annotation;

                    std::shared_ptr<ConstTypeInfo> constType = std::dynamic_pointer_cast<ConstTypeInfo>(type);
                    std::string annotationName = constType ? constType->type().name() : type->name();

                    if (locattr) {
                        size_t loc = locattr->as<Dwarf::Off>();
                        void *addr = reinterpret_cast<void*>(loc);

                        annotation = std::make_shared<AnnotationInfoImpl>(annotationName, addr, type);
                    } else if (constattr) {
                        Dwarf::Block* block = constattr->as<Dwarf::Block*>();

                        // we leak the block because we need it alive until the program ends
                        void *addr = std::malloc(block->bl_len);
                        std::memcpy(addr, block->bl_data, block->bl_len);
                        std::shared_ptr<const Dwarf::Debug> dbg = die.get_debug();
                        if (dbg)
                            dbg->dealloc(block);

                        annotation = std::make_shared<AnnotationInfoImpl>(annotationName, addr, type);
                    }

                    if (!annotation)
                        break;

                    size_t off = get_src_location_offset(die);
                    if (off)
                        ctx.annotations[off] = annotation;
                } else {

                    size_t offset = get_offset(die);
                    if (offset == static_cast<size_t>(-1))
                        break;

                    std::shared_ptr<FieldInfoImpl> finfo = std::make_shared<FieldInfoImpl>(die.get_name(), offset, type, structinfo);
                    structinfo.add_field(finfo);

                    size_t off = get_src_location_offset(die);
                    if (off)
                        ctx.annotated[off] = AnyAnnotated(finfo);
                }
            } break;
            case DW_TAG_subprogram: {
                if (!die.get_name()) // ignore unnamed methods
                    break;

                auto return_type = get_type_attr(ctx, die);
                if (!return_type)
                    return_type = VOID_TYPE;

                std::shared_ptr<MethodInfoImpl> method = std::make_shared<MethodInfoImpl>(die.get_name(), return_type, structinfo);

                std::unique_ptr<const Dwarf::Attribute> vattr = die.get_attribute(DW_AT_virtuality);
                std::unique_ptr<const Dwarf::Attribute> vtabattr = die.get_attribute(DW_AT_vtable_elem_location);
                if (vattr && vtabattr) {
                    Dwarf::Signed virtuality = vattr->as<Dwarf::Signed>();
                    if (virtuality != DW_VIRTUALITY_none)
                        method->set_vtable_index(vtabattr->as<uint64_t>());
                }

                Dwarf::Off off = die.get_offset();
                ctx.methods.insert(std::make_pair(off, StructOrUnionMethod(method)));

                structinfo.add_method(method);

                size_t foff = get_src_location_offset(die);
                if (foff)
                    ctx.annotated[foff] = AnyAnnotated(method);
            } break;
            case DW_TAG_inheritance: {
                auto super_type = get_type_attr(ctx, die);
                if (!super_type)
                    break;

                structinfo.add_supertype(std::dynamic_pointer_cast<StructInfo>(super_type));
            } break;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    static Dwarf::Die::TraversalResult handle_union_member(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        UnionInfoImpl& unioninfo = *boost::get<std::shared_ptr<UnionInfoImpl>>(ctx.type_stack.top());
        switch (tag.get_id()) {
            case DW_TAG_member: {
                auto type = get_type_attr(ctx, die);
                if (!type)
                    break;

                std::shared_ptr<UnionFieldInfoImpl> finfo = std::make_shared<UnionFieldInfoImpl>(die.get_name(), type, unioninfo);
                unioninfo.add_field(finfo);

                size_t off = get_src_location_offset(die);
                if (off)
                    ctx.annotated[off] = AnyAnnotated(finfo);
            } break;
            case DW_TAG_subprogram: {
                if (!die.get_name()) // ignore unnamed methods
                    break;

                auto return_type = get_type_attr(ctx, die);
                if (!return_type)
                    return_type = VOID_TYPE;

                std::shared_ptr<UnionMethodInfoImpl> method = std::make_shared<UnionMethodInfoImpl>(die.get_name(), return_type, unioninfo);

                Dwarf::Off off = die.get_offset();
                ctx.methods.insert(std::make_pair(off, StructOrUnionMethod(method)));

                unioninfo.add_method(method);

                size_t foff = get_src_location_offset(die);
                if (foff)
                    ctx.annotated[foff] = AnyAnnotated(method);
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

        ctx.type_stack.push(StructOrUnion(structinfo));
        die.traverse_headless(handle_member, &ctx);
        ctx.type_stack.pop();

        size_t off = get_src_location_offset(die);
        if (off)
            ctx.annotated[off] = AnyAnnotated(structinfo);

        return structinfo;
    }

    static std::shared_ptr<TypeInfo> build_union_type(Dwarf::Die &die, BuildContext& ctx, bool register_parent) {
        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        size_t size = !attrsize ? 0 : attrsize->as<Dwarf::Off>();

        Container* parent = get_parent(ctx);

        std::string name = die.get_name() ?: ("anonymous#" + std::to_string(ctx.anonymous_count++));
        std::shared_ptr<UnionInfoImpl> info = std::make_shared<UnionInfoImpl>(name, size);
        ctx.types[die.get_offset()] = info;

        if (register_parent)
            info->set_parent(parent);

        ctx.type_stack.push(StructOrUnion(info));
        die.traverse_headless(handle_union_member, &ctx);
        ctx.type_stack.pop();

        size_t off = get_src_location_offset(die);
        if (off)
            ctx.annotated[off] = AnyAnnotated(info);

        return info;
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

                auto type = get_type_attr(ctx, die);
                if (!type)
                    break;

                std::unique_ptr<const Dwarf::Attribute> locattr = die.get_attribute(DW_AT_location);
                if (!locattr)
                    break;

                size_t loc = locattr->as<Dwarf::Off>();

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
                    namespaces[ns->fullname()] = ns;
                }
                ctx.namespace_stack.push(&*ns);
                die.traverse_headless(build_metadata, data);
                ctx.namespace_stack.pop();

                size_t off = get_src_location_offset(die);
                if (off)
                    ctx.annotated[off] = AnyAnnotated(ns);
            } return Dwarf::Die::TraversalResult::SKIP;
            case DW_TAG_unspecified_type:
            case DW_TAG_const_type:
            case DW_TAG_base_type:
            case DW_TAG_class_type:
            case DW_TAG_structure_type:
            case DW_TAG_union_type:
            case DW_TAG_pointer_type:
            case DW_TAG_typedef: {
                std::shared_ptr<TypeInfo> type = build_type(die, ctx, true);
                if (type) {
                    std::string unprefixed_name = type->fullname().substr(2, type->fullname().size() - 2);

                    type_registry[type->fullname()] = type;
                    type_registry[unprefixed_name] = type;
                    // Special cases for easy lookup in the C language
                    switch (tag.get_id()) {
                        case DW_TAG_structure_type: type_registry["struct " + unprefixed_name] = type; break;
                        case DW_TAG_union_type:     type_registry["union "  + unprefixed_name] = type; break;
                        default: break;
                    }
                }
            } break;
            case DW_TAG_subprogram: {
                std::unique_ptr<const Dwarf::Attribute> attrspec = die.get_attribute(DW_AT_specification);
                if (!attrspec) {
                    die.traverse_headless(infer_types, &ctx);

                    if (!die.get_name()) // ignore unnamed functions
                        break;

                    auto return_type = get_type_attr(ctx, die);
                    if (!return_type)
                        return_type = VOID_TYPE;

                    std::shared_ptr<FunctionInfoImpl> func = std::make_shared<FunctionInfoImpl>(die.get_name(), return_type, *parent);

                    add_func_to_parent(ctx, func);

                    std::unique_ptr<const Dwarf::Attribute> attraddr = die.get_attribute(DW_AT_low_pc);
                    if (!attraddr)
                        break;
                    Dwarf::Addr addr = attraddr->as<Dwarf::Addr>();

                    func->address_ = reinterpret_cast<void*>(addr);

                    size_t off = get_src_location_offset(die);
                    if (off)
                        ctx.annotated[off] = AnyAnnotated(func);
                } else {
                    Dwarf::Off off = attrspec->as<Dwarf::Off>();
                    MethodOffsetMap::iterator it = ctx.methods.find(off);
                    if (it != ctx.methods.end()) {
                        struct AddMethod : boost::static_visitor<void> {
                            void operator()(std::shared_ptr<MethodInfoImpl>& method) {
                                if (method->is_virtual())
                                    return;

                                std::unique_ptr<const Dwarf::Attribute> attraddr = die.get_attribute(DW_AT_low_pc);
                                if (!attraddr)
                                    return;
                                Dwarf::Addr addr = attraddr->as<Dwarf::Addr>();

                                method->address_ = reinterpret_cast<void*>(addr);
                            }

                            void operator()(std::shared_ptr<UnionMethodInfoImpl>& method) {
                                std::unique_ptr<const Dwarf::Attribute> attraddr = die.get_attribute(DW_AT_low_pc);
                                if (!attraddr)
                                    return;
                                Dwarf::Addr addr = attraddr->as<Dwarf::Addr>();

                                method->address_ = reinterpret_cast<void*>(addr);
                            }

                            AddMethod(Dwarf::Die& d) : die(d) {}

                            Dwarf::Die& die;
                        };

                        AddMethod visitor(die);
                        it->second.apply_visitor(visitor);
                    }
                }
            } break;
            case DW_TAG_variable: {
                const char *name = die.get_name();
                auto locattr = die.get_attribute(DW_AT_location);
                if (!name || !locattr)
                    break;

                auto type = get_type_attr(ctx, die);
                if (!type)
                    break;

                std::string prefix("insight_annotation");
                if (std::string(name).substr(0, prefix.size()) == prefix) {
                    size_t loc = locattr->as<Dwarf::Off>();
                    void *addr = reinterpret_cast<void*>(loc);

                    std::shared_ptr<ConstTypeInfo> constType = std::dynamic_pointer_cast<ConstTypeInfo>(type);
                    std::string annotationName = constType ? constType->type().name() : type->name();

                    size_t off = get_src_location_offset(die);
                    if (off)
                        ctx.annotations[off] = std::make_shared<AnnotationInfoImpl>(annotationName, addr, type);
                } else {
                    size_t loc = locattr->as<Dwarf::Off>();
                    void *addr = reinterpret_cast<void*>(loc);

                    auto var = std::make_shared<VariableInfoImpl>(name, addr, type, *parent);

                    add_var_to_parent(ctx, var);

                    size_t off = get_src_location_offset(die);
                    if (off)
                        ctx.annotated[off] = AnyAnnotated(var);
                }
            } break;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    void process_annotations(BuildContext& ctx) {

        auto annotated = ctx.annotated.begin();
        auto annotation = ctx.annotations.begin();

        for (; annotation != ctx.annotations.end(); ++annotation) {
            // skip until we reach the end or the next annotated element
            for (; annotated != ctx.annotated.end() && annotated->first <= annotation->first; ++annotated);

            if (annotated == ctx.annotated.end())
                break;

            AddAnnotation visitor(annotation->second);
            annotated->second.apply_visitor(visitor);
        }

        ctx.annotated.clear();
        ctx.annotations.clear();
    }

    void initialize() {
        std::shared_ptr<const Dwarf::Debug> dbg = Dwarf::Debug::self();
        type_registry["void"] = VOID_TYPE;

        BuildContext ctx(*dbg);
        ctx.namespace_stack.push(&ROOT_NAMESPACE);

        for (const Dwarf::CompilationUnit &cu : *dbg) {
            cu.get_die()->traverse_headless(Insight::build_metadata, &ctx);

            process_annotations(ctx);
        }

    }
}

static class Init {
public:
    Init() { Insight::initialize(); }
} init;
