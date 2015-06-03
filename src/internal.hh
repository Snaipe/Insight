#ifndef INSIGHT_INTERNAL_HH
# define INSIGHT_INTERNAL_HH

# include <libdwarf++/dwarf.hh>
# include "insight/types"
# include "insight/range"

extern Dwarf::Debug &dbg;

namespace Insight {

    extern std::unordered_map<std::string, std::unique_ptr<StructInfo>> classes_by_absolute_name;
    //extern NamespaceInfo root_namespace;

    class FieldInfoImpl : public FieldInfo {
    public:
        FieldInfoImpl(const char *name, size_t offset, std::weak_ptr<TypeInfo> type);
        virtual const std::string& name() const;
        virtual const TypeInfo& type() const;
        virtual const size_t offset() const;
    private:
        size_t offset_;
        std::string name_;
        std::weak_ptr<TypeInfo> type_;
    };

    class MethodInfoImpl : public MethodInfo {
    public:
        MethodInfoImpl(const char *name, void* address);
        virtual const std::string& name() const override;
        virtual const void* address() const override;
    private:
        std::string name_;
        void* address_;
    };

    class StructInfoImpl : public StructInfo {
    public:
        StructInfoImpl(const char *name);
        virtual const std::string& name() const override;
        virtual const Range<MethodInfo> methods() const override;
        virtual const Range<FieldInfo> fields() const override;
        virtual const MethodInfo& method(std::string name) const override;
        virtual const FieldInfo& field(std::string name) const override;
        void add_field(std::unique_ptr<FieldInfo>& field);
        void add_method(std::unique_ptr<MethodInfo>& method);
    private:
        std::string name_;
        RangeCollection<MethodInfo> methods_;
        RangeCollection<FieldInfo> fields_;
    };
}

#endif /* !INSIGHT_INTERNAL_HH */
