#include "core.hh"
#include "util/mangle.hh"

namespace Insight {

    std::shared_ptr<NamespaceInfoImpl> ROOT_NAMESPACE = std::make_shared<NamespaceInfoImpl>("");
    std::shared_ptr<TypeInfo> VOID_TYPE = std::make_shared<PrimitiveTypeInfoImpl>("void", 0, PrimitiveKind::VOID, ROOT_NAMESPACE);
    std::unordered_map<std::string, std::shared_ptr<NamespaceInfo>> namespaces;

    std::unordered_map<std::string, std::shared_ptr<TypeInfo>> type_registry;
    std::unordered_map<size_t, std::shared_ptr<TypeInfo>> inferred_type_registry;
    std::vector<std::shared_ptr<Named>> all_objects;

    TypeInfo& type_of_(void *dummy_addr) {
        return *inferred_type_registry.at(reinterpret_cast<size_t>(dummy_addr));
    }

    TypeInfo& type_of_(std::string name) {
        return *type_registry.at(name);
    }

    TypeInfo& type_of_(const std::type_info& info) {
        return type_of_(demangle(std::string(info.name())));
    }

    NamespaceInfo& root_namespace() {
        return *ROOT_NAMESPACE;
    }

    NamespaceInfo& namespace_of_(std::string name) {
        return *namespaces.at(name);
    }

}
