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
#include "dwarf.hh"
#include "type.hh"
#include "inference.hh"
#include "annotation.hh"
#include "util/mangle.hh"

namespace Insight {

    template <typename S>
    struct get_superclass : public boost::static_visitor<std::shared_ptr<S>> {
        template <typename T>
        std::shared_ptr<S> operator()(std::shared_ptr<T>& info) const {
            return std::dynamic_pointer_cast<S>(info);
        }
    };

    std::shared_ptr<NamespaceInfoImpl> ROOT_NAMESPACE = std::make_shared<NamespaceInfoImpl>("");
    std::unordered_map<std::string, std::shared_ptr<NamespaceInfo>> namespaces;

    std::shared_ptr<TypeInfo> VOID_TYPE = std::make_shared<PrimitiveTypeInfoImpl>("void", 0, PrimitiveKind::VOID, ROOT_NAMESPACE);

    BuildContext::BuildContext(const Dwarf::Debug& d)
            : dbg(d)
            , types()
            , methods()
            , anonymous_count(0)
            , container_stack()
            , annotations()
    {}

    std::shared_ptr<Container> get_parent(BuildContext& ctx) {
        return boost::apply_visitor(get_superclass<Container>(), ctx.container_stack.top());
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
        return *ROOT_NAMESPACE;
    }

    NamespaceInfo& namespace_of_(std::string name) {
        return *namespaces.at(name);
    }

    size_t get_offset(Dwarf::Die &die) {
        std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_data_member_location);
        if (attr) {
            return attr->as<Dwarf::Unsigned>();
        }
        return static_cast<size_t>(-1);
    }

    static Dwarf::Die::TraversalResult build_metadata(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        std::shared_ptr<Container> parent = get_parent(ctx);

        switch (tag.get_id()) {
            case DW_TAG_namespace: {
                std::shared_ptr<NamespaceInfoImpl> parentns = boost::get<std::shared_ptr<NamespaceInfoImpl>>(ctx.container_stack.top());
                std::shared_ptr<NamespaceInfoImpl> ns;
                auto it = parentns->nested_namespaces_.find(die.get_name());
                if (it != parentns->nested_namespaces_.end()) {
                    ns = std::dynamic_pointer_cast<NamespaceInfoImpl>(it->second);
                } else {
                    ns = std::make_shared<NamespaceInfoImpl>(die.get_name(), parent);
                    parentns->add_nested_namespace(ns);
                    namespaces[ns->fullname()] = ns;
                }
                ctx.container_stack.push(AnyContainer(ns));
                die.traverse_headless(build_metadata, data);
                ctx.container_stack.pop();

                mark_element_line(ctx, die, ns);
            } return Dwarf::Die::TraversalResult::SKIP;
            case DW_TAG_unspecified_type:
            case DW_TAG_const_type:
            case DW_TAG_base_type:
            case DW_TAG_class_type:
            case DW_TAG_structure_type:
            case DW_TAG_union_type:
            case DW_TAG_pointer_type:
            case DW_TAG_enumeration_type:
            case DW_TAG_typedef: {
                std::shared_ptr<TypeInfo> type = build_type(die, ctx, true);
                if (type) {
                    std::string unprefixed_name = type->fullname().substr(2, type->fullname().size() - 2);

                    type_registry[type->fullname()] = type;
                    type_registry[unprefixed_name] = type;
                    // Special cases for easy lookup in the C language
                    switch (tag.get_id()) {
                        case DW_TAG_structure_type:     type_registry["struct " + unprefixed_name] = type; break;
                        case DW_TAG_union_type:         type_registry["union "  + unprefixed_name] = type; break;
                        case DW_TAG_enumeration_type:   type_registry["enum "   + unprefixed_name] = type; break;
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

                    std::shared_ptr<FunctionInfoImpl> func = std::make_shared<FunctionInfoImpl>(die.get_name(), return_type, parent);

                    add_func_to_parent(ctx, func);

                    std::unique_ptr<const Dwarf::Attribute> attraddr = die.get_attribute(DW_AT_low_pc);
                    if (!attraddr)
                        break;
                    Dwarf::Addr addr = attraddr->as<Dwarf::Addr>();

                    func->address_ = reinterpret_cast<void*>(addr);

                    mark_element_line(ctx, die, func);
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

                    auto var = std::make_shared<VariableInfoImpl>(name, addr, type, parent);

                    add_var_to_parent(ctx, var);

                    mark_element_line(ctx, die, var);
                }
            } break;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    void initialize() {
        std::shared_ptr<const Dwarf::Debug> dbg = Dwarf::Debug::self();
        type_registry["void"] = VOID_TYPE;

        BuildContext ctx(*dbg);
        ctx.container_stack.push(AnyContainer(ROOT_NAMESPACE));

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