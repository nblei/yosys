#include "kernel/yosys.h"
#include "passes/brisc/brisc_common.hpp"
#include "passes/brisc/json.hpp"
#include "kernel/consteval.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN
///////////////////////////////////////////////////////////////////////////
/// @class ResetPass
/// @brief Reset the design
///////////////////////////////////////////////////////////////////////////
struct ResetPass: public Pass {
    ResetPass() : Pass("reset") {}

    void execute(std::vector<std::string> argv, Design* design) YS_OVERRIDE {
        RTLIL::Const port_drivers;
        const nlohmann::json& j = design->brisc.j;
        const std::string& inst = "reset";

        if (design->brisc.insts.find(inst) == design->brisc.insts.end()) {
            add_inst(design, inst);
        }

        ConstEval ce(design->top_module());
        ce.clear();

        log_assert(nullptr != design->brisc.inports_spec);
        log_assert(nullptr != design->brisc.outdff_spec);
        RTLIL::SigSpec ports_spec = *(design->brisc.inports_spec);
        RTLIL::SigSpec& dff_spec = *(design->brisc.outdff_spec);
        RTLIL::SigSpec undef;
        Const& iconst = design->brisc.insts[inst];
        log("ports_spec: %d, iconst: %d\n", ports_spec.size(), iconst.size());

        ce.set(ports_spec, iconst);

        ce.set(dff_spec,
               RTLIL::Const(std::vector<State>(dff_spec.size(), State::Sx)));

        for (unsigned i = 0; i < design->brisc.pipeline_depth; ++i) {
            SigSpec _dff = dff_spec;
            log_assert(ce.eval(_dff, undef));
            Const c;
            ce.set(dff_spec, _dff.as_const());
        }

        RTLIL::SigSpec all_sigs;
        for (Wire* wire : design->top_module()->wires()) {
            all_sigs.append(RTLIL::SigSpec(wire));
        }

        RTLIL::SigSpec eval = all_sigs;

        while (!ce.eval(eval, undef)) {
            ce.set(undef,
                    RTLIL::Const(std::vector<State>(undef.size(), State::Sx)));
            eval = all_sigs;
        }

        log_assert(design->brisc.all_sigs == nullptr);
        design->brisc.all_sigs = new SigSpec(all_sigs);
        log_assert(design->brisc.initial_values == nullptr);
        design->brisc.initial_values = new SigSpec(eval);

        generate_initial_state(&design->brisc);
        generate_untoggled_set(&design->brisc);
    }

    void generate_initial_state(RTLIL::BRISC* brisc)
    {
        log_assert(brisc->initial_values != nullptr);
        
       // If a DFF output, add to SigSpec
        /// TODO mapping between dff_inports and dff_outports
        SigSpec reset_state;
        const std::set<IdString>& input_dff = brisc->input_dff;
        for (const SigBit& sb : brisc->initial_values->bits()) {
            if (input_dff.find(sb.wire->name) == input_dff.end()) {
                continue;
            }
            reset_state.append(sb);
        }
        // brisc->state_container.push_back(reset_state.as_const());
        log("reset_state size: %d, as_const: %d\n", reset_state.size(),
                reset_state.as_const().size());
        log("output_ports.size(): %lu\n", brisc->output_ports.size());
        log_assert(reset_state.size() == brisc->outdff_spec->size());
        Const reset_state_const = reset_state.as_const();
        log_assert(brisc->poset.insert(reset_state_const));
    }

    void generate_untoggled_set(RTLIL::BRISC* brisc)
    {
        for (Wire* wire : brisc->design->top_module()->wires()) {
            brisc->untoggled_wires.insert(wire->name);
        }
    }

    void log_eval(const RTLIL::SigSpec& all_sigs, const RTLIL::SigSpec& eval)
    {
        int offset = 0;
        for (int i = 0; i < all_sigs.size(); ++i) {
            log_wire(all_sigs[i].wire);
            for (int j = 0; j < all_sigs[i].wire->width; ++j) {
                char c;
                switch (eval.bits()[offset+j].data) {
                    case State::S0:
                        c = '0';
                        break;
                    case State::S1:
                        c = '1';
                        break;
                    default:
                        c = 'x';
                        break;
                }
                log("%c", c);
            }
            std::cout << std::endl;
            offset += all_sigs[i].wire->width;
        }
    }


} ExecuteInstPass;

PRIVATE_NAMESPACE_END

