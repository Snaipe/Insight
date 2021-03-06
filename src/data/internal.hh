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
# include <unordered_set>
# include "insight/types"
# include "insight/range"

#define MIXIN(Name, Decl, Type)                                         \
    class Decl ## Container {                                           \
    public:                                                             \
        virtual const Range<Type> Name ## s() const = 0;                \
        virtual Type& Name(std::string name) const = 0;                 \
        virtual void add_ ## Name(std::shared_ptr<Type> Name) = 0;      \
    };                                                                  \
                                                                        \
    template <typename T>                                               \
    class Decl ## ContainerBase : public T, virtual public Decl ## Container {  \
    public:                                                             \
        Decl ## ContainerBase() : T(), Name ## s_() {}                  \
                                                                        \
        virtual const Range<Type> Name ## s() const override {          \
            return Range<Type>(Name ## s_);                             \
        }                                                               \
                                                                        \
        virtual Type& Name(std::string name) const override {           \
            return *Name ## s_.at(name);                                \
        }                                                               \
                                                                        \
        virtual void add_ ## Name(std::shared_ptr<Type> Name) override { \
            Name ## s_[Name->name()] = Name;                            \
        }                                                               \
                                                                        \
        RangeCollection<Type> Name ## s_;                               \
    }

#define WEAK_MIXIN(Name, Decl, Type)                                    \
    class Decl ## Container {                                           \
    public:                                                             \
        virtual const WeakRange<Type> Name ## s() const = 0;            \
        virtual Type& Name(std::string name) const = 0;                 \
        virtual void add_ ## Name(std::weak_ptr<Type> Name) = 0;        \
    };                                                                  \
                                                                        \
    template <typename T>                                               \
    class Decl ## ContainerBase : public T, virtual public Decl ## Container {  \
    public:                                                             \
        Decl ## ContainerBase() : T(), Name ## s_() {}                  \
                                                                        \
        virtual const WeakRange<Type> Name ## s() const override {      \
            return WeakRange<Type>(Name ## s_);                         \
        }                                                               \
                                                                        \
        virtual Type& Name(std::string name) const override {           \
            return *Name ## s_.at(name).lock();                         \
        }                                                               \
                                                                        \
        virtual void add_ ## Name(std::weak_ptr<Type> Name) override {  \
            Name ## s_[Name.lock()->name()] = Name;                     \
        }                                                               \
                                                                        \
        WeakRangeCollection<Type> Name ## s_;                           \
    }


extern Dwarf::Debug &dbg;

namespace Insight {

    template <class T>
    class NameBase : public T {
    public:
        NameBase() : T(), fullname_(""), name_("") {}
        NameBase(const std::string& name) : fullname_(name), name_(name) {}
        NameBase(std::string&& name) : fullname_(name), name_(name) {}
        NameBase(const std::string& parent, const std::string& name) : fullname_(parent + "::" + name), name_(name) {}
        virtual const std::string& name() const override {
            return name_;
        }
        virtual const std::string& fullname() const override {
            return fullname_;
        }

        void set_fullname(const std::string& parent, const std::string& name) {
            fullname_ = parent + "::" + name;
        }

        std::string fullname_;
        std::string name_;
    };

    class MutableChild : virtual public Child {
    public:
        virtual void set_parent(std::shared_ptr<Container> parent) = 0;
    };

    MIXIN(annotation, AnnotationInfo, AnnotationInfo);

    template <class T>
    class ChildBase : public NameBase<AnnotationInfoContainerBase<T>>, virtual public MutableChild {
    public:
        ChildBase() : NameBase<AnnotationInfoContainerBase<T>>(), parent_() {}
        ChildBase(std::shared_ptr<Container> parent) : NameBase<AnnotationInfoContainerBase<T>>(), parent_(parent) {}
        ChildBase(const std::string &name) : NameBase<AnnotationInfoContainerBase<T>>(name), parent_() {}
        ChildBase(std::string&& name) : NameBase<AnnotationInfoContainerBase<T>>(name), parent_() {}
        ChildBase(const std::string &name, std::shared_ptr<Container> parent) : NameBase<AnnotationInfoContainerBase<T>>(parent->fullname(), name), parent_(parent) {}
        ChildBase(std::string&& name, std::shared_ptr<Container> parent) : NameBase<AnnotationInfoContainerBase<T>>(parent->fullname(), name), parent_(parent) {}

        virtual Container& parent() const override {
            return *parent_.lock();
        }

        virtual void set_parent(std::shared_ptr<Container> parent) override {
            parent_ = parent;
            NameBase<AnnotationInfoContainerBase<T>>::set_fullname(parent->fullname(), this->name());
        }

        std::weak_ptr<Container> parent_;
    };

    template <class T>
    class TypeBase : public ChildBase<T> {
    public:
        TypeBase() : ChildBase<T>(), size_(0) {}
        TypeBase(const std::string &name) : ChildBase<T>(name), size_(0) {}
        TypeBase(std::string&& name) : ChildBase<T>(name), size_(0) {}
        TypeBase(const std::string& name, size_t size) : ChildBase<T>(name), size_(size) {}
        TypeBase(std::string&& name, size_t size) : ChildBase<T>(name), size_(size) {}
        TypeBase(const std::string& name, size_t size, std::shared_ptr<Container> parent) : ChildBase<T>(name, parent), size_(size) {}
        TypeBase(std::string&& name, size_t size, std::shared_ptr<Container> parent) : ChildBase<T>(name, parent), size_(size) {}

        virtual size_t size_of() const override {
            return size_;
        };

        virtual void *allocate() const {
            return ::operator new(size_of());
        }

        size_t size_;
    };

    MIXIN(function, FunctionInfo, FunctionInfo);
    MIXIN(method, MethodInfo, MethodInfo);
    MIXIN(field, FieldInfo, FieldInfo);
    MIXIN(variable, VariableInfo, VariableInfo);
    MIXIN(method, UnionMethodInfo, UnionMethodInfo);
    MIXIN(field, UnionFieldInfo, UnionFieldInfo);
    MIXIN(nested_namespace, NamespaceInfo, NamespaceInfo);
    MIXIN(type, TypeInfo, TypeInfo);
    MIXIN(value, EnumConstantInfo, EnumConstantInfo);
    WEAK_MIXIN(supertype, Supertype, StructInfo);

    template <typename T>
    using ContainerBase = FunctionInfoContainerBase<VariableInfoContainerBase<TypeInfoContainerBase<T>>>;

    template <typename T>
    using NamespaceBase = ContainerBase<NamespaceInfoContainerBase<T>>;

    using StructTypeBase = ContainerBase<MethodInfoContainerBase<FieldInfoContainerBase<SupertypeContainerBase<StructInfo>>>>;
    using UnionTypeBase = ContainerBase<UnionMethodInfoContainerBase<UnionFieldInfoContainerBase<UnionInfo>>>;


    template <class T>
    class TypedBase : public ChildBase<T> {
    public:
        TypedBase(const char *name, std::weak_ptr<TypeInfo> type, std::shared_ptr<Container> parent)
            : ChildBase<T>(std::string(name), parent)
            , type_(type)
        {}

        TypedBase(const char *name, std::weak_ptr<TypeInfo> type)
            : ChildBase<T>(std::string(name))
            , type_(type)
        {}

        TypedBase(std::string name, std::weak_ptr<TypeInfo> type)
            : ChildBase<T>(name)
            , type_(type)
        {}

        virtual TypeInfo& type() const override {
            return *type_.lock();
        }

        std::weak_ptr<TypeInfo> type_;
    };

    class FieldInfoImpl : public TypedBase<FieldInfo> {
    public:
        FieldInfoImpl(const char *name, size_t offset, std::weak_ptr<TypeInfo> type, std::shared_ptr<Container> parent);
        virtual size_t offset() const override;

        size_t offset_;
    };

    class UnionFieldInfoImpl : public TypedBase<UnionFieldInfo> {
    public:
        UnionFieldInfoImpl(const char *name, std::weak_ptr<TypeInfo> type, std::shared_ptr<Container> parent);
    };

    class VariableInfoImpl : public TypedBase<VariableInfo> {
    public:
        VariableInfoImpl(const char *name, void* address, std::weak_ptr<TypeInfo> type, std::shared_ptr<Container> parent);
        virtual void* address() const override;

        void* address_;
    };

    template <class T>
    class CallableBase : public ChildBase<T> {
    public:
        CallableBase(const char *name, std::weak_ptr<TypeInfo> return_type, std::shared_ptr<Container> parent)
                : ChildBase<T>(std::string(name), parent)
                , address_(nullptr)
                , return_type_(return_type)
                , parameters_()
        {}

        virtual void* address() const override {
            return address_;
        }

        virtual TypeInfo& return_type() const override {
            return *return_type_.lock();
        }

        virtual const Range<ParameterInfo> parameters() const override {
            return Range<ParameterInfo>(parameters_);
        }

        void* address_;
        std::weak_ptr<TypeInfo> return_type_;
        RangeCollection<ParameterInfo> parameters_;
    };

    class MethodInfoImpl : public CallableBase<MethodInfo> {
    public:
        MethodInfoImpl(const char *name, std::weak_ptr<TypeInfo> return_type, std::shared_ptr<Container> parent);
        virtual bool is_virtual() const override;
        virtual size_t vtable_index() const override;

        void set_vtable_index(size_t index);

        bool virtual_;
        size_t vtab_index_;
    };

    class UnionMethodInfoImpl : public CallableBase<UnionMethodInfo> {
    public:
        UnionMethodInfoImpl(const char *name, std::weak_ptr<TypeInfo> return_type, std::shared_ptr<Container> parent);
    };

    class FunctionInfoImpl : public CallableBase<FunctionInfo> {
    public:
        FunctionInfoImpl(const char *name, std::weak_ptr<TypeInfo> return_type, std::shared_ptr<Container> parent);
    };

    class StructInfoImpl : public TypeBase<StructTypeBase> {
    public:
        StructInfoImpl(std::string& name, size_t size);

        virtual bool is_supertype(const TypeInfo &type) const override;
        virtual bool is_ancestor(const TypeInfo &type) const override;
        virtual void add_supertype(std::weak_ptr<StructInfo> supertype) override;

        std::unordered_set<std::string> ancestors_;
    };

    class UnionInfoImpl : public TypeBase<UnionTypeBase> {
    public:
        UnionInfoImpl(std::string& name, size_t size);
    };

    class UnspecifiedTypeInfoImpl : public TypeBase<UnspecifiedTypeInfo> {
    public:
        UnspecifiedTypeInfoImpl(const char* name);
    };

    class PrimitiveTypeInfoImpl : public TypeBase<PrimitiveTypeInfo> {
    public:
        PrimitiveTypeInfoImpl(const char* name, size_t size, PrimitiveKind kind, std::shared_ptr<Container> parent);
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

    class NamespaceInfoImpl : public ChildBase<NamespaceBase<NamespaceInfo>> {
    public:
        NamespaceInfoImpl(const char* name);
        NamespaceInfoImpl(const char* name, std::shared_ptr<Container> parent);

        virtual Container& parent() const override;
    };

    class AnnotationInfoImpl : public TypedBase<AnnotationInfo> {
    public:
        AnnotationInfoImpl(std::string name, void *data, std::weak_ptr<TypeInfo> type);
        virtual void* data_ptr() const override;
        virtual Annotated& annotated_element() const override;
        void set_annotated(std::shared_ptr<Annotated>& annotated);

        void* data_;
        std::weak_ptr<Annotated> annotated_;
    };

    class EnumConstantInfoImpl : public NameBase<EnumConstantInfo> {
    public:
        EnumConstantInfoImpl(const char* name, void *data, std::shared_ptr<EnumInfo>& type);
        virtual void* data_ptr() const override;
        virtual EnumInfo& type() const override;

        void* data_;
        std::weak_ptr<EnumInfo> type_;
    };

    class EnumInfoImpl : public TypeBase<EnumConstantInfoContainerBase<EnumInfo>> {
    public:
        EnumInfoImpl(std::string name, size_t size);
    };

    class ParameterInfoImpl : public TypedBase<ParameterInfo> {
    public:
        ParameterInfoImpl(std::string name, std::weak_ptr<TypeInfo> type);
        virtual size_t index() const override;

        size_t index_;
    };
}

#endif /* !INSIGHT_INTERNAL_HH */
