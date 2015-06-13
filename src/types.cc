#include "internal.hh"

namespace Insight {

    // MethodInfo

    MethodInfoImpl::MethodInfoImpl(char const *name, std::weak_ptr<TypeInfo> return_type)
        : NameBase<MethodInfo>(std::string(name))
        , return_type_(return_type)
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

    void StructInfoImpl::add_field(std::unique_ptr<FieldInfo>& field) {
        fields_[field->name()] = std::move(field);
    }

    void StructInfoImpl::add_method(std::unique_ptr<MethodInfo>& method) {
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

    TypeInfo &PointerTypeInfoImpl::pointed_type() const {
        return *type_.lock();
    }

    ConstTypeInfoImpl::ConstTypeInfoImpl(std::shared_ptr<TypeInfo>& type)
            : TypeBase(type->name() + " const", type->size_of())
            , type_(type)
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

}
