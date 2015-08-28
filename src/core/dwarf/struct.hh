#ifndef INSIGHT_STRUCT_HH
# define INSIGHT_STRUCT_HH

# include "type.hh"

namespace Insight {

    struct StructBuilder : public Dwarf::DefaultDieVisitor {

        Result operator()(Dwarf::TaggedDie<DW_TAG_member> &die);
        Result operator()(Dwarf::TaggedDie<DW_TAG_subprogram> &die);
        Result operator()(Dwarf::TaggedDie<DW_TAG_inheritance> &die);

        template <typename T>
        Result operator()([[gnu::unused]] T& t) {
            return Result::SKIP;
        }

        StructBuilder(std::shared_ptr<StructInfoImpl> info, TypeBuilder &tb);

    private:
        std::shared_ptr<StructInfoImpl> info;
        TypeBuilder& tb;
    };

    std::shared_ptr<TypeInfo> build_struct_type(Dwarf::Die &die, TypeBuilder& tb, bool register_parent);

}

#endif /* !INSIGHT_STRUCT_HH */
