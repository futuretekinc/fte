#include "fte_target.h"
#include "radio.h"
#include "fte_lora.h"
#include "sx1276-LoRaRegs.h"

const char_ptr    _pOpModeStrings[] = 
{
    "Sleep", 
    "Standby", 
    "Synthesizer Tx", 
    "Transmitter", 
    "Synthesizer Rx", 
    "Receiver", 
    "Receiver Single", 
    "CAD"
};

char_ptr    FTE_LORA_getOpModeString(uint_32 ulOpMode)
{
    if (ulOpMode & ~RFLR_OPMODE_MASK)
    {
        return  _pOpModeStrings[ulOpMode & ~RFLR_OPMODE_MASK];
    }
    
    return  "Unknown";
}
