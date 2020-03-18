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
struct ConstPropPass: public Pass {
    ConstPropPass() : Pass("replace_consts") {}

    dict<SigBit, Cell*> sigbit_to_driver_idx;
    dict<SigBit, std::vector<Cell*>> sigbit_to_driven_idx;
    dict<SigBit, RTLIL::State> sigbit_to_initial_value;
    SigMap sigmap;

    void execute(std::vector<std::string>, Design* design) YS_OVERRIDE {
        generate_wire_cell_map(design);
        generate_initial_map(design->brisc);
        Module* mod = design->top_module();

        for (IdString wireid : design->brisc.untoggled_wires) {
            log("Wire %s untoggled\n", wireid.c_str());
            Wire* wire = mod->wires_[wireid];
            SigBit sb(wire);
            if (wire->port_input) {
                log("Unused input port %s\n", wireid.c_str());
                set_const_input(mod, sb);
                continue;
            }

            Cell* cell = sigbit_to_driver_idx[sb];
            log_assert(cell != nullptr);
            for (auto &conn : cell->connections()) {
                if (cell->output(conn.first)) {
                    log("In celltype %s\n", cell->type.c_str());
                    log("Can replace %s with constant %d\n",
                            conn.first.c_str(), sigbit_to_initial_value[sb]);
                    log_assert(cell->hasPort(conn.first));
                    SigSpec portss = cell->getPort(conn.first);
                    replace_cell(mod, cell, sigbit_to_initial_value[sb]);
                }
            }
        }
    }

    void replace_cell(Module* mod, Cell* cell, RTLIL::State value)
    {
        Wire* output_wire = nullptr;
        IdString s = cell->name;
        for (auto& conn : cell->connections()) {
            if (cell->output(conn.first)) {
                output_wire = conn.second.as_wire();
                break;
            }
        }
        log_assert(nullptr != output_wire);
        mod->remove(cell);
        mod->addBufGate(s, value, output_wire);
    }

    void generate_initial_map(const RTLIL::BRISC& brisc)
    {
        for (size_t i = 0; i < brisc.all_sigs->bits().size(); ++i) {
            sigbit_to_initial_value[brisc.all_sigs->bits()[i]] =
                brisc.initial_values->bits()[i].data;
        }
    }

    void generate_wire_cell_map(Design* design)
    {
        Module* mod = design->top_module();
        sigmap = SigMap(mod);
        for (auto cell : mod->cells()) {
            for (auto &conn : cell->connections()) {
                if (cell->output(conn.first)) {
                    for (auto bit : sigmap(conn.second)) {
                        sigbit_to_driver_idx[bit] = cell;
                    }
                }
                if (cell->input(conn.first)) {
                    for (auto bit : sigmap(conn.second)) {
                        sigbit_to_driven_idx[bit].push_back(cell);
                    }
                }
            }
        }
    }

    void set_const_input(Module* mod, SigBit bit)
    {
        log("%s %d\n", __FILE__, __LINE__);
        log("handling unused input %s\n", bit.wire->name.c_str());
        for (Cell* cell : sigbit_to_driven_idx[bit]) {
            log("%s %d\n", __FILE__, __LINE__);
            for (auto& conn : cell->connections()) {
                log("%s %d\n", __FILE__, __LINE__);
                if (cell->output(conn.first)) {
                    log("%s %d\n", __FILE__, __LINE__);
                    continue;
                }
                log("%s %d\n", __FILE__, __LINE__);
                if (conn.second.as_bit().wire == bit.wire) {
                    log("%s %d\n", __FILE__, __LINE__);
                    /// TODO Fix This with real value!
                    cell->setPort(conn.first, State::S0);
                    log("%s %d\n", __FILE__, __LINE__);
                }
            }
        }
    }

} ConstPropPass;

PRIVATE_NAMESPACE_END

