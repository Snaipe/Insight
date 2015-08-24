#include "core.hh"
#include "util/mangle.hh"

namespace Insight {

    std::shared_ptr<NamespaceInfoImpl> root_ns = std::make_shared<NamespaceInfoImpl>("");
    std::unordered_map<std::string, std::shared_ptr<NamespaceInfo>> namespaces;
    std::unordered_map<std::string, std::shared_ptr<TypeInfo>> type_registry;
    std::unordered_map<size_t, std::shared_ptr<TypeInfo>> inferred_type_registry;

    std::shared_ptr<TypeInfo> VOID_TYPE = std::make_shared<PrimitiveTypeInfoImpl>("void", 0, PrimitiveKind::VOID, root_ns);
    std::unordered_map<std::string, PrimitiveKind> PRIMITIVE_KINDS_FROM_NAME {
            {"char",                    PrimitiveKind::CHAR},
            {"signed char",             PrimitiveKind::CHAR},
            {"unsigned char",           PrimitiveKind::UNSIGNED_CHAR},
            {"int",                     PrimitiveKind::INT},
            {"unsigned int",            PrimitiveKind::UNSIGNED_INT},
            {"short int",               PrimitiveKind::SHORT_INT},
            {"short unsigned int",      PrimitiveKind::UNSIGNED_SHORT_INT},
            {"long int",                PrimitiveKind::LONG_INT},
            {"long unsigned int",       PrimitiveKind::UNSIGNED_LONG_INT},
            {"long long int",           PrimitiveKind::LONG_LONG_INT},
            {"long long unsigned int",  PrimitiveKind::UNSIGNED_LONG_LONG_INT},
            {"float",                   PrimitiveKind::FLOAT},
            {"double",                  PrimitiveKind::DOUBLE},
            {"long double",             PrimitiveKind::LONG_DOUBLE},
            {"bool",                    PrimitiveKind::BOOL},
            {"_Bool",                   PrimitiveKind::BOOL},
            {"complex float",           PrimitiveKind::FLOAT_COMPLEX},
            {"complex double",          PrimitiveKind::DOUBLE_COMPLEX},
            {"complex long double",     PrimitiveKind::LONG_DOUBLE_COMPLEX},
    };

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
        return *root_ns;
    }

    NamespaceInfo& namespace_of_(std::string name) {
        return *namespaces.at(name);
    }
}
