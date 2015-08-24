#ifndef INSIGHT_ENUM_HH
# define INSIGHT_ENUM_HH

# include "type.hh"

namespace Insight {

    std::shared_ptr<TypeInfo> build_enum_type(Dwarf::Die &die, BuildContext& ctx, bool register_parent);

}

#endif /* !INSIGHT_ENUM_HH */
