#include "BRISC/poset.h"

// bool BRISC::Const::operator<=(const BRISC::Const &other) const
// {
//     const std::vector<RTLIL::State>& bitsL = self->bits;
//     const std::vector<RTLIL::State>& bitsR = other.bits;
//     if (bitsL.size() != bitsR.size())
//         log_error("Comparing two RTLIL::Const of lengths %zu and %zu",
//                   conL.bits.size(), conR.bits.size());
// 
//     for (size_t i = 0; i < conL.bits.size()) {
//         const RTLIL::State bL = bitsL[i];
//         const RTLIL::State bR = bitsR[i];
//         switch (bL) {
//             case RTLIL::State::S0:
//                 switch (bR) {
//                     RTLIL::State::S0:
//                     RTLIL::State::Sx:
//                         break;
//                     RTLIL::State::S1:
//                         return false;
//                     default:
//                         log_error("Const bit value is %d", bR);
//                 }
//                 break;
//             case RTLIL::State::S1:
//                 switch (bR) {
//                     RTLIL::State::S1:
//                     RTLIL::State::Sx:
//                         break;
//                     RTLIL::State::S0:
//                         return false;
//                     default:
//                         log_error("Const bit value is %d", bR);
//                 }
//                 break;
//             case RTLIL::State::Sx:
//                 switch (bR) {
//                     RTLIL::State::Sx:
//                         break;
//                     RTLIL::State::S0:
//                     RTLIL::State::S1:
//                         return false;
//                     default:
//                         log_error("Const bit value is %d", bR);
//                 }
//                 break;
//             default:
//                 log_error("Const bit value is %d", bR);
//         } // end switch
//     } // end for
//     return true;
// }

int
BRISC::constcmp(const BRISC::Const& L, const BRISC::Const& R)
{

}
