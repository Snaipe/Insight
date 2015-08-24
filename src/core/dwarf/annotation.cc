#include "annotation.hh"

namespace Insight {

    void process_annotations(BuildContext& ctx) {

        auto annotated = ctx.annotated.begin();
        auto annotation = ctx.annotations.begin();

        for (; annotation != ctx.annotations.end(); ++annotation) {
            // skip until we reach the end or the next annotated element
            for (; annotated != ctx.annotated.end() && annotated->first <= annotation->first; ++annotated);

            if (annotated == ctx.annotated.end())
                break;

            AddAnnotation visitor(annotation->second);
            annotated->second.apply_visitor(visitor);
        }

        ctx.annotated.clear();
        ctx.annotations.clear();
    }

    size_t get_src_location_offset(Dwarf::Die& die) {
        std::unique_ptr<const Dwarf::Attribute> lineattr = die.get_attribute(DW_AT_decl_line);
        std::unique_ptr<const Dwarf::Attribute> fileattr = die.get_attribute(DW_AT_decl_file);

        if (!lineattr || !fileattr)
            return 0;

        return (fileattr->as<Dwarf::Unsigned>() << ((sizeof (size_t) >> 1) * 8)) | lineattr->as<Dwarf::Unsigned>();
    }

}
