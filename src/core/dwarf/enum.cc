#include "enum.hh"
#include "annotation.hh"

namespace Insight {

    static Dwarf::Die::TraversalResult handle_enum_member(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();

        std::shared_ptr<EnumInfoImpl> info = *static_cast<std::shared_ptr<EnumInfoImpl>*>(data);
        switch (tag.get_id()) {
            case DW_TAG_enumerator: {
                std::unique_ptr<const Dwarf::Attribute> constattr = die.get_attribute(DW_AT_const_value);
                if (!constattr || !die.get_name())
                    break;

                void* addr;
                switch (constattr->form()) {
                    case DW_FORM_block1:
                    case DW_FORM_block2:
                    case DW_FORM_block4:
                    case DW_FORM_block: {
                        Dwarf::Block* block = constattr->as<Dwarf::Block*>();

                        // we leak the block because we need it alive until the program ends
                        addr = std::malloc(block->bl_len);
                        std::memcpy(addr, block->bl_data, block->bl_len);
                        std::shared_ptr<const Dwarf::Debug> dbg = die.get_debug();
                        if (dbg)
                            dbg->dealloc(block);
                    } break;
                    default: {
                        Dwarf::Unsigned val = constattr->as<Dwarf::Unsigned>();

                        // we leak the block because we need it alive until the program ends
                        switch (info->size_of()) {
                            case 8: {
                                uint64_t* i = static_cast<uint64_t*>(std::malloc(info->size_of()));
                                *i = val;
                                addr = i;
                            } break;
                            case 4: {
                                uint32_t* i = static_cast<uint32_t*>(std::malloc(info->size_of()));
                                *i = val;
                                addr = i;
                            } break;
                            case 3: {
                                uint16_t* i = static_cast<uint16_t*>(std::malloc(info->size_of()));
                                *i = val;
                                addr = i;
                            } break;
                            case 1: {
                                uint8_t* i = static_cast<uint8_t*>(std::malloc(info->size_of()));
                                *i = val;
                                addr = i;
                            } break;
                            default: break;
                        }
                    } break;
                }

                std::shared_ptr<EnumInfo> iface = info;
                info->add_value(std::make_shared<EnumConstantInfoImpl>(die.get_name(), addr, iface));
            } break;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    std::shared_ptr<TypeInfo> build_enum_type(Dwarf::Die &die, BuildContext& ctx, bool register_parent) {
        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        size_t size = !attrsize ? 0 : attrsize->as<Dwarf::Off>();

        std::shared_ptr<Container> parent = get_parent(ctx);

        std::string name = die.get_name() ?: ("anonymous#" + std::to_string(ctx.anonymous_count++));
        std::shared_ptr<EnumInfoImpl> info = std::make_shared<EnumInfoImpl>(name, size);
        ctx.types[die.get_offset()] = info;

        if (register_parent)
            info->set_parent(parent);

        die.traverse_headless(handle_enum_member, &info);

        mark_element_line(ctx, die, info);

        return info;
    }

}
