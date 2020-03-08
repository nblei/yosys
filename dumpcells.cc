#include "kernel/yosys.h"
#include <cstdio>

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct DumpCellsPass : public Pass {
    DumpCellsPass() : Pass("dump_cells") {}
    void execute(std::vector<std::string>, Design* design) override {
        log("Dumping Cells");
        log_assert(design->modules().size() == 1);
        unsigned ffcount = 0;

        Module* mod = design->top_module();
        for (RTLIL::Cell* cell : mod->cells()) {
            log("Cell: %s, type: %s\n", cell->name.c_str(), cell->type.c_str());
            const char* s = strstr(cell->type.c_str(), "FF");
            if (s == NULL)
                { continue; }
            else
                { ++ffcount; }
        }
        log("Found %u flipflops", ffcount);
    }
} DumpCellsPass;

PRIVATE_NAMESPACE_END
