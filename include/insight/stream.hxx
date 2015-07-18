#ifndef INSIGHT_STREAM_HXX
# define INSIGHT_STREAM_HXX

# include <ostream>
# include "types"

namespace Insight {
    inline std::ostream& operator<<(std::ostream& out, Named& named) {
        out << named.fullname();
        return out;
    }
}

#endif /* !INSIGHT_STREAM_HXX */
