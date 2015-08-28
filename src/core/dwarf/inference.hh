#ifndef INSIGHT_INFERENCE_HH
# define INSIGHT_INFERENCE_HH

# include "dwarf.hh"
# include "type.hh"

namespace Insight {

    struct TypeInferer : public Dwarf::DefaultDieVisitor {

        Result operator()(Dwarf::TaggedDie<DW_TAG_variable>& die);
        Result operator()(Dwarf::TaggedDie<DW_TAG_lexical_block>& die);

        template <typename T>
        Result operator()([[gnu::unused]] T& die) {
            return Result::SKIP;
        }

        TypeInferer(TypeBuilder& tb);

    private:
        TypeBuilder& tb;
    };

}

#endif /* !INSIGHT_INFERENCE_HH */
