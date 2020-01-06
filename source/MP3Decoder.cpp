#include "MP3_Decoder.hpp"


LabSPI data_trans(0);
LabGPIO DREQ(0, 0);
LabGPIO XCS(1, 28);
LabGPIO CS_SD(1, 23);
LabGPIO SD_Detect(2, 2);
LabGPIO XDCS(1, 20);
LabGPIO XRESET(2, 7);
LabGPIO SW3(0, 29);
LabGPIO SW2(0, 30);
LabGPIO SW1(1, 15);
LabGPIO SW0(1, 19);

bool MP3_Decoder::initialize(){
    //GPIO pin setups
    DREQ.SetAsInput();
    XCS.SetAsOutput();
    XCS.SetHigh();
    CS_SD.SetAsOutput();
    CS_SD.SetHigh();
    SD_Detect.SetAsOutput();
    SD_Detect.SetHigh();
    XDCS.SetAsOutput();
    XDCS.SetHigh();
    XRESET.SetAsOutput();
    XRESET.SetHigh();
    SW3.SetAsInput();
    SW2.SetAsInput();
    //switch setup with sw1/sw0 with pull-down enabled
    LPC_IOCON->P1_15 |= 0b01 << 3;
    LPC_IOCON->P1_19 |= 0b01 << 3;
    SW1.SetAsInput();
    SW0.SetAsInput();
    //SPI setup
    bool ret = data_trans.Initialize(8, LabSPI::FrameModes::kSPI, 8);
    return ret;
}
void MP3_Decoder::ReadRegister(unsigned char addrByte){
    XDCS.SetHigh();
    while(!DREQ.ReadBool());
    XCS.SetLow();
    data_trans.Transfer(VS1053_Commands::READ_COMMAND);
    data_trans.Transfer(addrByte);
    data_trans.Transfer(0xFF); //garbage transfer
    data_trans.Transfer(0xFF); //garbage transfer
    XCS.SetHigh();
}
void MP3_Decoder::WriteRegister(unsigned char addrByte, uint16_t value){
    XDCS.SetHigh();
    while(!DREQ.ReadBool());
    XCS.SetLow();
    data_trans.Transfer(VS1053_Commands::WRITE_COMMAND);
    data_trans.Transfer(addrByte);
    data_trans.Transfer(value >> 8);
    data_trans.Transfer(value & 0xFF);
    XCS.SetHigh();
}
void MP3_Decoder::DataWrite(uint8_t *data, uint8_t byte_count, uint32_t start){
    XCS.SetHigh();
    XDCS.SetLow();
    
    for(int i=start; i<start+byte_count; i++){
        //printf("%i, %i\n", byte_count, start);
        data_trans.Transfer(data[i]);
        //printf("%i, %i\n", data[0], data[byte_count-1]);
    }
    XDCS.SetHigh();
}
void MP3_Decoder::HardReset(){
    XRESET.SetLow();
    Delay(100); //longer than necessary reset to avoid any conflicts
    data_trans.Transfer(0x00); //garbage transfer
    XRESET.SetHigh();
    while(!DREQ.ReadBool()); //wait until DREQ goes high to continue transfers
    SoftReset();
}
void MP3_Decoder::SoftReset(){
    WriteRegister(MP3_Decoder::RegisterSetup::MODE, 0x0804);//, 0x04);
    Delay(100); //longer than necessary reset to avoid any conflicts
    //data_trans.Transfer(0x00); //garbage transfer
    WriteRegister(MP3_Decoder::RegisterSetup::MODE, 0x0880);//, 0x80);
    WriteRegister(MP3_Decoder::RegisterSetup::CLOCK, 0xC000);//, 0x00);
    WriteRegister(MP3_Decoder::RegisterSetup::BASS, 0x0055);//, 0x55);
    WriteRegister(MP3_Decoder::RegisterSetup::VOL, 0x4040);//, 0x40);
    while(!DREQ.ReadBool()); //wait until DREQ goes high to continue transfers
    WriteRegister(MP3_Decoder::RegisterSetup::MODE, 0x0880);//, 0x80);
    WriteRegister(MP3_Decoder::RegisterSetup::CLOCK, 0xC000);//, 0x00);
    WriteRegister(MP3_Decoder::RegisterSetup::BASS, 0x0055);//, 0x55);
    WriteRegister(MP3_Decoder::RegisterSetup::VOL, 0x4040);//, 0x40);
    //confirm register values read back the same as they're written
    ReadRegister(MP3_Decoder::RegisterSetup::MODE);
    ReadRegister(MP3_Decoder::RegisterSetup::CLOCK);
    ReadRegister(MP3_Decoder::RegisterSetup::BASS);
    ReadRegister(MP3_Decoder::RegisterSetup::VOL);

}
void MP3_Decoder::IncreaseClock(){
    //data_trans.Initialize(8, LabSPI::FrameModes::kSPI, 4);
    XCS.SetHigh();
    XDCS.SetHigh();
}
bool MP3_Decoder::waitDREQ(){
    return DREQ.ReadBool();
}
FRESULT MP3_Decoder::scan_files (
    char* path        /* Start node to be scanned (***also used as work area***) */
)
{
    FRESULT res;
    DIR dir;
    UINT i;
    uint8_t index;
    static FILINFO fno;

    oled.Initialize();
    res = f_opendir(&dir, path);                       /* Open the directory */
    if (res == FR_OK) {
        for (;;) {
            res = f_readdir(&dir, &fno);                   /* Read a directory item */
            if (res != FR_OK || fno.fname[0] == 0) break;  /* Break on error or end of dir */
            if (fno.fattrib & AM_DIR) {                    /* It is a directory */
                i = strlen(path);
                sprintf(&path[i], "/%s", fno.fname);
                res = scan_files(path);                    /* Enter the directory */
                if (res != FR_OK) break;
                path[i] = 0;
            } else {      
                strcpy(songlist[index], fno.fname);         /* It is a file. */
                printf("%s/%s\n", path, fno.fname);
                index++;
            }
        }
        maxSong = index;
        f_closedir(&dir);
    }

    return res;
}
