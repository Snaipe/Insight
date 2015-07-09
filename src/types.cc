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

    MethodInfoImpl::MethodInfoImpl(char const *name, std::weak_ptr<TypeInfo> return_type)
        : NameBase<MethodInfo>(std::string(name))
        , return_type_(return_type)
        , virtual_(false)
        , vtab_index_(0)
        , address_(nullptr)
        , parameters_()
    {}

    const void *MethodInfoImpl::address() const {
        return address_;
    }

    const bool MethodInfoImpl::is_virtual() const {
        return virtual_;
    }

    const TypeInfo &MethodInfoImpl::return_type() const {
        return *return_type_.lock();
    }

    const Range<TypeInfo> MethodInfoImpl::parameter_types() const {
        return Range<TypeInfo>(parameters_);
    }

    size_t MethodInfoImpl::vtable_index() const {
        return vtab_index_;
    }

    void MethodInfoImpl::set_vtable_index(size_t index) {
        virtual_ = true;
        vtab_index_ = index;
    }

    // FieldInfo

    FieldInfoImpl::FieldInfoImpl(const char *name, size_t offset, std::weak_ptr<TypeInfo> type)
        : NameBase<FieldInfo>(std::string(name))
        , offset_(offset)
        , type_(type)
    {}

    const TypeInfo& FieldInfoImpl::type() const {
        return *type_.lock();
    }

    const size_t FieldInfoImpl::offset() const {
        return offset_;
    }

    // StructInfo

    StructInfoImpl::StructInfoImpl(std::string& name, size_t size)
        : TypeBase(name, size)
        , fields_()
        , methods_()
    {}

    Range<MethodInfo> const StructInfoImpl::methods() const {
        return Range<MethodInfo>(methods_);
    }

    Range<FieldInfo> const StructInfoImpl::fields() const {
        return Range<FieldInfo>(fields_);
    }

    void StructInfoImpl::add_field(std::shared_ptr<FieldInfo> field) {
        fields_[field->name()] = std::move(field);
    }

    void StructInfoImpl::add_method(std::shared_ptr<MethodInfo> method) {
        methods_[method->name()] = std::move(method);
    }

    const MethodInfo& StructInfoImpl::method(std::string name) const {
        return *methods_.at(name);
    }

    const FieldInfo& StructInfoImpl::field(std::string name) const {
        return *fields_.at(name);
    }

    // PrimitiveTypeInfo

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
