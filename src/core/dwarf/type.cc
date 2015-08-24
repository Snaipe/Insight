#include "type.hh"
#include "struct.hh"

namespace Insight {

    std::shared_ptr<TypeInfo> build_type(Dwarf::Die& die, BuildContext& ctx, bool register_parent) {
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

        std::shared_ptr<Container> parent = get_parent(ctx);

        std::shared_ptr<TypeInfo> type;
        std::string name = die.get_name() ?: "";

        const auto it = ctx.types.find(die.get_offset());
        if (it != ctx.types.end()) {
            type = it->second;
            if (register_parent) {
                std::shared_ptr<MutableChild> child = std::dynamic_pointer_cast<MutableChild>(type);
                child->set_parent(parent);
            }
        } else {
            const Dwarf::Tag &tag = die.get_tag();
            switch (tag.get_id()) {
                case DW_TAG_array_type: {

                } break;
                case DW_TAG_base_type: {
                    PrimitiveKind kind = primitiveKinds[die.get_name()];
                    std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
                    if (!attrsize) // ignore types with no size
                        break;

                    size_t size = attrsize->as<Dwarf::Off>();
                    auto t = std::make_shared<PrimitiveTypeInfoImpl>(die.get_name(), size, kind);

                    if (register_parent)
                        t->set_parent(parent);

                    ctx.types[die.get_offset()] = t;
                    type = t;
                } break;
                case DW_TAG_unspecified_type: {
                    auto t = std::make_shared<UnspecifiedTypeInfoImpl>(die.get_name());
                    ctx.types[die.get_offset()] = t;

                    // C++11 requires sizeof(nullptr_t) == sizeof(void*)
                    if (name == "decltype(nullptr)")
                        t->size_ = sizeof(void*);

                    if (register_parent)
                        t->set_parent(parent);

                    type = t;
                } break;
                case DW_TAG_class_type: {
                    type = build_struct_type(die, ctx, register_parent);
                } break;
                case DW_TAG_structure_type: {
                    type = build_struct_type(die, ctx, register_parent);
                } break;
                case DW_TAG_union_type: {
                    type = build_union_type(die, ctx, register_parent);
                } break;
                case DW_TAG_enumeration_type: {
                    type = build_enum_type(die, ctx, register_parent);
                } break;
                case DW_TAG_pointer_type: {
                    std::shared_ptr<PointerTypeInfoImpl> t = std::make_shared<PointerTypeInfoImpl>();
                    ctx.types[die.get_offset()] = t;

                    std::unique_ptr<const Dwarf::Attribute> attrtype = die.get_attribute(DW_AT_type);
                    std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
                    if (!attrsize)
                        break;
                    if (!attrtype) {
                        t->set_type(VOID_TYPE);
                    } else {
                        std::shared_ptr<TypeInfo> subtype = get_type(ctx, attrtype->as<Dwarf::Off>());
                        if (!subtype)
                            break;

                        t->set_type(subtype);
                    }

                    if (register_parent)
                        t->set_parent(parent);

                    t->size_ = attrsize->as<Dwarf::Unsigned>();
                    type = t;
                } break;
                case DW_TAG_const_type: {
                    std::shared_ptr<ConstTypeInfoImpl> t = std::make_shared<ConstTypeInfoImpl>();
                    ctx.types[die.get_offset()] = t;

                    auto subtype = get_type_attr(ctx, die);
                    if (!subtype)
                        break;

                    if (register_parent)
                        t->set_parent(parent);

                    t->set_type(subtype);
                    type = t;
                } break;
                case DW_TAG_typedef: {
                    const char *name = die.get_name();
                    if (!name)
                        break;

                    std::shared_ptr<TypeDefInfoImpl> t = std::make_shared<TypeDefInfoImpl>(name);
                    ctx.types[die.get_offset()] = t;

                    auto subtype = get_type_attr(ctx, die);
                    if (!subtype)
                        break;

                    if (register_parent)
                        t->set_parent(parent);

                    t->set_type(subtype);
                    type = t;
                } break;
                default: break;
            }
            if (type && register_parent)
                add_type_to_parent(ctx, type);
        }
        return type;
    }

    std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Die& die) {
        auto it = ctx.types.find(die.get_offset());
        if (it == ctx.types.end()) {
            return build_type(die, ctx, false);
        }
        return it->second;
    }

    std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Off offset) {
        auto it = ctx.types.find(offset);
        if (it == ctx.types.end()) {
            std::shared_ptr<Dwarf::Die> die = ctx.dbg.offdie(offset);
            return die ? build_type(*die, ctx, false) : nullptr;
        }
        return it->second;
    }

    std::shared_ptr<TypeInfo> get_type_attr(BuildContext &ctx, Dwarf::Die &die) {
        std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_type);
        if (!attr)
            return nullptr;

        auto target = attr->as_die();
        return target ? get_type(ctx, *target) : nullptr;
    }

}
