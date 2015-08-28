#ifndef INSIGHT_TYPE_HH
# define INSIGHT_TYPE_HH

# include "dwarf.hh"

namespace Insight {

    using Result = Dwarf::Die::TraversalResult;
    using Type = typename std::shared_ptr<TypeInfo>;

    struct TypeBuilder  {

        struct Visitor : public boost::static_visitor<std::shared_ptr<TypeInfo>> {

            Type operator()(Dwarf::TaggedDie<DW_TAG_base_type> &die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_unspecified_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_class_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_structure_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_union_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_enumeration_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_pointer_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_const_type>& die);
            Type operator()(Dwarf::TaggedDie<DW_TAG_typedef>& die);

            template <typename T>
            Type operator()([[gnu::unused]] T& die) {
                return nullptr;
            }

            Visitor(TypeBuilder& tb, bool rp, BuildContext& c);

            TypeBuilder& tb;
            bool register_parent;
            BuildContext& ctx;
        };

        std::shared_ptr<TypeInfo> build_type(Dwarf::AnyDie& anydie, bool register_parent);
        std::shared_ptr<TypeInfo> get_type(Dwarf::AnyDie& die);
        std::shared_ptr<TypeInfo> get_type(Dwarf::Off offset);
        std::shared_ptr<TypeInfo> get_type_attr(Dwarf::Die &die);

        TypeBuilder(BuildContext& ctx);

        BuildContext& ctx;
    };

}

# include "struct.hh"
# include "enum.hh"
# include "union.hh"

#endif /* !INSIGHT_TYPE_HH */
