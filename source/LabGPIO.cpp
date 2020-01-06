#include "LabGPIO.hpp"
#include "stdio.h"

void LabGPIO::GPIO1PullDown(){

    if((GetPort()==1) & (GetPin()==15 | GetPin()==19)){
        LPC_IOCON->P1_15 |= 0b01 << 3;
    }
}
void LabGPIO::SetAsInput(){
    ports[gpio_port]->DIR &= ~(1 << gpio_pin);
}
void LabGPIO::SetAsOutput(){
    ports[gpio_port]->DIR |= 1 << gpio_pin;
}
void LabGPIO::SetDirection(Direction dir){
    (dir == Direction::kOutput) ? SetAsOutput() : SetAsInput();
}
void LabGPIO::SetHigh(){
    ports[gpio_port]->SET = 1 << gpio_pin;
}
void LabGPIO::SetLow(){
    ports[gpio_port]->CLR = 1 << gpio_pin;
}
void LabGPIO::Toggle(){
    if(ReadBool())
        SetLow();
    else
        SetHigh();
}
void LabGPIO::set(State st){
    (st == State::kHigh) ? SetHigh() : SetLow();
}
LabGPIO::State LabGPIO::Read(){
    State s;
    return (ports[gpio_port]->PIN & 1 << gpio_pin) ? s = State::kHigh : s = State::kLow; 
}
bool LabGPIO::ReadBool(){
    return (ports[gpio_port]->PIN & 1 << gpio_pin); 
}
void LabGPIO::GPIOInterruptHandler(){
    uint8_t port = 0, pin;
    if(LPC_GPIOINT->IO0IntStatF | LPC_GPIOINT->IO0IntStatR){
        port = 0;
        pin = __builtin_ctz((LPC_GPIOINT->IO0IntStatR) | (LPC_GPIOINT->IO0IntStatF));  
        LPC_GPIOINT->IO0IntClr &= ~(1 << pin);  
        isr_map[port][pin]();
    }
    else {
        port = 2;
        pin = __builtin_ctz((LPC_GPIOINT->IO2IntStatR) | (LPC_GPIOINT->IO2IntStatF));
        LPC_GPIOINT->IO2IntClr &= ~(1 << pin);
        isr_map[port][pin]();
    }
    
    

}
void LabGPIO::EnableInterrupts(){
    RegisterIsr(GPIO_IRQn, GPIOInterruptHandler);
}
void LabGPIO::AttachInterruptHandler(IsrPointer isr, Edge edge){
   LabGPIO::isr_map[gpio_port][gpio_pin] = isr; //place function pointer into isr_map
    if(edge == Edge::kRising){
        if(gpio_port == 0){
            LPC_GPIOINT->IO0IntEnR |= 1 << gpio_pin;
        }
        else {
            LPC_GPIOINT->IO2IntEnR |= 1 << gpio_pin;
        }
    }
    else if(edge == Edge::kFalling){
        if(gpio_port == 0){
            LPC_GPIOINT->IO0IntEnF |= 1 << gpio_pin;
        }
        else {
            LPC_GPIOINT->IO2IntEnF |= 1 << gpio_pin;
        }
    }
    else if(edge == Edge::kBoth){
        if(gpio_port == 0){
            (LPC_GPIOINT->IO0IntEnF |= 1 << gpio_pin) & (LPC_GPIOINT->IO0IntEnR |= 1 << gpio_pin);
        }
        else {
            (LPC_GPIOINT->IO2IntEnF |= 1 << gpio_pin) & (LPC_GPIOINT->IO2IntEnR |= 1 << gpio_pin);
        }
    }
    else
    {
        if(gpio_port == 0){
            (LPC_GPIOINT->IO0IntEnF &= ~(1 << gpio_pin)) & (LPC_GPIOINT->IO0IntEnR &= ~(1 << gpio_pin));
        }
        else {
            (LPC_GPIOINT->IO2IntEnF &= ~(1 << gpio_pin)) & (LPC_GPIOINT->IO2IntEnR &= ~(1 << gpio_pin));
        }
    }
    
}
int LabGPIO::GetPin(){
    return gpio_pin;
}
int LabGPIO::GetPort(){
    return gpio_port;
}
