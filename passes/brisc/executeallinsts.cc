#include "kernel/yosys.h"
#include "passes/brisc/brisc_common.hpp"
#include "kernel/consteval.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct ExecuteAllInstsPass: public ScriptPass{
    Design* design = nullptr;
    ExecuteAllInstsPass()
        : ScriptPass("execute_all", "Execute all (non-reset) instructions") {}

    void execute(std::vector<std::string>, RTLIL::Design *design) YS_OVERRIDE
    {
        this->design = design;
        log_header(design, "Executing ExecuteAllInsts pass.\n");
        log_push();

        run_script(design);

        log_pop();
    }

    void script() YS_OVERRIDE
    {
        log_assert(design != nullptr);
        log_assert(design->brisc.poset.size() != 0);
        log_assert(design->brisc.ce == nullptr);
        design->brisc.ce = new ConstEval(design->top_module());
        ConstEval& ce = *(design->brisc.ce);
        log_assert(design->brisc.ce != nullptr);

        RTLIL::Const solo;
        bool changed;

        // TODO Move execution context (ConstEval) into BRISC structure
        // TODO do NOP flush on each loaded state
        do {
            if (design->brisc.poset.size() == 1) {
                solo = design->brisc.poset.max_poset_.front();
            }

            // Set Current State
            // design->brisc.state_container.clear();
            // design->brisc.state_container.push_front(solo);
            log("%s %d\n", __FILE__, __LINE__);
            SigSpec outdff = *(design->brisc.outdff_spec);
            ce.set(outdff,
                    design->brisc.poset.max_poset_.front());
            log("%s %d\n", __FILE__, __LINE__);
            ce.push();

            nlohmann::json& j = design->brisc.j;
            for (auto it : j["instructions"].items()) {
                log("visiting inst edge %s\n", it.key().c_str());

                // if (it.key().compare("reset") == 0)
                //     continue;
                // if (it.key().compare("nop") == 0)
                //     continue;

                run("execute_inst -s " + it.key());

                // If this is a 'terminal' state, do a NOP flush
                if (false == design->brisc.added_state) {
                    for (unsigned i = 0;
                         i < design->brisc.pipeline_depth - 1;
                         ++i)
                    {
                        run("execute_inst -v nop");
                    }
                }

                // Go back to current state
                ce.pop();
                ce.push();
            }

            if (design->brisc.poset.size() == 1) {
                changed = solo == design->brisc.poset.max_poset_.front();
            }

        } while ( design->brisc.poset.size() > 1 || (!changed) );
    }

} ExecuteAllInstsPass;

PRIVATE_NAMESPACE_END
