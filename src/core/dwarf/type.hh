#ifndef INSIGHT_TYPE_HH
# define INSIGHT_TYPE_HH

# include "dwarf.hh"

namespace Insight {

    std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Off offset);
    std::shared_ptr<TypeInfo> get_type(BuildContext& ctx, Dwarf::Die& die);
    std::shared_ptr<TypeInfo> get_type_attr(BuildContext &ctx, Dwarf::Die &die);
    std::shared_ptr<TypeInfo> build_type(Dwarf::Die& die, BuildContext& ctx, bool register_parent);

}

# include "struct.hh"
# include "enum.hh"
# include "union.hh"

#endif /* !INSIGHT_TYPE_HH */
