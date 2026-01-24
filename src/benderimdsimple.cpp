#include "benderimdsimple.h"
#include "iomatrix.h"

void BenderIMDSimple::DeInit() {
    // Reset state and fault flag
    imd_fault_flag = 0;
    Param::SetInt(Param::ImdState, IMD_states::IMD_OFF);
    Param::SetInt(Param::ImdFault, imd_fault_flag);
}

void BenderIMDSimple::Task100Ms() {
    int imd_meas = IOMatrix::GetAnaloguePin(IOMatrix::IMD_OK)->Get();
    if (imd_meas > 2000) {
        Param::SetInt(Param::ImdState, IMD_states::IMD_NORMAL);
        imd_fault_flag = 0;
    } else {
        Param::SetInt(Param::ImdState, IMD_states::IMD_MEASUREMENT_ERROR);
        imd_fault_flag = 1;
    }

    Param::SetInt(Param::ImdFault, imd_fault_flag);
}