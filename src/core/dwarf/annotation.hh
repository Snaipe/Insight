/*
 * This file is part of Insight.
 *
 * Copyright © 2015 Franklin "Snaipe" Mathieu <http://snaipe.me>
 *
 * Insight is free software: you can redistribute it and/or modify
 * it under the terms of the GNU General Public License as published by
 * the Free Software Foundation, either version 3 of the License, or
 * (at your option) any later version.
 *
 * Insight is distributed in the hope that it will be useful,
 * but WITHOUT ANY WARRANTY; without even the implied warranty of
 * MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 * GNU General Public License for more details.
 *
 * You should have received a copy of the GNU General Public License
 * along with Insight.  If not, see <http://www.gnu.org/licenses/>.
 *
 */
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
