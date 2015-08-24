#include "struct.hh"
#include "annotation.hh"

namespace Insight {

    static Dwarf::Die::TraversalResult handle_member(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        std::shared_ptr<StructInfoImpl> info = boost::get<std::shared_ptr<StructInfoImpl>>(ctx.container_stack.top());
        switch (tag.get_id()) {
            case DW_TAG_member: {
                std::string name(die.get_name() ?: "");

                auto type = get_type_attr(ctx, die);
                if (!type)
                    break;

                std::string prefix("insight_annotation");
                if (name.substr(0, prefix.size()) == prefix) {
                    std::unique_ptr<const Dwarf::Attribute> locattr = die.get_attribute(DW_AT_location);
                    std::unique_ptr<const Dwarf::Attribute> constattr = die.get_attribute(DW_AT_const_value);

                    std::shared_ptr<AnnotationInfoImpl> annotation;

                    std::shared_ptr<ConstTypeInfo> constType = std::dynamic_pointer_cast<ConstTypeInfo>(type);
                    std::string annotationName = constType ? constType->type().name() : type->name();

                    if (locattr) {
                        size_t loc = locattr->as<Dwarf::Off>();
                        void *addr = reinterpret_cast<void*>(loc);

                        annotation = std::make_shared<AnnotationInfoImpl>(annotationName, addr, type);
                    } else if (constattr) {
                        Dwarf::Block* block = constattr->as<Dwarf::Block*>();

                        // we leak the block because we need it alive until the program ends
                        void *addr = std::malloc(block->bl_len);
                        std::memcpy(addr, block->bl_data, block->bl_len);
                        std::shared_ptr<const Dwarf::Debug> dbg = die.get_debug();
                        if (dbg)
                            dbg->dealloc(block);

                        annotation = std::make_shared<AnnotationInfoImpl>(annotationName, addr, type);
                    }

                    if (!annotation)
                        break;

                    size_t off = get_src_location_offset(die);
                    if (off)
                        ctx.annotations[off] = annotation;
                } else {

                    size_t offset = get_offset(die);
                    if (offset == static_cast<size_t>(-1))
                        break;

                    std::shared_ptr<FieldInfoImpl> finfo = std::make_shared<FieldInfoImpl>(die.get_name(), offset, type, info);
                    info->add_field(finfo);

                    mark_element_line(ctx, die, finfo);
                }
            } break;
            case DW_TAG_subprogram: {
                if (!die.get_name()) // ignore unnamed methods
                    break;

                auto return_type = get_type_attr(ctx, die);
                if (!return_type)
                    return_type = VOID_TYPE;

                std::shared_ptr<MethodInfoImpl> method = std::make_shared<MethodInfoImpl>(die.get_name(), return_type, info);

                std::unique_ptr<const Dwarf::Attribute> vattr = die.get_attribute(DW_AT_virtuality);
                std::unique_ptr<const Dwarf::Attribute> vtabattr = die.get_attribute(DW_AT_vtable_elem_location);
                if (vattr && vtabattr) {
                    Dwarf::Signed virtuality = vattr->as<Dwarf::Signed>();
                    if (virtuality != DW_VIRTUALITY_none)
                        method->set_vtable_index(vtabattr->as<uint64_t>());
                }

                Dwarf::Off off = die.get_offset();
                ctx.methods.insert(std::make_pair(off, AnyMethod(method)));

                info->add_method(method);

                mark_element_line(ctx, die, method);
            } break;
            case DW_TAG_inheritance: {
                auto super_type = get_type_attr(ctx, die);
                if (!super_type)
                    break;

                info->add_supertype(std::dynamic_pointer_cast<StructInfo>(super_type));
            } break;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    std::shared_ptr<TypeInfo> build_struct_type(Dwarf::Die &die, BuildContext& ctx, bool register_parent) {

        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        size_t size = !attrsize ? 0 : attrsize->as<Dwarf::Off>();

        std::shared_ptr<Container> parent = get_parent(ctx);

        std::string name = die.get_name() ?: ("anonymous#" + std::to_string(ctx.anonymous_count++));
        std::shared_ptr<StructInfoImpl> info = std::make_shared<StructInfoImpl>(name, size);
        ctx.types[die.get_offset()] = info;

        if (register_parent)
            info->set_parent(parent);

        ctx.container_stack.push(AnyContainer(info));
        die.traverse_headless(handle_member, &ctx);
        ctx.container_stack.pop();

        mark_element_line(ctx, die, info);

        return info;
    }

}
