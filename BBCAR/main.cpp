#include "mbed.h"
#include "bbcar.h"
Ticker servo_ticker;
Ticker encoder_ticker1;
Ticker encoder_ticker2;

Timer debounc_sw2;
Timer Turntimer;

Serial pc(USBTX,USBRX); //tx,rx
Serial xbee(D12, D11);
PwmOut pin8(D8);
PwmOut pin9(D9);
DigitalIn pin3(D3);
DigitalIn pin4(D4);
DigitalInOut pin10(D10); //ping
BBCar car(pin9, pin8, servo_ticker);
parallax_encoder encoder_right(pin3, encoder_ticker1);
parallax_encoder encoder_left (pin4, encoder_ticker2);
parallax_ping ping(pin10);

InterruptIn sw2(SW2);
Serial uart(D1,D0); //tx,rx
DigitalInOut led(LED3);
Thread carThread(osPriorityNormal);
Thread sendThread(osPriorityNormal);
Thread xbeeThread(osPriorityNormal);
EventQueue carQueue(32* EVENTS_EVENT_SIZE);
EventQueue sendQueue(32* EVENTS_EVENT_SIZE);
EventQueue xbeeQueue(32* EVENTS_EVENT_SIZE);
void Car_Go();
void call_Car_Go();
void first_straight();
void first_turn();
void second_straight();
void back_turn();
void first_identify_number();
void back_to_lot();
void second_identify_number();
void third_straight();
void turn2();
void straght4();
void turn3();
void straght5();
void turn4();
void send_massege();
void xbeeSend();

int massege_order = 0;
float posx=0,posy=0,nowtime=0;
void xbeeSend(){
    char outbuf[4];
    char testbuf[3];
    float x,y,t;
    testbuf[0] = 1;
    testbuf[1] = 1;
    testbuf[2] = 1;

    while(1){
       // pc.printf("aa");
        x = posx;
        y = posy;
        t = nowtime;
        sprintf(outbuf,"%1.1f",x);
        xbee.printf("%s",testbuf);
        sprintf(outbuf,"%1.1f",y);
        xbee.printf("%s",testbuf);
        sprintf(outbuf,"%1.1f",t);
        xbee.printf("%s",testbuf);
        wait(1);   
    }
}
void send_massege(){
    char s[20];
    if(massege_order == 0){
        sprintf(s,"data_matrix_classify");
        uart.puts(s);
        wait(0.5);
        massege_order++;
    }
    else if(massege_order == 1){
        sprintf(s,"numberclassification");
        uart.puts(s);
        wait(0.5);
        massege_order++;
    }
    else if(massege_order == 2){
        sprintf(s,"numberclassification");
        uart.puts(s);
        wait(0.5);
        massege_order++;
    }
}

void first_straight(){
    encoder_right.reset();
    encoder_left.reset();
    car.goStraight(50);
    float p[10];
    float p_avg;
    for(int i=0;i<10;i++){
        p[i]=500;
    }
    while(encoder_left.get_cm()<60 || encoder_right.get_cm()<60) {
        wait_ms(50);
        for(int i=0;i<10;i++){
            if(i==9){
                p[i] = (float)ping;
            }
            else {
                p[i] = p[i+1];
            }
        }
    }
    while(1){
        for(int i=0;i<10;i++){
            if(i==9){
                p[i] = (float)ping;
                if(p[i]>20){
                    p_avg+=1;
                }
            }
            else {
                p[i] = p[i+1];
                if(p[i]>20){
                    p_avg+=1;
                }
            }
        }
        if((float)p_avg>7){
        }
        else{
            car.stop();
            break;
        }
        p_avg = 0;
        wait(0.01);
    }

    led = 1;
}
void first_turn(){
    encoder_right.reset();
    encoder_left.reset();
    wait(1);
    float p[10];
    float p_avg = 0;
    Turntimer.reset();
    Turntimer.start();
    car.turn(50,0.1);
    while(Turntimer.read()<1){
        for(int i=0;i<10;i++){
            if(i==9){
                p[i] = (float)ping;
            }
            else {
                p[i] = p[i+1];
            }
        }
    }
        
    //wait(1.25);
    //led  = 0;
    car.stop();
    wait(2);
    car.turn(50,0.1);
    
    while(1){
        for(int i=0;i<10;i++){
            if(i==9){
                p[i] = (float)ping;
                if(p[i]<40||p[i]>120){
                    p_avg+=1;
                }
            }
            else {
                p[i] = p[i+1];
                if(p[i]<40||p[i]>120){
                    p_avg+=1;
                }
            }
        }
        if(p_avg>8){
        }
        else{
            car.stop();
            break;
        }
        p_avg = 0;
        wait(0.01);
    }
    
    led = 1;
    /*
    wait(2);
    car.stop();
    */
}
void second_straight(){
    encoder_right.reset();
    encoder_left.reset();
    wait(1);
    car.goStraight(80);
    while(encoder_left.get_cm()<30 || encoder_right.get_cm()<10) {
        wait_ms(50);
        //pc.printf("%f , %f\r\n",encoder_left.get_cm(),encoder_right.get_cm());
    }
    while(1){
        if((float)ping>50){
        }
        else{
            car.stop();
            break;
        }
        wait(0.1);
    }
}
void back_turn(){
    encoder_right.reset();
    encoder_left.reset();
    wait(1);
    car.turn(-80,0.1);
    wait(1.1);
    car.stop();
    /*
    Turntimer.reset();
    Turntimer.start();
    float timer_read;
    while(1){
        if((float)ping>25){
        }
        else{
            car.stop();
            timer_read = Turntimer.read();
            break;
        }
        wait(0.1);
    }
    */
    /*
    wait(1);
    car.turn(-150,0.1);
    wait(timer_read);
    car.stop();
    /*
    while(1){
        pc.printf("%f\r\n",(float)ping);
    }
    */
}
void first_identify_number(){  
    char s[21];
    sprintf(s,"numberclassification");
    uart.puts(s);
    //pc.printf("send number classification\r\n");
    wait(0.5);
}
void back_to_lot(){
    encoder_right.reset();
    encoder_left.reset();
    wait(1);
    car.goStraight(-100);
    while(1){
        if((float)ping<40){
        }
        else{
            car.stop();
            break;
        }
        wait(0.1);
    }
    car.stop();
}
void second_identify_number(){
    char s[21];
    sprintf(s,"numberclassification");
    uart.puts(s);
    //pc.printf("send number classification\r\n");
    wait(0.5);
}
void third_straight(){
    encoder_right.reset();
    encoder_left.reset();
    wait(1);
    car.goStraight(100);
    while(1){
        if((float)ping>25){
        }
        else{
            car.stop();
            break;
        }
        wait(0.1);
    }
}
void turn2(){
    encoder_right.reset();
    encoder_left.reset();
    wait(1);
    car.turn(150,-0.1);
    wait(1.5);
    led  = 0;
    while(1){
        if((float)ping<40||(float)ping>100){
        }
        else{
            car.stop();
            break;
        }
        wait(0.1);
    }
    led = 1;
}
void straght4(){
    encoder_right.reset();
    encoder_left.reset();
    wait(1);
    car.goStraight(100);
    while(1){
        if((float)ping>40){
        }
        else{
            car.stop();
            break;
        }
        wait(0.1);
    }
}
void turn3(){
    encoder_right.reset();
    encoder_left.reset();
    wait(1);
    car.turn(150,-0.1);
    wait(1.5);
    led  = 0;
    while(1){
        if((float)ping<60||(float)ping>110){
        }
        else{
            car.stop();
            break;
        }
        wait(0.1);
    }
    led = 1;
}
void straght5(){
    encoder_right.reset();
    encoder_left.reset();
    wait(1);
    car.goStraight(100);
    while(1){
        if((float)ping>25){
        }
        else{
            car.stop();
            break;
        }
        wait(0.1);
    }
}
void turn4(){
    encoder_right.reset();
    encoder_left.reset();
    wait(1);
    car.turn(150,-0.1);
    wait(1.5);
    led  = 0;
    while(1){
        if((float)ping<40||(float)ping>100){
        }
        else{
            car.stop();
            break;
        }
        wait(0.1);
    }
    led = 1;
}
void call_Car_Go(){
    if(debounc_sw2.read()>1){
      carQueue.call(Car_Go);
      debounc_sw2.reset();
      debounc_sw2.start();
   }
}

void Car_Go(){
first_straight();
first_turn();
second_straight();
back_turn();
first_identify_number();
back_to_lot();
second_identify_number();
third_straight();
turn2();
straght4();
turn3();
straght5();
turn4();
}

void reply_messange(char *xbee_reply, char *messange)
{
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    xbee_reply[2] = xbee.getc();
    if (xbee_reply[1] == 'O' && xbee_reply[2] == 'K')
    {
        //pc.printf("%s\r\n", messange);
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
        xbee_reply[2] = '\0';
    }
}
void check_addr(char *xbee_reply, char *messenger)
{
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    xbee_reply[2] = xbee.getc();
    xbee_reply[3] = xbee.getc();
    //pc.printf("%s = %c%c%c\r\n", messenger, xbee_reply[1], xbee_reply[2], xbee_reply[3]);
    xbee_reply[0] = '\0';
    xbee_reply[1] = '\0';
    xbee_reply[2] = '\0';
    xbee_reply[3] = '\0';
}

int main() {
    
    /*--------------------------------------------------------------*/
    /*
        char xbee_reply[4];
    // XBee setting
    xbee.baud(9600);
    xbee.printf("+++");
    xbee_reply[0] = xbee.getc();
    xbee_reply[1] = xbee.getc();
    if (xbee_reply[0] == 'O' && xbee_reply[1] == 'K')
    {
        pc.printf("enter AT mode.\r\n");
        xbee_reply[0] = '\0';
        xbee_reply[1] = '\0';
    }
    xbee.printf("ATMY 0x265\r\n");
    reply_messange(xbee_reply, "setting MY : 0x265");
    xbee.printf("ATDL 0x165\r\n");
    reply_messange(xbee_reply, "setting DL : 0x165");
    xbee.printf("ATCN\r\n");
    reply_messange(xbee_reply, "exit AT mode");
    pc.printf("start\r\n");*/

     /*--------------------------------------------------------------*/
    
    
    led = 1;
    debounc_sw2.reset();
    debounc_sw2.start();
    uart.baud(9600);
    carThread.start(callback(&carQueue,&EventQueue::dispatch_forever));
    sendThread.start(callback(&sendQueue,&EventQueue::dispatch_forever));
    xbeeThread.start(callback(&xbeeQueue,&EventQueue::dispatch_forever));
    sw2.rise(call_Car_Go);
    //xbeeQueue.call(xbeeSend);
}