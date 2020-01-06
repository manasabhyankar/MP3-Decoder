#include "LabGPIO.hpp"
#include "LabSPI.hpp"
#include "L0_LowLevel/LPC40xx.h"
#include "third_party/fatfs/source/ff.h"
#include "third_party/fatfs/source/ffconf.h"
#include "third_party/fatfs/source/diskio.h"
#include "L1_Drivers/timer.hpp"
#include "L2_HAL/displays/led/onboard_led.hpp"
#include "utility/log.hpp"
#include "utility/time.hpp"
#include "L3_Application/oled_terminal.hpp"
class MP3_Decoder{
    public:
        
        MP3_Decoder(){}
        
        enum GPIOfunction
        {
            kVolumeUp = 0,
            kVolumeDown = 1,
            kBackTrack = 2,
            kNextTrack = 3,
            kIDLE = 4
        };

        enum GPIOstate
        {
            kTrack = 0,
            kVolume = 1,
            kPlayback = 2
        };
        enum RegisterSetup
        {
            MODE = 0x0,
            BASS = 0x2,
            CLOCK = 0x3,
            VOL = 0xB
        };
        enum playState
        {
            kPause = 0,
            kPlay = 1
        };
        
        enum VS1053_Commands
        {
            READ_COMMAND = 0x3,
            WRITE_COMMAND = 0x2
        };
        GPIOstate currentGPIOstate = GPIOstate::kPlayback;
        playState currentPlayState = playState::kPlay;
        GPIOfunction currentGPIOfunction = GPIOfunction::kIDLE;
        uint8_t songIndex = 0;
        uint8_t maxSong = 10;
        uint16_t MaxVol = 250;
        uint16_t MinVol = 100;
        uint16_t volume = 40;
        uint16_t VOLUME_OFFSET = 0x101;
        OledTerminal oled;
        char songlist [10][256];
        bool initialize();
        void WriteRegister(unsigned char addrByte, uint16_t value);//, uint8_t value2);
        void ReadRegister(unsigned char addrByte);
        void DataWrite(uint8_t *data, uint8_t byte_count, uint32_t start);
        void HardReset();
        void SoftReset();
        void IncreaseClock();
        bool waitDREQ();
        FRESULT scan_files (char* path);
    private:
};