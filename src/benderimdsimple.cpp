#include "benderimdsimple.h"
#include "iomatrix.h"
#include "digio.h"

void BenderIMDSimple::DeInit() {
    // Reset state and fault flag
    imd_fault_flag = 0;
    Param::SetInt(Param::ImdState, IMD_states::IMD_OFF);
    Param::SetInt(Param::ImdFault, imd_fault_flag);
}

void BenderIMDSimple::Task100Ms() {
    DigIo* pin = IOMatrix::GetPin(IOMatrix::IMD_OK);

    if (pin == &DigIo::dummypin) {
        Param::SetInt(Param::ImdState, IMD_states::IMD_OFF);
        Param::SetInt(Param::ImdFault, 1);
        return;
    }

    int imd_meas = pin->Get();
    if (imd_meas == 1) {
        Param::SetInt(Param::ImdState, IMD_states::IMD_NORMAL);
        imd_fault_flag = 0;
    } else {
        Param::SetInt(Param::ImdState, IMD_states::IMD_MEASUREMENT_ERROR);
        imd_fault_flag = 1;
    }

    Param::SetInt(Param::ImdFault, imd_fault_flag);
}