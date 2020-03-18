#include "kernel/yosys.h"

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

struct BriscPass : public ScriptPass
{
    BriscPass ()
        : ScriptPass("brisc", "Do a BRISC synthesis") {};

    std::string verilog_path, lang_path;
    void help() YS_OVERRIDE
    {
        log("\n");
        log("    brisc <verilog-file> <input-language-file>\n");
        help_script();
        log("\n");
    }

    void clear_flags() YS_OVERRIDE
    {
        verilog_path.clear();
        lang_path.clear();
    }

    void
    execute(std::vector<std::string> argv, RTLIL::Design *design) YS_OVERRIDE
    {
        clear_flags();
        if (3 != argv.size()) {
            log_error("Usage: %s <verilog-file> <input-lang-file>\n",
                      argv[0].c_str());
            exit(1);
        }
        verilog_path = argv[1];
        lang_path = argv[2];

        log_header(design, "Executing BRISC on %s with %s.\n",
                   verilog_path.c_str(), lang_path.c_str());
        log_push();

        run_script(design);
        design->scratchpad_set_string("brisc-lang-path", lang_path);

        log_pop();
    }

    void script() YS_OVERRIDE
    {
        run(stringf("auto_synth %s", verilog_path.c_str()));
        run("brisc_check_inputs");
    }

} BriscPass;

PRIVATE_NAMESPACE_END

