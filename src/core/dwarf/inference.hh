#ifndef INSIGHT_INFERENCE_HH
# define INSIGHT_INFERENCE_HH

# include "dwarf.hh"

namespace Insight {

    Dwarf::Die::TraversalResult infer_types(Dwarf::Die &die, void *data);

}

#endif /* !INSIGHT_INFERENCE_HH */
