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
#ifndef INSIGHT_DWARF_HH
# define INSIGHT_DWARF_HH

# include <memory>
# include <unordered_map>
# include <vector>
# include <map>
# include <stack>
# include <boost/variant.hpp>
# include <libdwarf++/dwarf.hh>
# include <libdwarf++/die.hh>
# include <libdwarf++/cu.hh>
# include "insight/insight"
# include "data/internal.hh"

#define CONTAINER_VISITOR(type, Name, operation) \
    struct Name : public boost::static_visitor<void> { \
        template <typename T> \
        void operator()(std::shared_ptr<T>& info) const { operation; } \
        Name (std::shared_ptr<type>& t) : ptr(t) {} \
        std::shared_ptr<type>& ptr; \
    }

namespace Insight {

    using AnyMethod = boost::variant<std::shared_ptr<MethodInfoImpl>, std::shared_ptr<UnionMethodInfoImpl>>;

    using AnyAnnotated = boost::variant<
            std::shared_ptr<StructInfoImpl>,
            std::shared_ptr<UnionInfoImpl>,
            std::shared_ptr<EnumInfoImpl>,
            std::shared_ptr<VariableInfoImpl>,
            std::shared_ptr<FunctionInfoImpl>,
            std::shared_ptr<MethodInfoImpl>,
            std::shared_ptr<FieldInfoImpl>,
            std::shared_ptr<UnionMethodInfoImpl>,
            std::shared_ptr<UnionFieldInfoImpl>,
            std::shared_ptr<NamespaceInfoImpl>
    >;

    using AnyContainer = boost::variant<
            std::shared_ptr<StructInfoImpl>,
            std::shared_ptr<UnionInfoImpl>,
            std::shared_ptr<NamespaceInfoImpl>
    >;

    template<typename T>
    using OffsetMap = std::unordered_map<size_t, T>;

    using TypeOffsetMap = OffsetMap<std::shared_ptr<TypeInfo>>;
    using MethodOffsetMap = OffsetMap<AnyMethod>;
    using AddressOffsetMap = OffsetMap<void*>;

    struct BuildContext {
        BuildContext(const Dwarf::Debug& d);

        const Dwarf::Debug& dbg;
        TypeOffsetMap types;
        MethodOffsetMap methods;
        AddressOffsetMap method_addresses;
        int anonymous_count;
        std::stack<AnyContainer> container_stack;
        std::map<size_t, std::shared_ptr<AnnotationInfoImpl>> annotations;
        std::map<size_t, AnyAnnotated> annotated;
    };

    CONTAINER_VISITOR(TypeInfo, add_type, info->add_type(ptr));
    CONTAINER_VISITOR(FunctionInfo, add_function, info->add_function(ptr));
    CONTAINER_VISITOR(VariableInfo, add_variable, info->add_variable(ptr));

    size_t get_offset(Dwarf::Die &die);
    std::shared_ptr<Container> get_parent(BuildContext& ctx);

    extern std::shared_ptr<NamespaceInfoImpl> ROOT_NAMESPACE;
    extern std::shared_ptr<TypeInfo> VOID_TYPE;
    extern std::unordered_map<std::string, std::shared_ptr<NamespaceInfo>> namespaces;

    extern std::unordered_map<std::string, std::shared_ptr<TypeInfo>> type_registry;
    extern std::unordered_map<size_t, std::shared_ptr<TypeInfo>> inferred_type_registry;

    inline void add_type_to_parent(BuildContext& ctx, std::shared_ptr<TypeInfo> ptr) {
        boost::apply_visitor(add_type(ptr), ctx.container_stack.top());
    }

    inline void add_func_to_parent(BuildContext& ctx, std::shared_ptr<FunctionInfo> ptr) {
        boost::apply_visitor(add_function(ptr), ctx.container_stack.top());
    }

    inline void add_var_to_parent(BuildContext& ctx, std::shared_ptr<VariableInfo> ptr) {
        boost::apply_visitor(add_variable(ptr), ctx.container_stack.top());
    }

}

#endif /* !INSIGHT_DWARF_HH */
