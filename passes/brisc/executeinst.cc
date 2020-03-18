#include "kernel/yosys.h"
#include "passes/brisc/brisc_common.hpp"
#include "passes/brisc/json.hpp"
#include "kernel/consteval.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN
///////////////////////////////////////////////////////////////////////////////
/// @class ExecuteInstPass
/// @brief Execute an Instruction
///////////////////////////////////////////////////////////////////////////////
struct ExecuteInstPass: public Pass {
    ExecuteInstPass() : Pass("execute_inst") {}

    void execute(std::vector<std::string> argv, Design* design) YS_OVERRIDE {
        if (argv.size() != 3) {
            log_error("usage: %s <inst>\n", argv[0].c_str());
            log_abort();
        }

        RTLIL::Const port_drivers;
        const nlohmann::json& j = design->brisc.j;
        const std::string& inst = argv[1];

        if (design->brisc.insts.find(argv[1]) == design->brisc.insts.end()) {
            add_inst(design, inst);
        }

        // Evaluate
        ConstEval ce(design->top_module());
        ce.clear();

        Const& cur_state = design->brisc.state_container.front();
        ce.set(*design->brisc.outdff_spec, cur_state);

        ce.set(*design->brisc.inports_spec, design->brisc.insts[inst]);

        SigSpec undef;
        SigSpec all_sigs = *design->brisc.all_sigs;
        assert(ce.eval(all_sigs, undef));

        SigSpec next_state = *design->brisc.indff_spec;
        assert(ce.eval(next_state, undef));

        // Add State to Poset
        Const next_state_const = next_state.as_const();
        if (design->brisc.poset.insert(next_state_const)) {
            design->brisc.state_container.push_back(next_state_const);
            design->brisc.added_state = true;
            log("Added state\n");
        }
        else {
            design->brisc.added_state = false;
        }

        // Remove Toggled Gates
        log("Before execution: %lu untoggled wires\n",
                design->brisc.untoggled_wires.size());
        for (int i = 0; i < all_sigs.bits().size(); ++i) {
            log_assert(design->brisc.all_sigs->bits()[i].wire != nullptr);
            if (all_sigs.bits()[i].data !=
                    design->brisc.initial_values->bits()[i].data) {
                design->brisc.untoggled_wires.erase(
                        design->brisc.all_sigs->bits()[i].wire->name);
            }
        }
        log("After execution: %lu untoggled wires\n",
                design->brisc.untoggled_wires.size());
    }
} ExecuteInstPass;

PRIVATE_NAMESPACE_END
