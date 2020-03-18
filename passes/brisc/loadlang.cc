#include "kernel/yosys.h"
#include "passes/brisc/json.hpp"
#include "passes/brisc/brisc_common.hpp"
#include <iostream>
#include <fstream>
#include "passes/brisc/brisc_common.hpp"
#include <regex>
#include <ctype.h>

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////
/// @class LoadLangPass
/// @brief Load and check a 'language file'
///////////////////////////////////////////////////////////////////////////
struct LoadLangPass: public Pass {
    const std::regex bin_regex;
    const std::regex bin_repr_regex;
    const std::regex hex_repr_regex;

    LoadLangPass() :
        Pass("brisc_load_lang"),
        bin_regex(std::regex("[01x_]+", std::regex::icase)),
        bin_repr_regex(std::regex("[1-9][0-9]*'b([01x_]+)", std::regex::icase)),
        hex_repr_regex(std::regex("[1-9][0-9]*'h([01x_]+)", std::regex::icase))
                {}

    void execute(std::vector<std::string> argv, Design* design) YS_OVERRIDE {
        if (argv.size() != 2) {
            log_error("usage: %s <langfile-path>\n", argv[0].c_str());
            log_abort();
        }

        log("%s %d\n", __FILE__, __LINE__);
        nlohmann::json& j = load_json(design, argv[1].c_str());
        get_clock(design, j);
        log("%s %d\n", __FILE__, __LINE__);

        /// Get pool of input ports
        std::set<IdString>& inports = design->brisc.input_ports;
        log("%s %d\n", __FILE__, __LINE__);


        for (auto& inst : j["instructions"].items()) {
            for (auto& port : inst.value()["ports"].items()) {
                port.value() = transform_signal(
                        port.value().get<std::string>().c_str());

                // Check to see if is valid port
                if (inports.find(port.key()) == inports.end()) {
                    log_error("Design missing port '%s'\n",
                              (port.key()).c_str());
                    log_abort();
                }

            }
            std::cout << j["instructions"][inst.key()]["ports"] << std::endl;
            xify_unused_ports(design, inports, inst.key());
        }

        log("%s %d\n", __FILE__, __LINE__);


        log_assert(j["instructions"].contains("nop"));
        log_assert(j["instructions"].contains("reset"));
        log_assert(j["pipeline-depth"].is_number_unsigned());
        design->brisc.pipeline_depth = j["pipeline-depth"].get<unsigned>();
        std::cout << j << std::endl;
        add_insts(design, j["instructions"]);
    }

    void xify_unused_ports(
            Design* design,
            const std::set<IdString>& inports,
            const std::string& inst)
    {
        nlohmann::json& j = design->brisc.j;
        for (auto it = inports.begin(); it != inports.end(); ++it) {
            std::string port = it->str();
            std::cout << port << std::endl;
            int width = get_width(design->top_module(), port);
            if (!j["instructions"].contains(inst)) {
                log_error("Language File missing expected instruction %s",
                        inst.c_str());
                log_abort();
            }
            if (j["instructions"][inst]["ports"].contains(port)) {
                log("xifying %s\n", port.c_str());
                log_assert(
                        j["instructions"][inst]["ports"][port].
                            get<std::string>().size() ==
                                                static_cast<size_t>(width));
            }
            else {
                j["instructions"][inst]["ports"][port] = std::string(width, 'x');
            }
        }
    }


    ///////////////////////////////////////////////////////////////////////////
    /// @brief Transforms input signal into binary tristate representation
    ///        i.e. 12'h3x2 -> 0011xxxx0010
    /// @param signal signal is in the form: <num>'b[10xX]{num}, 
    ///        <num>'h[0-9a-fA-FxX]{num/4}, or [10xX]+
    ///        or as a straight binary string
    /// @return the transformed version of signal
    ///////////////////////////////////////////////////////////////////////////
    std::string transform_signal(const std::string& signal)
    {
        // Check To Make Sure signal is formatted correctly
        std::cout << "Transforming: " << signal << std::endl;
        const char* csig = signal.c_str();
        std::cmatch cm;
        std::string matched;
        bool bin = false, hex = false;
        if (std::regex_match(csig, cm, bin_regex)) {
            log("Bin\n");
            matched = signal;
            bin = true;
        }
        else if (std::regex_match(csig, cm, bin_repr_regex)) {
            log("Bin-repr\n");
            matched = cm[1];
            bin = true;
        }
        else if (std::regex_match(csig, cm, hex_repr_regex)) {
            log("hex-repr\n");
            matched = cm[1];
            hex = true;
        }
        else {
            log_error("Invalid signal specificatin: %s\n", csig);
            log_abort();
        }

        log("Matched signal: %s\n", matched.c_str());
        std::string rv = "";
        if (bin) {
            for (const char c : matched) {
                switch (c) {
                    case '0':
                    case '1':
                    case 'x':
                    case 'X':
                        rv += c;
                        break;
                    case '_':
                        break;
                    default:
                        log_error("Unknown character %c in %s\n",
                                  c, matched.c_str());
                        log_abort();
                }
            }
        }
        else if (hex) {
            for (const char c : matched) {
                unsigned char k;
                bool use_k = false, use_x = false;
                switch (tolower(c)) {
                    case '0' ... '9':
                        k = c - '0';
                        use_k = true;
                        break;
                    case 'a' ... 'f':
                        k = c - 'a' + 10;
                        use_k = true;
                        break;
                    case 'x':
                        use_x = true;;
                        break;
                    case '_':
                        break;
                    default:
                        log_error("Unknown character %c in %s\n",
                                  c, matched.c_str());
                        log_abort();
                }
                if (use_k) {
                    for (int i = 3; i >= 0; --i) {
                        if (k & (1 << i)) {
                            rv += '1';
                        }
                        else {
                            rv += '0';
                        }
                    }
                }
                else if (use_x) {
                    rv += "xxxx";
                }
                else {
                    log_error("%s %d: Shouldn't reach here\n",
                              __FILE__, __LINE__);
                    log_abort();
                }
            }
        }
        else {
            log_error("%s %d: Shouldn't reach here\n", __FILE__, __LINE__);
            log_abort();
        }
        std::cout << "Transformed: " << rv << std::endl;
        return rv;
    }

    void get_clock(Design* design, const nlohmann::json& j)
    {
        log_assert(j.contains("clock"));
        design->brisc.clock = j.find("clock")->get<std::string>();
    }

    nlohmann::json& load_json(Design* design, const char* langfile)
    {
        log("Loading Language file `%s`\n", langfile);
        std::ifstream i(langfile);
        i >> design->brisc.j;
        nlohmann::json& j = design->brisc.j;
        return j;
    }

    ///////////////////////////////////////////////////////////////////////////
    /// @brief Add all instructions to design
    /// @param design The design being BRISC'ed
    /// @param insts  json representation of all instructions
    ///////////////////////////////////////////////////////////////////////////
    void add_insts(Design* design, nlohmann::json& insts)
    {
        for (const auto it : insts.items()) {
            add_inst(design, it.key());
        }
    }


} LoadLangPass;

PRIVATE_NAMESPACE_END
