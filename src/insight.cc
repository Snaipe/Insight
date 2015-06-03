#include <unordered_map>
#include <libdwarf++/dwarf.hh>
#include <libdwarf++/die.hh>
#include <libdwarf++/cu.hh>
#include "internal.hh"

namespace Insight {

    std::unordered_map<std::string, std::unique_ptr<StructInfo>> classes;

    static size_t get_offset(Dwarf::Die &die) {
        std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_data_member_location);
        if (attr) {
            return attr->as<Dwarf::Unsigned>();
        }
        return static_cast<size_t>(-1);
    }

    static Dwarf::Die::TraversalResult handle_member(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();

        StructInfoImpl *structinfo = static_cast<StructInfoImpl*>(data);
        switch (tag.get_id()) {
            case DW_TAG_member: {
                size_t offset = get_offset(die);
                if (offset == static_cast<size_t>(-1))
                    break;
                std::unique_ptr<const Dwarf::Attribute> attr = die.get_attribute(DW_AT_type);
                std::weak_ptr<TypeInfo> weak(std::shared_ptr<TypeInfo>(nullptr));
                std::unique_ptr<FieldInfo> finfo = std::make_unique<FieldInfoImpl>(die.get_name(), offset, weak);
                structinfo->add_field(finfo);
            } break;
            case DW_TAG_subprogram: {

            } break;
            default: return Dwarf::Die::TraversalResult::TRAVERSE;
        }
        return Dwarf::Die::TraversalResult::SKIP;
    }

    static Dwarf::Die::TraversalResult handle_die(Dwarf::Die &die, void *data) {
        const Dwarf::Tag &tag = die.get_tag();
        switch (tag.get_id()) {
            case DW_TAG_base_type: {
            } break;
            case DW_TAG_structure_type:
            case DW_TAG_class_type: {
                if (!die.get_name()) // ignore anonymous structures
                    return Dwarf::Die::TraversalResult::SKIP;

                std::unique_ptr<StructInfo> structinfo = std::make_unique<StructInfoImpl>(die.get_name());
                die.traverse(handle_member, &*structinfo);
                classes[structinfo->name()] = std::move(structinfo);
            } return Dwarf::Die::TraversalResult::SKIP;
            default: break;
        }
        return Dwarf::Die::TraversalResult::TRAVERSE;
    }

    void initialize() {

        std::shared_ptr<Dwarf::Debug> dbg = Dwarf::Debug::self();

        for (const Dwarf::CompilationUnit &cu : *dbg) {
            std::shared_ptr<Dwarf::Die> die = cu.get_die();
            die->traverse(Insight::handle_die, nullptr);
        }

    }
}
