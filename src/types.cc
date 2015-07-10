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
#include "internal.hh"

namespace Insight {

    // MethodInfo

    MethodInfoImpl::MethodInfoImpl(char const *name, std::weak_ptr<TypeInfo> return_type, Container& parent)
        : CallableBase<MethodInfo>(name, return_type, parent)
        , virtual_(false)
        , vtab_index_(0)
    {}

    const bool MethodInfoImpl::is_virtual() const {
        return virtual_;
    }

    size_t MethodInfoImpl::vtable_index() const {
        return vtab_index_;
    }

    void MethodInfoImpl::set_vtable_index(size_t index) {
        virtual_ = true;
        vtab_index_ = index;
    }

    // FunctionInfo

    FunctionInfoImpl::FunctionInfoImpl(char const *name, std::weak_ptr<TypeInfo> return_type, Container& parent)
            : CallableBase<FunctionInfo>(name, return_type, parent)
    {}

    // FieldInfo

    FieldInfoImpl::FieldInfoImpl(const char *name, size_t offset, std::weak_ptr<TypeInfo> type, Container& parent)
        : TypedBase<FieldInfo>(name, type, parent)
        , offset_(offset)
    {}

    const size_t FieldInfoImpl::offset() const {
        return offset_;
    }

    // VariableInfo

    VariableInfoImpl::VariableInfoImpl(const char *name, void* address, std::weak_ptr<TypeInfo> type, Container& parent)
        : TypedBase<VariableInfo>(name, type, parent)
        , address_(address)
    {}

    void* VariableInfoImpl::address() const {
        return address_;
    }

    // StructInfo

    StructInfoImpl::StructInfoImpl(std::string& name, size_t size)
        : TypeBase(name, size)
    {}

    // NamespaceInfo
    NamespaceInfoImpl::NamespaceInfoImpl(const char* name, Container& parent)
        : ChildBase(std::string(name), parent)
    {}

    NamespaceInfoImpl::NamespaceInfoImpl(const char* name)
            : ChildBase(std::string(name), *this)
    {}

    // PrimitiveTypeInfo

    PrimitiveTypeInfoImpl::PrimitiveTypeInfoImpl(const char* name, size_t size, PrimitiveKind kind, Container& parent)
        : TypeBase(std::string(name), size, parent)
        , kind_(kind)
    {}

    PrimitiveTypeInfoImpl::PrimitiveTypeInfoImpl(const char* name, size_t size, PrimitiveKind kind)
            : TypeBase(std::string(name), size)
            , kind_(kind)
    {}

    PrimitiveKind PrimitiveTypeInfoImpl::kind() const {
        return kind_;
    }

    PointerTypeInfoImpl::PointerTypeInfoImpl(std::shared_ptr<TypeInfo> type, size_t size)
        : TypeBase(type->name() + "*", size)
        , type_(type)
    {}

    PointerTypeInfoImpl::PointerTypeInfoImpl()
        : TypeBase()
        , type_()
    {}

    TypeInfo &PointerTypeInfoImpl::pointed_type() const {
        return *type_.lock();
    }

    ConstTypeInfoImpl::ConstTypeInfoImpl(std::shared_ptr<TypeInfo>& type)
        : TypeBase(type->name() + " const", type->size_of())
        , type_(type)
    {}

    ConstTypeInfoImpl::ConstTypeInfoImpl()
        : TypeBase()
        , type_()
    {}

    TypeInfo &ConstTypeInfoImpl::type() const {
        return *type_.lock();
    }

    TypeDefInfoImpl::TypeDefInfoImpl(const char* name, std::shared_ptr<TypeInfo>& type)
        : TypeBase(name, type->size_of())
        , type_(type)
    {}

    TypeInfo &TypeDefInfoImpl::aliased_type() const {
        return *type_.lock();
    }

    TypeDefInfoImpl::TypeDefInfoImpl(const char *name)
        : TypeBase(name)
        , type_()
    {}

    void PointerTypeInfoImpl::set_type(std::shared_ptr<TypeInfo> &type) {
        type_ = type;
        name_ = type->name() + "*";
    }

    void ConstTypeInfoImpl::set_type(std::shared_ptr<TypeInfo> &type) {
        type_ = type;
        name_ = type->name() + " const";
    }

    void TypeDefInfoImpl::set_type(std::shared_ptr<TypeInfo> &type) {
        type_ = type;
    }

}
