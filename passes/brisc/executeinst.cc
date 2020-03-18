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
    struct Args {
        bool setstate;
        bool overwrite;
        std::string inst;
    } Args;

    void clear_flags() YS_OVERRIDE {
        Args.setstate = false;
        Args.inst = "";
    }

    void parse_input(std::vector<std::string>& argv)
    {
        clear_flags();
        enum {
            e_none = 0,
            e_inst = 1
        } position = e_none;
        for (auto it = argv.begin() + 1; it != argv.end(); ++it) {
            if (it->empty()) {
                log_error("Empty string in command line argument??? WTF???\n");
            }
            if (it->at(0) == '-') {
                if (position != e_none) {
                    help();
                    log_abort();
                }
                /// Parse Options
                for (auto stit = it->begin() + 1; stit != it->end(); ++stit) {
                    switch (*stit) {
                        case 's':
                            Args.setstate = true;
                            break;
                        case 'v':
                            Args.overwrite = true;
                            break;
                        default:
                            log_error("Unknown option %c\n", *stit);
                            help();
                            log_abort();
                    }
                }
            }
            else {
                switch (position) {
                    case e_none:
                        position = e_inst;
                        Args.inst = *it;
                        break;
                    case e_inst:
                        help();
                        log_abort();
                        break;
                    default:
                        log_error("Control flow error.  Should not reach\n");
                        log_abort();
                }
            }
        }
    }

    void help() YS_OVERRIDE
    {
        log("\n");
        log("    execute_inst [-sv] <inst>\n");
        log("\n");
        log("This pass simulates execution of instruction <inst>.\n");
        log("\n");
        log("    -s\n");
        log("        enables pushing state onto design->brisc.ce\n");
        log("    -v\n");
        log("        enables overwriting previously driven signals.\n");
        log("        Used for NOP flush.\n");
        log("\n");
    }

    void execute(std::vector<std::string> argv, Design* design) YS_OVERRIDE {

        parse_input(argv);
        const nlohmann::json& j = design->brisc.j;
        const std::string& inst = Args.inst;

        // Evaluate
        ConstEval& ce = *design->brisc.ce;

        // State is set in executeallinst
        // Const& cur_state = design->brisc.state_container.front();
        // ce.set(*design->brisc.outdff_spec, cur_state);

        log("%s %d\n", __FILE__, __LINE__);
        log("inports_spec.size: %d, insts[%s].size: %d\n",
                design->brisc.inports_spec->size(),
                inst.c_str(),
                design->brisc.insts[inst].size());
        log("inst as const: %s\n",
                design->brisc.insts[inst].as_string().c_str());
        for (SigBit bit : design->brisc.inports_spec->bits()) {
            if (bit.wire == nullptr) {
                log_error("Found input port which is not a wire!\n");
                log_abort();
            }
        }

        SigSpec inports = *(design->brisc.inports_spec);
        log("%s %d\n", __FILE__, __LINE__);
        ce.set(inports, design->brisc.insts[inst],
                Args.overwrite);
        log("%s %d\n", __FILE__, __LINE__);

        /// TODO set internal architectural registers 

        SigSpec undef;
        SigSpec all_sigs = *design->brisc.all_sigs;
        assert(ce.eval(all_sigs, undef));

        SigSpec next_state = *design->brisc.indff_spec;
        assert(ce.eval(next_state, undef));

        // Add State to Poset, or set state in ce for NOPFlush
        Const next_state_const = next_state.as_const();

        if (Args.setstate && design->brisc.poset.insert(next_state_const)) {
            log("Added state\n");
        }
        else {
            log("%s %d\n", __FILE__, __LINE__);
            ce.set(*design->brisc.outdff_spec, next_state_const, true);
            log("%s %d\n", __FILE__, __LINE__);
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
