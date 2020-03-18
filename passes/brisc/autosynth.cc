#include "kernel/yosys.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct AutoSynthPass : public ScriptPass
{
    AutoSynthPass()
        : ScriptPass("auto_synth", "Synthesize verilog to logical gates") {}

    std::string verilog_path;

    void help() YS_OVERRIDE
    {
        log("\n");
        log("    auto_synth <verilog-file>\n");
        help_script();
        log("\n");
    }

    void clear_flags() YS_OVERRIDE
    {
        verilog_path.clear();
    }

    void execute(std::vector<std::string> argv,
                 RTLIL::Design *design) YS_OVERRIDE
    {
        clear_flags();
        if (2 != argv.size()) {
            log_error("Usage: %s <verilog-file>\n", argv[0].c_str());
            log_abort();
            exit(1);
        }
        verilog_path = argv[1];

        log_header(design, "Executing AUTO_SYNTH pass.\n");
        log_push();

        run_script(design);

        log_pop();

    }

    void script() YS_OVERRIDE
    {
        run(stringf("read_verilog %s", verilog_path.c_str()));
        run("proc");
        run("opt");
        run("fsm");
        run("opt");
        run("memory");
        run("opt");
        run("flatten");
        run("opt");
        run("techmap");
        run("opt");
        run("flatten");
        run("opt");
        run("clean");
    }

} AutoSynthPass;

PRIVATE_NAMESPACE_END
