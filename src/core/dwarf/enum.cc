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
#include "enum.hh"
#include "annotation.hh"

namespace Insight {

    Result EnumBuilder::operator()(Dwarf::TaggedDie<DW_TAG_enumerator> &die) {
        std::unique_ptr<const Dwarf::Attribute> constattr = die.get_attribute(DW_AT_const_value);
        if (!constattr || !die.get_name())
            return Result::SKIP;

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

        return Result::SKIP;
    }

    EnumBuilder::EnumBuilder(std::shared_ptr<EnumInfoImpl> info, TypeBuilder &tb)
        : info(info)
        , tb(tb)
    {}

    std::shared_ptr<TypeInfo> build_enum_type(Dwarf::Die &die, TypeBuilder& tb, bool register_parent) {
        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        size_t size = !attrsize ? 0 : attrsize->as<Dwarf::Off>();

        std::shared_ptr<Container> parent = get_parent(tb.ctx);

        std::string name = die.get_name() ?: ("anonymous#" + std::to_string(tb.ctx.anonymous_count++));
        std::shared_ptr<EnumInfoImpl> info = std::make_shared<EnumInfoImpl>(name, size);
        tb.ctx.types[die.get_offset()] = info;

        if (register_parent)
            info->set_parent(parent);

        EnumBuilder builder(info, tb);

        die.visit_headless(builder);

        mark_element_line(tb.ctx, die, info);

        return info;
    }

}
