#include "kernel/yosys.h"
#include "kernel/sigtools.h"
#include "passes/brisc/brisc_common.hpp"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
/// @class GetSourcesAndSinksPass
///////////////////////////////////////////////////////////////////////////////
struct GetSourcesAndSinksPass : public Pass {
    GetSourcesAndSinksPass() : Pass("gss") {}

    void getffs(Design* design);
    void getports(Design* design);
    SigSpec* generate_sigspecs(Design*, const std::set<IdString>&);
    SigMap sigmap;
    Module* mod;
    std::set<int> visited;

    void execute(std::vector<std::string>, Design* design) YS_OVERRIDE {
        log("Getting Sources & Sinks\n");
        log_assert(design->modules().size() == 1);
        mod = design->top_module();
        sigmap = SigMap(mod);
        getports(design);
        getffs(design);
        log("%lu output dffs, %lu input dffs\n", design->brisc.output_dff.size(),
                design->brisc.input_dff.size());

        design->brisc.inports_spec = generate_sigspecs(
                design, design->brisc.input_ports);
        design->brisc.outports_spec = generate_sigspecs(
                design, design->brisc.output_ports);
        design->brisc.outdff_spec = generate_sigspecs(
                design, design->brisc.output_dff);
        design->brisc.indff_spec = generate_sigspecs(
                design, design->brisc.input_dff);
        log_assert(nullptr != design->brisc.inports_spec);
        log_assert(nullptr != design->brisc.outports_spec);
        log_assert(nullptr != design->brisc.indff_spec);
        log_assert(nullptr != design->brisc.outdff_spec);
        log("%lu output dffs, %lu input dffs\n", design->brisc.output_dff.size(),
                design->brisc.input_dff.size());
    }
} GetSourcesAndSinksPass;


///////////////////////////////////////////////////////////////////////////////
/// @brief Get flip-flops
/// @output design->brisc.comb_cells
/// @output design->brisc.dff_cells
/// @output design->brisc.output_dff
/// @output design->brisc.input_dff
///////////////////////////////////////////////////////////////////////////////
void GetSourcesAndSinksPass::getffs(Design* design)
{
    unsigned ffcount = 0;
    std::set<RTLIL::IdString> ff_celltypes;

    for (RTLIL::Cell* cell : mod->cells()) {
        const char* s = strstr(cell->type.c_str(), "FF");
        if (s == NULL)
            { 
                design->brisc.comb_cells.push_back(cell);
                continue;
            }
        ++ffcount;
        ff_celltypes.insert(cell->type);

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
            log_assert(spec.size() == 1);
            RTLIL::SigBit bit = spec[0];
            design->brisc.input_dff.insert(bit.wire->name);
            log("FlipFlop Input: %s\n", bit.wire->name.c_str());
        }

        spec = sigmap(cell->getPort("\\Q"));
        if (visited.find(spec.get_hash()) == visited.end()) {
            visited.insert(spec.get_hash());
            log_assert(spec.size() == 1);
            RTLIL::SigBit bit = spec[0];
            design->brisc.output_dff.insert(bit.wire->name);
            log("FlipFlop Output: %s\n", bit.wire->name.c_str());
        }
        design->brisc.dff_cells.push_back(cell);
        Wire* q = cell->getPort(ID(Q)).as_wire();
        Wire* d = cell->getPort(ID(D)).as_wire();
        design->brisc.dff_in_to_out[q] = d;
    }
    log("Found %u flipflops\n", ffcount);
}

RTLIL::SigSpec * GetSourcesAndSinksPass::generate_sigspecs(
        Design* design, const std::set<IdString>& p)
{
    SigSpec* spec = new RTLIL::SigSpec();
    log_assert(spec != nullptr);
    for (auto it = p.begin(); it != p.end(); ++it) {
        (spec)->append(design->top_module()->wire(*it));
    }
    return spec;
}

void GetSourcesAndSinksPass::getports(Design* design)
{
    for (RTLIL::Wire* wire : mod->wires()) {
        if (wire->port_id == 0)
            { continue; }
        if (wire->port_input && wire->port_output) {
            log_error(
                    "Wire %s is type INOUT.  No support for INOUT ports",
                     wire->name.c_str());
            log_abort();
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
            design->brisc.input_ports.insert(wire->name);
        }
        else {
            log("Output: %s\n", wire->name.c_str());
            design->brisc.output_ports.insert(wire->name);
        }
    }

}


PRIVATE_NAMESPACE_END
