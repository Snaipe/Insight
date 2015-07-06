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
#ifndef INSIGHT_INTERNAL_HH
# define INSIGHT_INTERNAL_HH

# include <libdwarf++/dwarf.hh>
# include "insight/types"
# include "insight/range"

extern Dwarf::Debug &dbg;

namespace Insight {

    template <class T>
    class NameBase : public T {
    public:
        NameBase() : name_("") {}
        NameBase(const std::string& name) : name_(name) {}
        NameBase(std::string&& name) : name_(name) {}
        virtual const std::string& name() const override {
            return name_;
        }

        std::string name_;
    };

    template <class T>
    class TypeBase : public NameBase<T> {
    public:
        TypeBase() : NameBase<T>(), size_(0) {}
        TypeBase(const std::string &name) : NameBase<T>(name), size_(0) {}
        TypeBase(std::string&& name) : NameBase<T>(name), size_(0) {}
        TypeBase(const std::string& name, size_t size) : NameBase<T>(name), size_(size) {}
        TypeBase(std::string&& name, size_t size) : NameBase<T>(name), size_(size) {}
        virtual size_t size_of() const override {
            return size_;
        };

        size_t size_;
    };

    class FieldInfoImpl : public NameBase<FieldInfo> {
    public:
        FieldInfoImpl(const char *name, size_t offset, std::weak_ptr<TypeInfo> type);
        virtual const TypeInfo& type() const override;
        virtual const size_t offset() const override;

        size_t offset_;
        std::weak_ptr<TypeInfo> type_;
    };

    class MethodInfoImpl : public NameBase<MethodInfo> {
    public:
        MethodInfoImpl(const char *name, std::weak_ptr<TypeInfo> return_type);
        virtual const void* address() const override;
        virtual const bool is_virtual() const override;
        virtual const TypeInfo& return_type() const override;
        virtual const Range<TypeInfo> parameter_types() const override;

        void* address_;
        bool virtual_;
        std::weak_ptr<TypeInfo> return_type_;
        RangeCollection<TypeInfo> parameters_;
    };

    class StructInfoImpl : public TypeBase<StructInfo> {
    public:
        StructInfoImpl(std::string& name, size_t size);
        virtual const Range<MethodInfo> methods() const override;
        virtual const Range<FieldInfo> fields() const override;
        virtual const MethodInfo& method(std::string name) const override;
        virtual const FieldInfo& field(std::string name) const override;
        void add_field(std::unique_ptr<FieldInfo>& field);
        void add_method(std::unique_ptr<MethodInfo>& method);

        RangeCollection<MethodInfo> methods_;
        RangeCollection<FieldInfo> fields_;
    };

    class PrimitiveTypeInfoImpl : public TypeBase<PrimitiveTypeInfo> {
    public:
        PrimitiveTypeInfoImpl(const char* name, size_t size, PrimitiveKind kind);
        virtual PrimitiveKind kind() const override;

        PrimitiveKind kind_;
    };

    class PointerTypeInfoImpl : public TypeBase<PointerTypeInfo> {
    public:
        PointerTypeInfoImpl();
        PointerTypeInfoImpl(std::shared_ptr<TypeInfo> type, size_t size);
        virtual TypeInfo& pointed_type() const override;

        void set_type(std::shared_ptr<TypeInfo>& type);

        std::weak_ptr<TypeInfo> type_;
    };

    class ConstTypeInfoImpl : public TypeBase<ConstTypeInfo> {
    public:
        ConstTypeInfoImpl();
        ConstTypeInfoImpl(std::shared_ptr<TypeInfo>& type);
        virtual TypeInfo& type() const override;

        void set_type(std::shared_ptr<TypeInfo>& type);

        std::weak_ptr<TypeInfo> type_;
    };

    class TypeDefInfoImpl : public TypeBase<TypeDefInfo> {
    public:
        TypeDefInfoImpl(const char* name);
        TypeDefInfoImpl(const char* name, std::shared_ptr<TypeInfo>& type);
        virtual TypeInfo& aliased_type() const override;

        void set_type(std::shared_ptr<TypeInfo>& type);

        std::weak_ptr<TypeInfo> type_;
    };
}

#endif /* !INSIGHT_INTERNAL_HH */
