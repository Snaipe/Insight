#ifndef INSIGHT_COMPARE_HXX
# define INSIGHT_COMPARE_HXX

namespace Insight {

    inline bool operator==(const Container& lhs, const Container& rhs) {
        if (&lhs == &rhs)
            return true;

        if (const NamespaceInfo* o = dynamic_cast<const NamespaceInfo*>(&lhs)) {
            return *o == rhs;
        } else if (const StructInfo* o = dynamic_cast<const StructInfo*>(&lhs)) {
            return *o == rhs;
        }
        return false;
    }

    inline bool operator!=(const Container& lhs, const Container& rhs) {
        return !(lhs == rhs);
    }

    inline bool operator==(const TypeInfo& lhs, const TypeInfo& rhs) {
        if (&lhs == &rhs)
            return true;

        return lhs.name() == rhs.name() && lhs.parent() == rhs.parent();
    }

    inline bool operator!=(const TypeInfo& lhs, const TypeInfo& rhs) {
        return !(lhs == rhs);
    }

    inline bool operator==(const NamespaceInfo& lhs, const Container& rhs) {
        if (&lhs == &rhs)
            return true;

        if (const NamespaceInfo* o = dynamic_cast<const NamespaceInfo*>(&rhs)) {
            // stop recursing on the root namespace
            if (lhs.name().length() == 0)
                return o->name().length() == 0;
            return lhs.name() == o->name() && lhs.parent() == o->parent();
        }
        return false;
    }

    inline bool operator==(const StructInfo& lhs, const Container& rhs) {
        if (&lhs == &rhs)
            return true;

        if (const TypeInfo* o = dynamic_cast<const TypeInfo*>(&rhs))
            return lhs == *o;
        return false;
    }
}

#endif /* !INSIGHT_COMPARE_HXX */
