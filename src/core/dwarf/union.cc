#include "union.hh"
#include "annotation.hh"

namespace Insight {

    static Dwarf::Die::TraversalResult handle_union_member(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        std::shared_ptr<UnionInfoImpl> info = boost::get<std::shared_ptr<UnionInfoImpl>>(ctx.container_stack.top());
        switch (tag.get_id()) {
            case DW_TAG_member: {
                auto type = get_type_attr(ctx, die);
                if (!type)
                    break;

                std::shared_ptr<UnionFieldInfoImpl> finfo = std::make_shared<UnionFieldInfoImpl>(die.get_name(), type, info);
                info->add_field(finfo);

                mark_element_line(ctx, die, finfo);
            } break;
            case DW_TAG_subprogram: {
                if (!die.get_name()) // ignore unnamed methods
                    break;

                auto return_type = get_type_attr(ctx, die);
                if (!return_type)
                    return_type = VOID_TYPE;

                std::shared_ptr<UnionMethodInfoImpl> method = std::make_shared<UnionMethodInfoImpl>(die.get_name(), return_type, info);

                Dwarf::Off off = die.get_offset();
                ctx.methods.insert(std::make_pair(off, AnyMethod(method)));

                info->add_method(method);

                mark_element_line(ctx, die, method);
            } break;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    std::shared_ptr<TypeInfo> build_union_type(Dwarf::Die &die, BuildContext& ctx, bool register_parent) {
        std::unique_ptr<const Dwarf::Attribute> attrsize = die.get_attribute(DW_AT_byte_size);
        size_t size = !attrsize ? 0 : attrsize->as<Dwarf::Off>();

        std::shared_ptr<Container> parent = get_parent(ctx);

        std::string name = die.get_name() ?: ("anonymous#" + std::to_string(ctx.anonymous_count++));
        std::shared_ptr<UnionInfoImpl> info = std::make_shared<UnionInfoImpl>(name, size);
        ctx.types[die.get_offset()] = info;

        if (register_parent)
            info->set_parent(parent);

        ctx.container_stack.push(AnyContainer(info));
        die.traverse_headless(handle_union_member, &ctx);
        ctx.container_stack.pop();

        mark_element_line(ctx, die, info);

        return info;
    }

}
