#include "inference.hh"

namespace Insight {

    Result TypeInferer::operator()(Dwarf::TaggedDie<DW_TAG_variable>& die) {
        const char *name = die.get_name();
        if (!name)
            return Result::SKIP;

        if (std::string(name) != "insight_typeof_dummy")
            return Result::SKIP;

        auto type = tb.get_type_attr(die);
        if (!type)
            return Result::SKIP;

        std::unique_ptr<const Dwarf::Attribute> locattr = die.get_attribute(DW_AT_location);
        if (!locattr)
            return Result::SKIP;

        size_t loc = locattr->as<Dwarf::Off>();

        auto inferred_type = std::dynamic_pointer_cast<PointerTypeInfoImpl>(type);
        inferred_type_registry[loc] = inferred_type->type_.lock();

        return Result::SKIP;
    }

    Result TypeInferer::operator()([[gnu::unused]] Dwarf::TaggedDie<DW_TAG_lexical_block>& die) {
        return Result::TRAVERSE;
    }

    TypeInferer::TypeInferer(TypeBuilder& tb) : tb(tb) {}

}
