#pragma once 

#include <cstdint>
#include "L0_LowLevel/LPC40xx.h"
#include "L0_LowLevel/interrupt.hpp"
class LabGPIO
{
 public:
  enum class Direction : uint8_t
  {
    kInput  = 0,
    kOutput = 1
  };
  enum class State : uint8_t
  {
    kLow  = 0,
    kHigh = 1
  };
  enum class Edge
  {
    kNone = 0,
    kRising,
    kFalling,
    kBoth
  };
  static constexpr size_t kPorts = 2;
  static constexpr size_t kPins = 32;

  inline static LPC_GPIO_TypeDef *ports[6] = {LPC_GPIO0, LPC_GPIO1, LPC_GPIO2, LPC_GPIO3, LPC_GPIO4, LPC_GPIO5};
  LabGPIO(uint8_t port, uint8_t pin) : gpio_port(port), gpio_pin(pin), int_port(port){};
  LabGPIO(){};
  void SetAsInput();

  void SetAsOutput();

  void GPIO1PullDown();

  void SetDirection(Direction direction);

  void SetHigh();

  void SetLow();

  void Toggle();
  void set(State state);
  State Read();
  bool ReadBool();
  void AttachInterruptHandler(IsrPointer isr, Edge edge); //place into isr_map location of GPIO instance's attachment handler
  static void EnableInterrupts();
  int GetPin();
  int GetPort();
 private:
  uint8_t gpio_port;
  uint8_t gpio_pin;
  uint8_t int_port;
  static inline IsrPointer isr_map[kPorts][kPins] = {nullptr}; //function map
  static void GPIOInterruptHandler(); //master GPIO interrupt handler using lookup table
};