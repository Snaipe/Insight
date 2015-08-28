#ifndef INSIGHT_CORE_CC_H
# define INSIGHT_CORE_CC_H

# include <unordered_map>
# include <memory>
# include <string>
# include "insight/insight"
# include "data/internal.hh"

namespace Insight {

    extern std::shared_ptr<NamespaceInfoImpl> ROOT_NAMESPACE;
    extern std::shared_ptr<TypeInfo> VOID_TYPE;
    extern std::unordered_map<std::string, std::shared_ptr<NamespaceInfo>> namespaces;

    extern std::unordered_map<std::string, std::shared_ptr<TypeInfo>> type_registry;
    extern std::unordered_map<size_t, std::shared_ptr<TypeInfo>> inferred_type_registry;

}

#endif /* !INSIGHT_CORE_CC_H */
