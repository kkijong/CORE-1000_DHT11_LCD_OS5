#include "mbed.h"
#include "TextLCD.h"



//########################################
// LCD Library
//########################################

TextLCD lcd(PC_6,PC_8,PC_5,PC_0,PB_7,PC_13,PB_12);

//########################################
// End of LCD Library
//########################################

#define DHTLIB_OK                0
#define DHTLIB_ERROR_CHECKSUM   -1
#define DHTLIB_ERROR_TIMEOUT    -2

Timer tmr;

DigitalInOut data_pin(PB_10);

int humidity;
int temperature;

//########################################
// DHT11 Library
//########################################
int dht_read(void){
    
    // BUFFER TO RECEIVE
    uint8_t bits[5];
    uint8_t cnt = 7;
    uint8_t idx = 0;
    
    tmr.stop();
    tmr.reset();

    // EMPTY BUFFER
    for(int i=0; i< 5; i++) bits[i] = 0;

    // REQUEST SAMPLE
    data_pin.output();
    data_pin.write(0);
    //wait_ms(18); 
    thread_sleep_for(18); //mbed os5 ver _ ( [wait_ms] can not be used )
    data_pin.write(1);
    wait_us(40);
    data_pin.input();

    // ACKNOWLEDGE or TIMEOUT
    unsigned int loopCnt = 10000;
    
    while(!data_pin.read())if(!loopCnt--)return DHTLIB_ERROR_TIMEOUT;

    loopCnt = 10000;
    
    while(data_pin.read())if(!loopCnt--)return DHTLIB_ERROR_TIMEOUT;

    // READ OUTPUT - 40 BITS => 5 BYTES or TIMEOUT
    for(int i=0; i<40; i++){
        
        loopCnt = 10000;
        
        while(!data_pin.read())if(loopCnt-- == 0)return DHTLIB_ERROR_TIMEOUT;

        //unsigned long t = micros();
        tmr.start();

        loopCnt = 10000;
        
        while(data_pin.read())if(!loopCnt--)return DHTLIB_ERROR_TIMEOUT;

        if(tmr.read_us() > 40) bits[idx] |= (1 << cnt);
        
        tmr.stop();
        tmr.reset();
        
        if(cnt == 0){   // next byte?
        
            cnt = 7;    // restart at MSB
            idx++;      // next byte!
            
        }else cnt--;
        
    }

    // WRITE TO RIGHT VARS
    // as bits[1] and bits[3] are allways zero they are omitted in formulas.
    humidity    = bits[0]; 
    temperature = bits[2]; 

    uint8_t sum = bits[0] + bits[2];  

    if(bits[4] != sum)return DHTLIB_ERROR_CHECKSUM;
    
    return DHTLIB_OK;
    
}

char buffer[17];

//########################################
// End of DHT11 Library
//########################################

int main(void){
    
    lcd.gotoxy(1,1);
    lcd.printf("DHT11");
    //wait_ms(100);
    thread_sleep_for(100); //mbed os5 ver _ ( [wait_ms] can not be used )
    for(;;){
        int fehler=dht_read();
            lcd.gotoxy(1,2);
            lcd.printf("Hum %2d%%  Tmp %2dc  ", humidity, temperature);
            wait_us(500000);
        }
}
