#include "kernel/yosys.h"

#ifndef BRISC_COMMON_HPP
#define BRISC_COMMON_HPP

USING_YOSYS_NAMESPACE
PRIVATE_NAMESPACE_BEGIN

int get_width(Module* mod, IdString name)
{
    Wire* _wire = mod->wire(name);
    return _wire->width;
}

bool is_ff(const Cell* cell)
{
    return cell->hasPort(ID(Q));
}

///////////////////////////////////////////////////////////////////////////////
/// @brief Add instruction to Design::BRISC::insts
/// @param design The design being BRISC'ed
/// @param inst Name of an instruction
/// @remark Design must have its json entry set
///////////////////////////////////////////////////////////////////////////////
void add_inst(Design* design, const std::string& inst)
{
    log("Adding inst %s\n", inst.c_str());

    const nlohmann::json& j = design->brisc.j;
    if (j["instructions"].count(inst) == 0) {
        log_error("no instruction %s in config\n", inst.c_str());
        std::cout << j["instructions"] << std::endl;
        log_abort();
    }

    // TODO Also Registers
    const nlohmann::json& jports = j["instructions"][inst]["ports"];
    const std::set<IdString>& inports = design->brisc.input_ports;

    std::vector<RTLIL::State> cvec;
    for (auto port = inports.begin(); port != inports.end(); ++port) {
        // if (0 == design->brisc.clock.compare(port->c_str())) {
        //     log("Skipping %s\n", design->brisc.clock.c_str());
        //     continue;
        // }
        for (char c : j["instructions"][inst]["ports"][port->c_str()].
                            get<std::string>()) {
            switch (c) {
                case '0':
                    cvec.push_back(State::S0);
                    break;
                case '1':
                    cvec.push_back(State::S1);
                    break;
                case 'x':
                case 'X':
                    cvec.push_back(State::Sx);
                    break;
                default:
                    log_error("Invalid bit, '%c', in inst %s\n", c,
                            inst.c_str());
                    log_abort();
            }
        }
    }

    design->brisc.insts[inst] = RTLIL::Const(cvec);
    log_const(design->brisc.insts[inst]);
    std::cout << design->brisc.insts[inst].as_string() << std::endl;
}

PRIVATE_NAMESPACE_END
#endif
