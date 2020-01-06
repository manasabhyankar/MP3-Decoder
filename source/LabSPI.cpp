#include "LabSPI.hpp"
#include "L0_LowLevel/LPC40xx.h"
#include <stdio.h>

void LabSPI::SSPnPower(){
    
    printf("Setting ON SSP%u.\n", whichSSP);
    LPC_SC->PCONP |= 1 << PowerShiftValues[whichSSP];
}
bool LabSPI::SetClocks(uint8_t divide){
    printf("Setting clock rate.\n");
    SSP_n[whichSSP]->CPSR |= 1 << 2; //set CPSR to 2
    //if((divide%2 == 0) & (divide != 0)){
        SSP_n[whichSSP]->CR0 |= divide << RegisterShiftValues::kClkRate;
        //return true;
    //}
    // else {
    //     printf("That value of clock division is invalid!\n");
    //     return false;
    // }
}
void LabSPI::SetPinMode(){
    printf("Setting pin mode to SSP.\n");
    for(int i=0; i<3; i++){
        *SPI_pin[whichSSP][i] = ((*SPI_pin[whichSSP][i] & ~(FunctionModeShift[whichSSP])) | (FunctionModeShift[whichSSP]));
    }
}
bool LabSPI::Initialize(uint8_t data_size_select, FrameModes format, uint8_t divide){
    SSPnPower(); //turn on SSPn
    bool cont = SetClocks(divide); //set clock
   if(cont){
      SetPinMode(); //set pin mode to SSP
      printf("Setting data_size_select to %u.\nSetting format to SPI mode.\nEnabling device.\n", data_size_select);
      SSP_n[whichSSP]->CR0 |= (data_size_select-1) << kDataSize;
      SSP_n[whichSSP]->CR0 |= format << kFrameFormat;
      SSP_n[whichSSP]->CR0 |= 1 << kPolarity;
      SSP_n[whichSSP]->CR0 |= 1 << kPhase;
      SSP_n[whichSSP]->CR1 |= 1 << kEnable;
      return true;
   }
   else {return false;};
}
uint8_t LabSPI::Transfer(uint8_t data){
    
    
    SSP_n[whichSSP]->DR = data;
    //check if the device is busy
    while(SSP_n[whichSSP]->SR & 1 << kBusyBit){
        continue;
    }
    //if the loop exits, then the device is no longer busy
    //return the value inside the DR
    
    return static_cast<uint8_t>(SSP_n[whichSSP]->DR);
}
uint8_t LabSPI::GetStatusByte(){
    FlashSRByte1 byte;
    Transfer(0x05);
    byte.status_byte = Transfer(0);
    //printf("%s%s", bit_rep[byte.status_byte >> 4], bit_rep[byte.status_byte & 0x0F]);
    byte.status_byte = Transfer(0);
    //printf("%s%s", bit_rep[byte.status_byte >> 4], bit_rep[byte.status_byte & 0x0F]);
    return byte.status_byte;
}