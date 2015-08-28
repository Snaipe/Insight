#ifndef INSIGHT_ENUM_HH
# define INSIGHT_ENUM_HH

# include "type.hh"

namespace Insight {

    struct EnumBuilder : public Dwarf::DefaultDieVisitor {

        Result operator()(Dwarf::TaggedDie<DW_TAG_enumerator> &die);

        template <typename T>
        Result operator()([[gnu::unused]] T& t) {
            return Result::SKIP;
        }

        EnumBuilder(std::shared_ptr<EnumInfoImpl> info, TypeBuilder &tb);

    private:
        std::shared_ptr<EnumInfoImpl> info;
        TypeBuilder& tb;
    };

    std::shared_ptr<TypeInfo> build_enum_type(Dwarf::Die &die, TypeBuilder& tb, bool register_parent);

}

#endif /* !INSIGHT_ENUM_HH */
