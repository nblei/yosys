#include "kernel/yosys.h"
#include "kernel/rtlil.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct GetAndDumpPass : public ScriptPass
{
    GetAndDumpPass()
        : ScriptPass("getanddump", "get and dump sources and sinks") {}

    void help() YS_OVERRIDE
    {
        log("\n");
        log("Help ...\n");
    }

    void execute(std::vector<std::string>, RTLIL::Design *design) YS_OVERRIDE
    {
        log_header(design, "Executing GetAndDump pass.\n");
        log_push();

        run_script(design);
    }

    void script() YS_OVERRIDE
    {
        run("gss");
        run("dump_sources");
    }

} GetAndDumpPass;

PRIVATE_NAMESPACE_END
