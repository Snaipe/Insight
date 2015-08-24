#include "inference.hh"
#include "type.hh"

namespace Insight {

    Dwarf::Die::TraversalResult infer_types(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        BuildContext& ctx = *static_cast<BuildContext*>(data);

        switch (tag.get_id()) {
            case DW_TAG_variable: {
                const char *name = die.get_name();
                if (!name)
                    break;

                if (std::string(name) != "insight_typeof_dummy")
                    break;

                auto type = get_type_attr(ctx, die);
                if (!type)
                    break;

                std::unique_ptr<const Dwarf::Attribute> locattr = die.get_attribute(DW_AT_location);
                if (!locattr)
                    break;

                size_t loc = locattr->as<Dwarf::Off>();

                auto inferred_type = std::dynamic_pointer_cast<PointerTypeInfoImpl>(type);
                inferred_type_registry[loc] = inferred_type->type_.lock();
            }
            case DW_TAG_lexical_block: return Dwarf::Die::TraversalResult::TRAVERSE;
            default: break;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

}
