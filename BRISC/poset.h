#include "kernel/yosys.h"
#include "kernel/rtlil.h"

#ifndef BRISC_POSET_H
#define BRISC_POSET_H
YOSYS_NAMESPACE_BEGIN

///////////////////////////////////////////////////////////////////////////////
/// @class BRISC::Const
/// @brief A version of RTLIL::Const with operator overloads suitable for
///        BRISC's Poset
/// @see RTLIL::Const
///////////////////////////////////////////////////////////////////////////////
struct BRISC::Const : public RTLIL::Const
{
	bool operator <(const BRISC::Const &other) const = delete;
	bool operator ==(const BRISC::Const &other) const = delete;
	bool operator !=(const BRISC::Const &other) const = delete;
	bool operator <=(const BRISC::Const &other) const = delete;
    bool operator >(const BRISC::Const &other) const = delete;
}

///////////////////////////////////////////////////////////////////////////////
/// @brief compares two BRISC::Const structures
/// @return return negative number if 
/// @see BRISC::Const
///////////////////////////////////////////////////////////////////////////////
int BRISC::constcmp(const BRISC::Const& L, const BRISC::Const& R);

struct BRISC::Poset
{
    // Notes:
    // max_poset_ --- vector of evaluated signals (input ports & regs)
    std::vector<RTLIL::Const> max_poset_;

    ///////////////////////////////////////////////////////////////////////////
    /// @brief Inserts an RTLIL::Const into the Poset.  The Poset may not
    ///        actually contain it if it already contains a larger value
    ///
    /// @param con The RTLIL::Const to insert into the Poset
    /// @return insert returns true iff the Poset did not already contain
    ///         a larger value
    ///////////////////////////////////////////////////////////////////////////
    bool insert(const RTLIL::Const &con);

private:
    ///////////////////////////////////////////////////////////////////////////
    /// @brief Does less than or equal comparison of two elements of the Poset
    ///
    /// @param conL The left hand side of the less than comparison
    /// @param conR The right hand side of the less than comparison
    /// @return lt returns true iff the conL <= conR
    ///////////////////////////////////////////////////////////////////////////
    bool compare(const RTLIL::Const &conL, const RTLIL::Const &conR);
}

YOSYS_NAMESPACE_END
#endif
