#include "internal.hh"

namespace Insight {

    const std::string& MethodInfoImpl::name() const {
        return name_;
    }

    MethodInfoImpl::MethodInfoImpl(char const *name, void* address)
        : name_(name)
        , address_(address)
    {}

    const std::string& FieldInfoImpl::name() const {
        return name_;
    }

    const TypeInfo& FieldInfoImpl::type() const {
        return *type_.lock();
    }

    const size_t FieldInfoImpl::offset() const {
        return offset_;
    }

    FieldInfoImpl::FieldInfoImpl(const char *name, size_t offset, std::weak_ptr<TypeInfo> type)
        : name_(name)
        , offset_(offset)
        , type_(type)
    {}

    StructInfoImpl::StructInfoImpl(const char *name)
        : name_(name)
        , fields_()
        , methods_()
    {}

    const std::string& StructInfoImpl::name() const {
        return name_;
    }

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

    const void *MethodInfoImpl::address() const {
        return address_;
    }
}
