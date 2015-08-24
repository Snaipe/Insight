#ifndef INSIGHT_ANNOTATION_HH
# define INSIGHT_ANNOTATION_HH

# include "type.hh"

namespace Insight {

    void process_annotations(BuildContext& ctx);
    size_t get_src_location_offset(Dwarf::Die& die);

    struct AddAnnotation : boost::static_visitor<void> {
        AddAnnotation(std::shared_ptr<AnnotationInfoImpl> annotation) : annotation_(annotation) {}

        template <typename T>
        void operator()(std::shared_ptr<T>& it) {
            it->add_annotation(annotation_);
        }

        std::shared_ptr<AnnotationInfoImpl> annotation_;
    };

    template <typename T>
    void mark_element_line(BuildContext& ctx, Dwarf::Die& die, std::shared_ptr<T>& elt) {
        size_t off = get_src_location_offset(die);
        if (off)
            ctx.annotated[off] = AnyAnnotated(elt);
    }
}

#endif /* !INSIGHT_ANNOTATION_HH */
