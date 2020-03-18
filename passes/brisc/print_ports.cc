#include "kernel/yosys.h"
#include "kernel/sigtools.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct PrintPortsPass : public Pass {
    PrintPortsPass() : Pass("print_ports") {}

    void execute(std::vector<std::string>, Design* design) YS_OVERRIDE {
        for (auto port : design->brisc.input_ports) {
            log("%s\n", port.c_str());
        }
    }

} PrintPortsPass;

PRIVATE_NAMESPACE_END
