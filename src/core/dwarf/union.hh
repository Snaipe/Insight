#ifndef INSIGHT_UNION_HH
# define INSIGHT_UNION_HH

# include "type.hh"

namespace Insight {

    struct UnionBuilder : public Dwarf::DefaultDieVisitor {

        Result operator()(Dwarf::TaggedDie<DW_TAG_member> &die);
        Result operator()(Dwarf::TaggedDie<DW_TAG_subprogram> &die);

        template <typename T>
        Result operator()([[gnu::unused]] T& t) {
            return Result::SKIP;
        }

        UnionBuilder(std::shared_ptr<UnionInfoImpl> info, TypeBuilder &tb);

    private:
        std::shared_ptr<UnionInfoImpl> info;
        TypeBuilder& tb;

    };

    std::shared_ptr<TypeInfo> build_union_type(Dwarf::Die &die, TypeBuilder& tb, bool register_parent);

}

#endif /* !INSIGHT_UNION_HH */
