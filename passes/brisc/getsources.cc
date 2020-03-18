#include "kernel/yosys.h"
#include "kernel/sigtools.h"
#include <cassert>

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
/// @class GetSourcesAndSinksPass
///////////////////////////////////////////////////////////////////////////////
struct GetSourcesAndSinksPass : public Pass {
    GetSourcesAndSinksPass() : Pass("gss") {}

    void getffs(Design* design);
    void getports(Design* design);
    SigMap sigmap;
    Module* mod;
    std::set<int> visited;

    void execute(std::vector<std::string>, Design* design) override {
        log("Getting Sources & Sinks\n");
        log_assert(design->modules().size() == 1);
        mod = design->top_module();
        sigmap = SigMap(mod);
        getports(design);
        getffs(design);
    }
} GetSourcesAndSinksPass;

void GetSourcesAndSinksPass::getffs(Design* design)
{
    design->selection_vars[ID("BRISC_DFF_OUTPUT")] =
        RTLIL::Selection(false);
    design->selection_vars[ID("BRISC_DFF_INPUT")] =
        RTLIL::Selection(false);
    RTLIL::Selection& dff_output_sel =
        design->selection_vars[ID("BRISC_DFF_OUTPUT")];
    RTLIL::Selection& dff_input_sel =
        design->selection_vars[ID("BRISC_DFF_INPUT")];

    dff_input_sel.selected_modules.insert(mod->name);
    dff_input_sel.selected_members[mod->name] = pool<RTLIL::IdString>();

    dff_output_sel.selected_modules.insert(mod->name);
    dff_output_sel.selected_members[mod->name] = pool<RTLIL::IdString>();

    unsigned ffcount = 0;
    std::set<RTLIL::IdString> ff_celltypes;
    for (RTLIL::Cell* cell : mod->cells()) {
        const char* s = strstr(cell->type.c_str(), "FF");
        log("cell: %s, type: %s\n", cell->name.c_str(), cell->type.c_str());
        if (s == NULL)
            { continue; }
        ++ffcount;
        ff_celltypes.insert(cell->type);
        for (auto it : cell->connections()) {
            log("\t%s -> %d\n", it.first.c_str(), it.second.size());
        }

        if (!cell->hasPort(ID(D))) {
            log_error("Flip-Flop missing '\\D' port\n");
            exit(1);
        }
        if (!cell->hasPort(ID(Q))) {
            log_error("Flip-Flop missing '\\Q' port\n");
            exit(1);
        }

        RTLIL::SigSpec spec = sigmap(cell->getPort("\\D"));
        if (visited.find(spec.get_hash()) == visited.end()) {
            visited.insert(spec.get_hash());
            assert(spec.size() == 1);
            RTLIL::SigBit bit = spec[0];
            dff_input_sel.selected_members[mod->name].insert(bit.wire->name);
            log("FlipFlop Input: %s\n", bit.wire->name.c_str());
        }

        spec = sigmap(cell->getPort("\\Q"));
        if (visited.find(spec.get_hash()) == visited.end()) {
            visited.insert(spec.get_hash());
            assert(spec.size() == 1);
            RTLIL::SigBit bit = spec[0];
            dff_output_sel.selected_members[mod->name].insert(bit.wire->name);
            log("FlipFlop Output: %s\n", bit.wire->name.c_str());
        }
    }
    log("Found %u flipflops", ffcount);
}

void GetSourcesAndSinksPass::getports(Design* design)
{
    design->selection_vars[ID("BRISC_PORT_INPUT")] =
        RTLIL::Selection(false);
    design->selection_vars[ID("BRISC_PORT_OUTPUT")] =
        RTLIL::Selection(false);
    RTLIL::Selection& input_sel =
        design->selection_vars[ID("BRISC_PORT_INPUT")];
    RTLIL::Selection& output_sel =
        design->selection_vars[ID("BRISC_PORT_OUTPUT")];

    input_sel.selected_modules.insert(mod->name);
    input_sel.selected_members[mod->name] = pool<RTLIL::IdString>();

    output_sel.selected_modules.insert(mod->name);
    output_sel.selected_members[mod->name] = pool<RTLIL::IdString>();

    for (RTLIL::Wire* wire : mod->wires()) {
        if (wire->port_id == 0)
            { continue; }
        if (wire->port_input && wire->port_output) {
            log_error(
                    "Wire %s is type INOUT.  No support for INOUT ports",
                     wire->name.c_str());
            exit(1);
        }

        // Wire is an input port
        RTLIL::SigSpec spec = sigmap(wire);
        if (visited.find(spec.get_hash()) != visited.end())
            { continue; }
        // TODO: Visited with SigSpec or with Wire???
        visited.insert(spec.get_hash());
        if (wire->port_input) {
            log("Input: %s\n", wire->name.c_str());
            input_sel.selected_members[mod->name].insert(wire->name);
        }
        else {
            log("Output: %s\n", wire->name.c_str());
            output_sel.selected_members[mod->name].insert(wire->name);
        }
    }
}

PRIVATE_NAMESPACE_END
