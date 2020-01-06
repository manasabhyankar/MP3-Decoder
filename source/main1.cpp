#include <stdio.h>
#include "MP3_Decoder.hpp"
#include <string>
#include "utility/rtos.hpp"
#include "third_party/FreeRTOS/Source/include/FreeRTOS.h"
#include "third_party/FreeRTOS/Source/include/queue.h"
#include "third_party/FreeRTOS/Source/include/task.h"


MP3_Decoder mp3;
FATFS tester;
FIL dir;
UINT br;
uint8_t chunk_count = 0;
uint16_t byte_count = 0;    
uint8_t buff[2048] = {};
char songlist[10][256];
FRESULT fres;
OnBoardLed led;
LabGPIO S3(0, 29);
LabGPIO S2(0, 30);
LabGPIO S1(1, 15);
LabGPIO S0(1, 19);
OledTerminal tt;
void SW3ISR()	//ISR switch between playing and pause music
{
	switch(mp3.currentGPIOstate)
	{
		case mp3.GPIOstate::kTrack:
            mp3.songIndex -= 1; //play the previous song
            mp3.currentGPIOfunction = mp3.GPIOfunction::kBackTrack;
			break;
		case mp3.GPIOstate::kVolume:
        if(mp3.volume-10 < 0){
            mp3.volume = mp3.MinVol;
        }
        else{
            mp3.volume -= 10; //reduce the volume
        }
            mp3.currentGPIOfunction = mp3.GPIOfunction::kVolumeDown;
			break;
        case mp3.GPIOstate::kPlayback:
            switch(mp3.currentPlayState){
                case mp3.playState::kPlay:  //pause the song
                    mp3.currentPlayState = mp3.playState::kPause;
                    break;
                case mp3.playState::kPause: //resume the song
                    mp3.currentPlayState = mp3.playState::kPlay;
                    break;
            }
        break;
        default:
            mp3.currentGPIOfunction = mp3.GPIOfunction::kIDLE;
        break;
	}
    LPC_GPIOINT->IO0IntClr |= 1 << 29;
}

void SW2ISR()	//ISR switch between volume control and track control state with SW1 and SW0
{
	switch(mp3.currentGPIOstate)
	{
		case mp3.GPIOstate::kTrack:
            mp3.songIndex += 1; //play the next song
            mp3.currentGPIOfunction = mp3.GPIOfunction::kNextTrack;
			break;
		case mp3.GPIOstate::kVolume:
            if(mp3.volume+10 < 0){
                mp3.volume = mp3.MaxVol;
            }
            else{
                mp3.volume += 10; //increase the volume
            }
            
            mp3.currentGPIOfunction = mp3.GPIOfunction::kVolumeUp;
			break;
        default:
            mp3.currentGPIOfunction = mp3.GPIOfunction::kIDLE;
            break;
	}
    LPC_GPIOINT->IO0IntClr |= 1 << 30;
}

void timerScanISR()
{
	switch (mp3.currentGPIOstate)
	{
		case mp3.GPIOstate::kTrack:
			if (S1.ReadBool())
			{
				led.Off(1);
                led.Off(0);		
                mp3.oled.Clear();
                mp3.oled.printf("Volume control state\n");								//the toggle lets the user know if their button registered
				mp3.currentGPIOstate = mp3.GPIOstate::kVolume;
			}
			break;
		case mp3.GPIOstate::kVolume:
			if (S1.ReadBool())
			{
                led.Off(1);
				led.On(0);
                mp3.oled.Clear();
                mp3.oled.printf("Playback control state\n");
				mp3.currentGPIOstate = mp3.GPIOstate::kPlayback;
			}
			break;
        case mp3.GPIOstate::kPlayback:
            if (S1.ReadBool())
			{
				led.On(1);
                led.On(0);
                mp3.oled.Clear();
                mp3.oled.printf("Track control state\n");
				mp3.currentGPIOstate = mp3.GPIOstate::kTrack;
			}
            break;
        default:
			break;
	}
}

void play(void *){
    int update = 0;
    while(1){
        mp3.oled.Clear();
        mp3.oled.printf("Playing: \n%s\n\n", mp3.songlist[mp3.songIndex]);
        mp3.currentPlayState = mp3.playState::kPlay;
        while(!f_eof(&dir)){
            f_read(&dir, buff, 2048, &br);
            while(chunk_count < sizeof(buff)/32){
                byte_count = chunk_count * 32;
                while(!mp3.waitDREQ());
                mp3.DataWrite(buff, 32, byte_count);
                chunk_count++;
                
                switch (mp3.currentPlayState)
			    {
                    
                    case mp3.playState::kPlay:
                        switch (mp3.currentGPIOfunction)
                        {
                            case mp3.GPIOfunction::kVolumeUp:
                                mp3.WriteRegister(mp3.VOL, mp3.volume * mp3.VOLUME_OFFSET);
                                mp3.currentGPIOfunction = mp3.GPIOfunction::kIDLE;
                                break;
                            case mp3.GPIOfunction::kVolumeDown:
                                mp3.WriteRegister(mp3.VOL, mp3.volume * mp3.VOLUME_OFFSET);
                                mp3.currentGPIOfunction = mp3.GPIOfunction::kIDLE;
                                break;
                            case mp3.GPIOfunction::kBackTrack:
                                if(mp3.songIndex <= 0)mp3.songIndex = mp3.maxSong;
                                else mp3.songIndex--;
                                //oled.Clear();
                                f_close(&dir);
                                // close sd file
                                mp3.SoftReset();
                                //init function to start the next song
                                f_open(&dir, mp3.songlist[mp3.songIndex], FA_READ);
                                mp3.oled.Clear();
                                mp3.oled.printf("Playing: \n%s\n", mp3.songlist[mp3.songIndex]);
                                mp3.currentGPIOfunction = mp3.GPIOfunction::kIDLE;
                                break;
                            case mp3.GPIOfunction::kNextTrack:
                                //oled.Clear();
                                if (mp3.songIndex+1 >= mp3.maxSong)mp3.songIndex = 0;
                                else mp3.songIndex++;
                                f_close(&dir);
                                // close sd file
                                mp3.SoftReset();
                                //init function to start the next song
                                f_open(&dir, mp3.songlist[mp3.songIndex], FA_READ);
                                mp3.oled.Clear();
                                mp3.oled.printf("Playing: \n%s\n", mp3.songlist[mp3.songIndex]);
                                mp3.currentGPIOfunction = mp3.GPIOfunction::kIDLE;
                                break;
                            default:
                                break;
                        }
                        break;
                    case mp3.playState::kPause:
                        while (mp3.currentPlayState == mp3.playState::kPause);	//pause until state changes
                        break;
			    }
            }
            chunk_count = 0;
            
        }
        
    }
}
int main(){   
    bool success = mp3.initialize();
    mp3.HardReset();
    char path[10];
    led.Initialize();
    S3.AttachInterruptHandler(SW3ISR, LabGPIO::Edge::kRising);
    S2.AttachInterruptHandler(SW2ISR, LabGPIO::Edge::kRising);
    S3.EnableInterrupts();
    S2.EnableInterrupts();
    fres = f_mount(&tester, "", 0);
    if(fres == FR_OK){
        strcpy(path, "/");
        fres = mp3.scan_files(path);
    }
    
    fres = f_open(&dir, mp3.songlist[mp3.songIndex], FA_READ);
    if(fres == FR_OK){
        while(!mp3.waitDREQ());
        Timer timerScan(TimerInterface::kTimer1);
        timerScan.Initialize(1'000'000,timerScanISR);
        timerScan.SetTimer(1'000'000,Timer::TimerIsrCondition::kInterruptRestart); //1'000'000 or 750'000 for the speed
        xTaskCreate(play, "play", 1024, NULL, 5, NULL);
        vTaskStartScheduler();
        printf("Opened track successfully!\n");
    }

    
    while(true);
}