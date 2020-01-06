/*
	Two button State Machine
	3 States
	State 01: Previous Track +  Next Track
	State 10: Lower Volume + Higher Volume
	State 11: Back Track + Next Track
*/

#define MaxVol 250
#define MinVol 100

#define CLK_DIV2 2
#define CLK_DIV8 8
/*TODO: check Arduino's peripheral clock*/

#define SPI_VOL 0xb
#define SPI_MODE 0x0
#define SPI_HDAT0 0xABAD
#define SPI_HDAT1 0x1DEA

#define XCS A3
#define XDCS A2
#define DREQ 0xA1

#define WRITE_COMMAND 0x02
#define READ_COMMAND 0x03

#define VOLUME_OFFSET 0x101

/*vSemaphoreCreateBinary(play_sem);*/ //makes a binary semaphore in main()

volatile typedef enum GPIOfunction : uint8_t
{
	kVolumeUp = 0;
	kVolumeDown = 1;
	kBackTrack = 2;
	kNextTrack = 3;
	kIDLE = 4;
};

volatile typedef enum playState : uint8_t
{
	kPause = 0;
	kPlay = 1;
};

volatile typedef enum GPIOstate : uint8_t
{
	kTrackState = 0;
	kVolumeState = 1;
};

GPIOfunction currentGPIOfunction = GPIOfunction::kIDLE;	//global variable
playState currentPlayState = playState::kPause;			//global variable
GPIOstate currentGPIOstate = GPIOstate::kTrackState;	//global variable
Gpio SW0(1, 19);	//global variable	
Gpio SW1(1, 15);	//global variable
OnBoardLed led;		//global variable, #include L2_HAL/displays/led/onboard_led.hpp

/*
	initialize SW0 and SW1 in main()

	SW0.GetPin().SetMod(Pin::Mode::kPullDown);
	SW1.GetPin().SetMod(Pin::Mode::kPullDown);
	SW0.SetAsInput();
	SW1.SetAsInput();
*/

/*
	initialize led in main()

	led.Initialize();
*/

int volume 40;
uint8_t songIndex;
uint8_t maxSong -= 1; //count amount of song through SD - 1 for index

/*
	initialize SW3 in main()

	Gpio SW3(0,29);
	SW3.GetPin().SetMode(Pin::Mode:kPullUp);
	SW3.AttachInterrupt(&PortSW3ISR,GpioInterface::Edge::kEdgeFalling);
	SW3.EnableInterrupts();
*/
// void PortSW3ISR()	//ISR switch between playing and pause music
// {
// 	switch(currentPlayState)
// 	{
// 		case GPIOfunction::kPlay:	//Play -> Pause
// 			led.Off(3);
// 			currentGPIOfunction = playState::kPause;
// 			break;
// 		case GPIOfunction::kPause: //Pause -> Play
// 			led.On(3);
// 			currentGPIOfunction = playState::kPlay;
// 			break;
// 	}
// }

/*
	initialize SW2 in main()

	Gpio SW2(0,30);
	SW2.GetPin().SetMode(Pin::Mode:kPullUp);
	SW2.AttachInterrupt(&PortSW2ISR,GpioInterface::Edge::kEdgeFalling);
	SW2.EnableInterrupts();
*/
// void PorSW2tISR()	//ISR switch between volume control and track control state with SW1 and SW0
// {
// 	switch (currentGPIOfunction)
// 	{
// 		case GPIOstate::kTrackState:	//Track -> Volume
// 			led.Off(2);
// 			currentGPIOstate = GPIOstate::kVolumeState;
// 			break;
// 		case GPIOstate::kVolume:	//Volume -> Track
// 			led.On(2);
// 			currentGPIOstate = GPIOstate::kTrack;
// 		break;
// 	}
// }

/*
	init timer in main() **careful where you start the initialization as it'll start the interrupt periodically afterwards**

	Timer timerScan(TimerInterface::kTimer0);
	timerScan.Initialize(1'000'000,timerScanISR);
	timerScan.SetTimer(1'000'000,Timer::InterruptRestart); //1'000'000 or 750'000 for the speed
*/
// void timerScanISR()
// {
// 	switch (currentGPIOstate)
// 	{
// 		case GPIOstate::kTrack:
// 			if (SW1.Read() && !SW0.Read())
// 			{
// 				led.Toggle(1);										//the toggle lets the user know if their button registered
// 				currentGPIOfunction = GPIOfunction::kVolumeDown;
// 				led.Toggle(1);
// 			}
// 			else if (!SW1.Read() && SW0.Read())
// 			{
// 				led.Toggle(0);
// 				currentGPIOfunction = GPIOfunction::kVolumeUp;
// 				led.Toggle(0);
// 			}
// 			else
// 			{
// 				currentGPIOfunction = GPIOfunction::kIDLE;
// 			}
// 			break;
// 		case GPIOstate::kVolume:
// 			if (SW1.Read() && !SW0.Read())
// 			{
// 				led.Toggle(1);
// 				currentGPIOfunction = GPIOfunction::kBackTrack;
// 				led.Toggle(1);
// 			}
// 			else if (!SW1.Read() && SW0.Read())
// 			{
// 				led.Toggle(0);
// 				currentGPIOfunction = GPIOfunction::kNextTrack;
// 				led.Toggle(0);
// 			}
// 			else
// 			{
// 				currentGPIOfunction = GPIOfunction::kIDLE;
// 			}
// 			break;
// 	}
// }


//xTaskCreate(playTask, (signed char*)"playTask", 1024, NULL, 1, &playTaskHandle); //create task in main()
// void playTask(void* p)
// {
// 	while (1)
// 	{
// 		/*open file and read into buffer*/

// 		currentPlayState == playState::kPlay;
// 		while (/*increment the bytes sent to buffer*/)
// 		{
// 			switch (currentPlayState)
// 			{
// 				case playState::kPlay:
// 					switch (currentGPIOfunction)
// 					{
// 						case GPIOfunction::kVolumeUp:
// 							volume += 10;
// 							if (volume > MaxVol) volume = MaxVol;
// 							writeRegister(SPI_VOL, volume * VOLUME_OFFSET);
// 							currentGPIOfunction = GPIOfunction::kIDLE;
// 							break;
// 						case GPIOfunction::kVolumedown:
// 							volume -= 10;
// 							if (volume < 0) volume = 0;
// 							writeRegister(SPI_VOL, volume * VOLUME_OFFSET);
// 							currentGPIOfunction = GPIOfunction::kIDLE;
// 							break;
// 						case GPIOfunction::kBackTrack:
// 							if (--songIndex < 0)songIndex = maxSong;
// 							//close sd file
// 							softReset();
// 							//init function to start the previous song
// 							currentGPIOfunction = GPIOfunction::kIDLE;
// 							break;
// 						case GPIOfunction::kNextTrack:
// 							if (++songIndex > maxSong)songIndex = 0;
// 							// close sd file
// 							softReset();
// 							//init function to start the next song
// 							currentGPIOfunction = GPIOfunction::kIDLE;
// 							break;
// 						default:
// 							break;
// 					}
// 					break;
// 				case playState::kPause:
// 					while (currentPlayState == playState::kPause);	//pause until state changes
// 					break;
// 			}
// 		}
// 		//close sd file
// 		softReset();
// 		if (++songIndex > maxSong)songIndex = 0;
// 	}
// }

// void writeRegister(unsigned char addrByte, uint32_t value)
// {
// 	//xdcs high - turn off data control
// 	//while (!readDREQ());
// 	//xcs low - turn on control
// 	SPI.Transfer(WRITE_COMMAND); //write command
// 	SPI.Transfer(addrByte);
// 	SPI.Transfer(value >> 8);
// 	SPI.Transfer(value & 0xFF);
// 	//xcs high - turn off control
// }

// void softReset()
// {
// 	SPI.SetClocks(CLK_DIV8);
// 	writeRegister(SPI_MODE, 0x0804);
// 	delay(2);
// 	//while (!readDREQ());
	
// 	writeRegister(SPI_HDAT0, 0xABAD);		//debug check
// 	writeRegister(SPI_HDAT1, 0x1DEA);
// 	if (readRegister(SPI_HDAT0) != 0xABAD || readRegister(SPI_HDAT1) != 0x1DEA)
// 	{
// 		printf("softReset() failed. Check Clock?");
// 	}
// }