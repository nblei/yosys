#include "kernel/yosys.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct DumpSourcesPass : public Pass {
    DumpSourcesPass() : Pass("dump_sources") {}
    void execute(std::vector<std::string>, Design* design) override {
        if (design->selection_vars.count(ID("BRISC_PORT_INPUT")) == 0) {
            log_error("Must run get_sources pass before dump_sources");
            return;
        }
        if (design->selection_vars.count(ID("BRISC_DFF_OUTPUT")) == 0) {
            log_error("Must run get_sources pass before dump_sources");
            return;
        }

        RTLIL::Selection sel = design->selection_vars[ID("BRISC_PORT_INPUT")];
        for (auto &mod : sel.selected_modules) {
            RTLIL::Module* active_mod = design->modules_[mod];
            for (auto & memb : sel.selected_members[mod]) {
                log("Port Member: %s\n", memb.c_str());
                log_wire(active_mod->wire(memb));
            }
        }

        sel = design->selection_vars[ID("BRISC_DFF_OUTPUT")];
        for (auto &mod : sel.selected_modules) {
            RTLIL::Module* active_mod = design->modules_[mod];
            for (auto & memb : sel.selected_members[mod]) {
                log("DFF Output Member: %s\n", memb.c_str());
                log_wire(active_mod->wire(memb));
            }
        }

    }
} DumpSourcesPass;

PRIVATE_NAMESPACE_END
