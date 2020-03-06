#include "BRISC/briscconst.hpp"

///////////////////////////////////////////////////////////////////////////
/// @brief compares two BRISC::Const structures
/// @return return negative number if 
/// @see BRISC::Const
///////////////////////////////////////////////////////////////////////////
BRISC::BRISCConst::Comparison cmp(const BRISC::BRISCCont& L,
                                         const BRISC::BRISCCont& R)
{
    bool lt = true;
    bool gt = true;
    bool eq = L.size() == R.size();
    if (!eq) {
        log_error("Cannot compare two BRISCConst of different size");
        exit(1);
    }

    for (int i = 0; i < L.size(); ++i) {
        RTLIL::State sl = L.bits[i];
        RTLIL::State sr = R.bits[i];
        using Comparison = BRISC::BRISCConst::Comparison;
        switch (sl) {
            RTLIL::S0:
                switch (rl) {
                    RTLIL::S0:
                        break;
                    RTLIL::S1:
                        return Comparison::NC;
                    RTLIL::Sx:
                        gt = false;
                        eq = false;
                }
        }
    }

}
