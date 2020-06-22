#include "mbed.h"
#include "bbcar.h"
#include "math.h"
Ticker servo_ticker;
Ticker encoder_ticker1;
Ticker encoder_ticker2;

Timer debounc_sw2;
Timer Turntimer;
Timer leastTimer;

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
Thread xbeeThread(osPriorityHigh);
EventQueue carQueue(32* EVENTS_EVENT_SIZE);
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
void xbeeSend();

void straight();
void turn_left_or_right();
void identify_data_matrix();
void identify_number();

int massege_order = 0;
float posx=0,posy=0,nowtime=0;
int status = 1;
float now_speed = 0;;
char number[6];
char payload[10];
char rotation_char[10];

void xbeeSend(){
    while(1){
        if(status==1){
            xbee.printf("Go straight,       Distance to wall is %f,     Speed is %f#",(float)ping,now_speed);
        }
        else if (status == 2){
            xbee.printf("Straight backward, Distance to wall is %f,     Speed is %f#",(float)ping,now_speed);
        }
        else if (status == 3){
            xbee.printf("Turn left,         Distance to wall is %f,     Speed is %f#",(float)ping,now_speed);
        }
        else if (status == 4){
            xbee.printf("Turn right,        Distance to wall is %f,     Speed is %f#",(float)ping,now_speed);
        }
        else if (status == 5){
            xbee.printf("Stop,              Distance to wall is ,       Speed Is 0#",(float)ping);
        }
        else if (status == 6){
            xbee.printf("identifying data matrix#");
        }
        else if (status == 7){
            xbee.printf("identifying number#");
        }
        else if (status == 8){
            xbee.printf("payload %s ,rotation %s#",payload,rotation_char);
        }
        else if (status == 9){
            xbee.printf("number is %s#",number);
        }
        wait(1);
    }
}
void identify_number(){
    char s[21];
    status = 7;
    sprintf(s,"numberclassification");
    uart.puts(s);
    int i = 0;
    while(1){
        if(uart.readable()){
            char recv = uart.getc();
            pc.printf("%c",recv);
            if(recv!='#'){
                number[i] = recv;
            }
            else{
                number[i] = '\0';
                break;
            }
            i++;            
        }
    }
    status = 9;
    pc.printf("%s\r\n",number);
}
void identify_data_matrix(){
    char s[21];
    status =6;
    sprintf(s,"data_matrix_classify");
    uart.puts(s);
    //float rotation;

    int i = 0;
    while(1){
        if(uart.readable()){
            char recv = uart.getc();
            pc.printf("%c",recv);
            if(recv!='#'){
                payload[i] = recv;
            }
            else{
                payload[i] = '\0';
                break;
            }
            i++;          
        }
    }                
    i = 0;

    while(1){
        if(uart.readable()){
            char recv = uart.getc();
            pc.printf("%c",recv);
            if(recv!='#'){
                rotation_char[i] = recv;
            }
            else{
                rotation_char[i] = '\0';
                break;
            }
            i++;          
        }
    }
    pc.printf("%s , %s\r\n",payload,rotation_char);
    status = 8;
    if(strcmp("fail",payload) == 0){
        pc.printf("fail\r\n");
    }
    else {
        pc.printf("%s , %f\r\n",payload,atof(rotation_char));
    }
}
void straight(float dist_upperbound,float dist_lowerbound,float speed,float least_time){
    now_speed = speed;
    status = (speed>0)?1:2;
    float p[10];
    int p_in_range;   
    for(int i =0;i<10;i++){
        p[i] = 500;
    }
    leastTimer.reset();
    leastTimer.start();
    car.goStraight(speed);
    while(leastTimer.read()<least_time){

    }
    while(1){
        for(int i=0;i<10;i++){
            if(i==9){
                p[i] = (float)ping;
                if(p[i]<dist_upperbound&&p[i]>dist_lowerbound&&p[i]!=500){
                    p_in_range+=1;
                }
            }
            else {
                p[i] = p[i+1];
                if(p[i]<dist_upperbound&&p[i]>dist_lowerbound&&p[i]!=500){
                    p_in_range+=1;
                }
            }
        }
        if(p_in_range == 10){
            car.stop();
            now_speed = 0;
            break;
        }
        p_in_range = 0;
        wait(0.01);
    }
    status = 5;
}
void turn_left_or_right(float dist_upperbound,float dist_lowerbound,float speed,int factor,float least_time){
    status = (factor == 1)?(speed>0)?3:4:(speed>0)?4:3;
    now_speed = abs(speed);
    float p[10];
    int p_in_range;
    for(int i =0;i<10;i++){
        p[i] = 500;
    }
    leastTimer.reset();
    leastTimer.start();
    car.turn(speed,factor);
    while(leastTimer.read()<least_time){

    }
    while(1){
        for(int i=0;i<10;i++){
            if(i==9){
                p[i] = (float)ping;
                if(p[i]<dist_upperbound&&p[i]>dist_lowerbound&&p[i]!=500){
                    p_in_range+=1;
                }
            }
            else {
                p[i] = p[i+1];
                if(p[i]<dist_upperbound&&p[i]>dist_lowerbound&&p[i]!=500){
                    p_in_range+=1;
                }
            }
        }
        if(p_in_range == 10){
            car.stop();
            now_speed = 0;
            break;
        }
        p_in_range = 0;
        wait(0.01);
    }
    status = 5;
}

void first_straight(){
    xbeeQueue.call(xbeeSend);
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
    /*
    float p[10];
    float p_avg = 0;
    */
    Turntimer.reset();
    Turntimer.start();
    car.turn(70,0.1);
    //wait(1.25);
    /*
    while(Turntimer.read()<0.3){
        for(int i=0;i<10;i++){
            if(i==9){
                p[i] = (float)ping;
            }
            else {
                p[i] = p[i+1];
            }
        }
    }
    */
        
    //wait(1.25);
    //led  = 0;
    //car.stop();
    //wait(2);
    //car.turn(150,0.1);
    while(1){
        if((float)ping<40||(float)ping>120){

        }
        else {
            car.stop();
            break;
        }
        wait(0.1);
    }
    led = 1;
    /*
    wait(2);
    car.stop();
    */
    // while(1){
    //     pc.printf("%f\r\n",(float)ping);
    // }
}
void second_straight(){
    encoder_right.reset();
    encoder_left.reset();
    wait(1);
    car.goStraight(50);
    /*
    while(encoder_left.get_cm()<30 || encoder_right.get_cm()<10) {
        wait_ms(50);
        //pc.printf("%f , %f\r\n",encoder_left.get_cm(),encoder_right.get_cm());
    }
    */
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
    xbeeQueue.call(xbeeSend);
    straight(25,20,50,10);
    wait(2);


    identify_data_matrix();
    wait(2);

    
    turn_left_or_right(25,20,50,1,3);
    wait(2);
    straight(40,35,50,5);
    wait(2);
    turn_left_or_right(25,20,-50,1,3);
    wait(2);


    identify_number();
    wait(2);


    straight(50,40,-50,3);
    wait(2);


    identify_number();
    wait(2);
    

    straight(25,20,50,3);
    wait(2);
    turn_left_or_right(25,20,50,-1,3);//75 65
    wait(2);
    straight(25,20,50,3);
    wait(2);
    turn_left_or_right(25,20,50,-1,3);//75 65
    wait(2);
    straight(25,20,50,10);
/*
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
*/
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
    printf("a");
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
    pc.printf("start\r\n");

     /*--------------------------------------------------------------*/
    
    
    led = 1;
    debounc_sw2.reset();
    debounc_sw2.start();
    uart.baud(9600);
    carThread.start(callback(&carQueue,&EventQueue::dispatch_forever));
    xbeeThread.start(callback(&xbeeQueue,&EventQueue::dispatch_forever));
    sw2.rise(call_Car_Go);
    
}