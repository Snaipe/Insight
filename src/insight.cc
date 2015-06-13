#include <vector>
#include <unordered_map>
#include <iostream>
#include <libdwarf++/dwarf.hh>
#include <libdwarf++/die.hh>
#include <libdwarf++/cu.hh>
#include "insight/insight"
#include "internal.hh"

namespace Insight {

    template<typename T>
    using OffsetMap = std::unordered_map<size_t, T>;

    struct BuildContext {
        BuildContext(const Dwarf::Debug& d)
            : dbg(d)
            , types()
            , methods()
        {}

        const Dwarf::Debug& dbg;
        OffsetMap<std::shared_ptr<TypeInfo>> types;
        OffsetMap<MethodInfoImpl*> methods;
        void* target;
    };

    std::unordered_map<std::string, std::shared_ptr<TypeInfo>> type_registry;

    const TypeInfo& type_of(std::string name) {
        return *type_registry.at(name);
    }

    static size_t get_offset(Dwarf::Die &die) {
        std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_data_member_location);
        if (attr) {
            return attr->as<Dwarf::Unsigned>();
        }
        return static_cast<size_t>(-1);
    }

    static std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Off offset);
    static std::shared_ptr<TypeInfo> build_struct_type(Dwarf::Die &die, BuildContext& ctx);

    static std::shared_ptr<TypeInfo> build_type(Dwarf::Die& die, BuildContext& ctx) {
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

        const Dwarf::Tag &tag = die.get_tag();
        std::shared_ptr<TypeInfo> type;
        std::string name;
        switch (tag.get_id()) {
            case DW_TAG_base_type: {
                name = die.get_name();
                PrimitiveKind kind = primitiveKinds[name];
                std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
                if (!attrsize) // ignore types with no size
                    break;

                size_t size = attrsize->as<Dwarf::Off>();
                type = std::make_shared<PrimitiveTypeInfoImpl>(die.get_name(), size, kind);
            } break;
            case DW_TAG_structure_type:
            case DW_TAG_class_type: {
                if (!die.get_name()) // ignore anonymous structures
                    break;

                type = build_struct_type(die, ctx);
            } break;
            case DW_TAG_pointer_type: {
                std::unique_ptr<const Dwarf::Attribute> attrtype = die.get_attribute(DW_AT_type);
                std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
                if (!attrtype || !attrsize)
                    break;
                std::shared_ptr<TypeInfo> subtype = get_type(ctx, attrtype->as<Dwarf::Off>());
                if (!subtype)
                    break;

                type = std::make_shared<PointerTypeInfoImpl>(subtype, attrsize->as<Dwarf::Unsigned>());
            } break;
            case DW_TAG_const_type: {
                std::unique_ptr<const Dwarf::Attribute> attrtype = die.get_attribute(DW_AT_type);
                if (!attrtype)
                    break;
                std::shared_ptr<TypeInfo> subtype = get_type(ctx, attrtype->as<Dwarf::Off>());
                if (!subtype)
                    break;

                type = std::make_shared<ConstTypeInfoImpl>(subtype);
            } break;
            default: break;
        }
        if (type) {
            std::cout << type->name() << std::endl;

            type_registry[type->name()] = type;
            ctx.types[die.get_offset()] = type;
        }
        return type;
    }

    static std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Off offset) {
        auto it = ctx.types.find(offset);
        if (it == ctx.types.end())
            return nullptr;
        return it->second;
    }

    static Dwarf::Die::TraversalResult handle_member(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext *ctx = static_cast<BuildContext*>(data);

        StructInfoImpl *structinfo = static_cast<StructInfoImpl*>(ctx->target);
        switch (tag.get_id()) {
            case DW_TAG_member: {
                size_t offset = get_offset(die);
                if (offset == static_cast<size_t>(-1))
                    break;
                std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_type);
                if (!attr)
                    break;

                std::weak_ptr<TypeInfo> weak(get_type(*ctx, attr->as<Dwarf::Off>()));
                std::unique_ptr<FieldInfo> finfo = std::make_unique<FieldInfoImpl>(die.get_name(), offset, weak);
                structinfo->add_field(finfo);
            } break;
            case DW_TAG_subprogram: {
                if (!die.get_name()) // ignore unnamed methods
                    break;

                std::unique_ptr<MethodInfo> method = std::make_unique<MethodInfoImpl>(die.get_name());
                Dwarf::Off off = die.get_offset();
                ctx->methods[off] = static_cast<MethodInfoImpl*>(&*method);

                structinfo->add_method(method);
            } break;
            default: return Dwarf::Die::TraversalResult::TRAVERSE;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    static std::shared_ptr<TypeInfo> build_struct_type(Dwarf::Die &die, BuildContext& ctx) {
        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        size_t size = !attrsize ? 0 : attrsize->as<Dwarf::Off>();

        std::shared_ptr<StructInfo> structinfo = std::make_shared<StructInfoImpl>(die.get_name(), size);

        BuildContext newctx = ctx;
        newctx.target = &*structinfo;

        die.traverse(handle_member, &newctx);
        return structinfo;
    }

    static Dwarf::Die::TraversalResult build_types(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        switch (tag.get_id()) {
            case DW_TAG_const_type:
            case DW_TAG_base_type:
            case DW_TAG_class_type:
            case DW_TAG_structure_type:
            case DW_TAG_pointer_type:
                build_type(die, ctx);
                break;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    static Dwarf::Die::TraversalResult build_post_metadata(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        switch (tag.get_id()) {
            case DW_TAG_compile_unit: return Dwarf::Die::TraversalResult::TRAVERSE;
            case DW_TAG_subprogram: {
                std::unique_ptr<const Dwarf::Attribute> attrspec = die.get_attribute(DW_AT_specification);
                if (!attrspec) {
                    // TODO: handle functions
                    return Dwarf::Die::TraversalResult::SKIP;
                } else {
                    Dwarf::Off off = attrspec->as<Dwarf::Off>();
                    auto it = ctx.methods.find(off);
                    if (it != ctx.methods.end()) {
                        std::unique_ptr<const Dwarf::Attribute> attraddr = die.get_attribute(DW_AT_low_pc);
                        if (!attraddr)
                            break;
                        Dwarf::Addr addr = attraddr->as<Dwarf::Addr>();

                        MethodInfoImpl* method = it->second;
                        method->address_ = reinterpret_cast<void*>(addr);
                    }
                }
            } break;
            case DW_TAG_pointer_type: break;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    void initialize() {

        std::shared_ptr<const Dwarf::Debug> dbg = Dwarf::Debug::self();

        BuildContext ctx(*dbg);

        for (const Dwarf::CompilationUnit &cu : *dbg) {
            cu.get_die()->traverse_headless(Insight::build_types, &ctx);
        }

        for (const Dwarf::CompilationUnit &cu : *dbg) {
            cu.get_die()->traverse(Insight::build_post_metadata, &ctx);
        }

    }
}


