#include "kernel/yosys.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct CheckInputsPass : public Pass {
    CheckInputsPass(): Pass("brisc_check_inputs",
            "Ensures inputs to BRISC are appropriate\n") {}

    void check_design(Design* design);

    void execute(std::vector<std::string>, Design* design) YS_OVERRIDE {
        log("Checking Inputs\n");
        check_design(design);
    }
                                  
} CheckInputsPass;

void
CheckInputsPass::check_design(Design* design)
{
    log_assert(design->modules().size() == 1 && "Unimodule design\n");
}

PRIVATE_NAMESPACE_END

