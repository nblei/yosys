#include "kernel/yosys.h"
#ifndef BRISC_CONST_H
#define BRISC_CONST_H
YOSYS_NAMESPACE_BEGIN

struct BRISC::BRISCConst : public RTLIL::Const {
    enum class {
        LT,     /// Less Than
        EQ,     /// Equal
        GT,     /// Greather Than
        NC      /// No Comparison
    } Comparison;

};

YOSYS_NAMESPACE_END
#endif
