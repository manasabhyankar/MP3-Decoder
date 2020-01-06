#include "L0_LowLevel/LPC40xx.h"
#include "LabGPIO.hpp"
#include <string>
class LabSPI
{
 public:
    /*
        Describes FrameFormat mode as per datasheet
    */
    enum FrameModes
    {
        kSPI = 0b00,
        kTI = 0b01,
        kMicrowire = 0b10,
        kNotSupp = 0b11
    };
    /*
        Describes shift amounts/bit locations of relevant control bits in CR0/CR1
    */
    enum RegisterShiftValues{
        kDataSize = 0,    //CR0
        kFrameFormat = 4, //CR0
        kPolarity = 6,    //CR0
        kPhase = 7,       //CR0
        kClkRate = 8,     //CR0
        kEnable = 1,      //CR1
        kBusyBit = 4      //SR
    };
    /*
        Lookup table of SSPn to relevant pins
    */
    volatile uint32_t *SPI_pin[3][3] = {
        {&LPC_IOCON->P0_18, &LPC_IOCON->P0_17, &LPC_IOCON->P0_15},
        {&LPC_IOCON->P0_9, &LPC_IOCON->P0_8, &LPC_IOCON->P0_7},
        {&LPC_IOCON->P1_1, &LPC_IOCON->P1_0, &LPC_IOCON->P1_4}
    };
    /*
        Describes the status bits coming out of the flash's SR
    */
    typedef union{ 
        uint8_t status_byte;
        struct {
            uint8_t kBPL : 1;
            uint8_t kRES1_6 : 1;
            uint8_t kEPE : 1;
            uint8_t kWPP : 1;
            uint8_t kRES1_3 : 1;
            uint8_t kBPO : 1;
            uint8_t kWEL : 1;
            uint8_t kBSY1_0 : 1;
            uint8_t kRES2_7 : 1;
            uint8_t kRES2_6 : 1;
            uint8_t kRES2_5 : 1;
            uint8_t kRSTE : 1;
            uint8_t kRES2_3 : 1;
            uint8_t kRES2_2 : 1;
            uint8_t kRES2_1 : 1;
            uint8_t kBSY2_0 : 1;
        }__attribute__((packed));
    } FlashSRByte1;
    /*
        Static string array for relevant status messages for flash's SR.
    */
    inline static const char *status_strings[2][16] = {
        {"Block protection unlocked.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Erase or program operation was successful.\n",
         "Write protection is currently enabled.\n"
         "Bit is reserved for future use and currently unused.\n", 
         "Memory array is unprotected.\n",
         "The device is not write enabled.\n",
         "Device is ready for communication.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Resets to this device will not be processed.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Device is ready for communication.\n",
         },
        {"Block protection locked.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Erase or program operation was not successful.\n",
         "Write protection has been disabled.\n"
         "Bit is reserved for future use and currently unused.\n", 
         "Memory array is protected.\n",
         "The device is write enabled.\n",
         "Device is currently busy.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Reset commands are enabled.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Bit is reserved for future use and currently unused.\n",
         "Device is currently busy.\n",
         }
    };
    /*
    Describes pins and function bits required to set function on SSP pins for IOCON register
        SSP0 pins [MOSI, MISO, SCK]: P0_18, P0_17, P0_15; SSP0 pins are function bit 010 for SSP config
        SSP1 pins [MOSI, MISO, SCK]: P0_9, P0_8, P0_7; SSP1 pins are function bit 010 for SSP config
        SSP2 pins [MOSI, MISO, SCK]: P1_1, P1_4, P1_0; is directly connected to Adesto flash, SSP2 pins are function bit 100 for SPI config
    */
    int FunctionModeShift[3] = {2, 2, 4};
    /*
    Describes bit locations in PCONP register for SSP modules
        SSP0: bit 21
        SSP1: bit 10
        SSP2: bit 20
    */
    int PowerShiftValues[3] = {21, 10, 20};
    inline static LPC_SSP_TypeDef *SSP_n[3] = {LPC_SSP0, LPC_SSP1, LPC_SSP2};
    /*
    TODO: Find a better way to have pin access without needing to access each pin individually and checking SSP_n
    */
    LabSPI(uint8_t SSP_num){
        whichSSP = SSP_num;
    }
    LabSPI(){}
    /*Takes no parameters
    Sets the appropriate bit in PCONP based on whichSSP value
    */
    void SSPnPower();
    /*
    Takes 1 parameter
    Set peripheral clock registers to reasonable values: CPSR and SCR
    */
    bool SetClocks(uint8_t divide);
    /*
    Takes no parameters
    Set pin function modes
    */
    void SetPinMode();
    /**
     * 1) Powers on SPPn peripheral
     * 2) Set peripheral clock
     * 3) Sets pins for specified peripheral to MOSI, MISO, and SCK
     *
     * @param data_size_select transfer size data width; To optimize the code, look for a pattern in the datasheet
     * @param format is the code format for which synchronous serial protocol you want to use.
     * @param divide is the how much to divide the clock for SSP; take care of error cases such as the value of 0, 1, and odd numbers
     *
     * @return true if initialization was successful
     */
    bool Initialize(uint8_t data_size_select, FrameModes format, uint8_t divide);

    /**
     * Transfers a byte via SSP to an external device using the SSP data register.
     * This region must be protected by a mutex static to this class.
     *
     * @return received byte from external device via SSP data register.
     */
    uint8_t Transfer(uint8_t send);
    /*
        Get readout of the bytes sent from the status register.
        returns single uint8_t of info from flash SR byte 1&2
    */
    uint8_t GetStatusByte();
 private:
	  uint8_t whichSSP;
};