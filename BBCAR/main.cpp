#include "mbed.h"
#include "bbcar.h"
#include "math.h"
Ticker servo_ticker;
Ticker encoder_ticker1;
Ticker encoder_ticker2;

Timer debounc_sw2;
Timer debounc_sw3;
Timer Turntimer;
Timer leastTimer;
Timer datamatrixTimer;

Serial pc(USBTX,USBRX); //tx,rx
Serial xbee(D12, D11);
PwmOut pin8(D8);
PwmOut pin9(D9);
DigitalIn pin3(D3);
DigitalIn pin4(D4);
DigitalOut ledRED(D7);
DigitalInOut pin10(D10); //ping
BBCar car(pin9, pin8, servo_ticker);
parallax_encoder encoder_right(pin3, encoder_ticker1);
parallax_encoder encoder_left (pin4, encoder_ticker2);
parallax_ping ping(pin10);

InterruptIn sw2(SW2);
InterruptIn sw3(SW3);
Serial uart(D1,D0); //tx,rx
DigitalOut led(LED3);
Thread endThread(osPriorityNormal);
Thread carThread(osPriorityHigh);
Thread xbeeThread(osPriorityHigh2);
Thread pingThread(osPriorityHigh1);
Thread LEDThread(osPriorityHigh);
EventQueue carQueue(32* EVENTS_EVENT_SIZE);
EventQueue xbeeQueue(32* EVENTS_EVENT_SIZE);
EventQueue endQueue(32* EVENTS_EVENT_SIZE);
EventQueue pingQueue(32* EVENTS_EVENT_SIZE);
EventQueue LEDQueue(32* EVENTS_EVENT_SIZE);
void Car_Go();
void call_Car_Go();

void xbeeSend();

void straight(float dist_upperbound,float dist_lowerbound,float speed,float least_time,int ping_use);
void straight2(float speed,float least_time,float distance,int ping_use);
void turn_left_or_right(float dist_upperbound,float dist_lowerbound,float speed,int factor,float least_time,int ping_use);
void identify_data_matrix();
void identify_number();
void identify_object(int t);
void cal_result();
void end_car();
void call_end_car();
void ping_function();
void led_function();
void encoder_turn(float speed ,int left,int step);

int status = 5;
float now_speed = 0;
float distance_1;
float distance_2;
float distance_3;
float now_ping = 0;
int led_mode = 2;
char number[6];
char payload[10];
char rotation_char[10];
char object[50];

void xbeeSend(){
    while(1){
        if(status==1){
            xbee.printf("Go straight,       Distance to wall is %f,     Speed is %f#",now_ping,now_speed);
        }
        else if (status == 2){
            xbee.printf("Straight backward, Distance to wall is %f,     Speed is %f#",now_ping,now_speed);
        }
        else if (status == 3){
            xbee.printf("Turn left,         Distance to wall is %f,     Speed is %f#",now_ping,now_speed);
        }
        else if (status == 4){
            xbee.printf("Turn right,        Distance to wall is %f,     Speed is %f#",now_ping,now_speed);
        }
        else if (status == 5){
            xbee.printf("Stop,              Distance to wall is %f,     Speed Is 0#",now_ping);
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
        else if (status == 10){
            xbee.printf("finish#");
        }
        else if (status == 11){
            xbee.printf("Object is %s#",object);
        }
        else if (status == 12){
            xbee.printf("Identify object distance_1 is %f#",distance_1);
        }
        else if (status == 13){
            xbee.printf("back left,         Distance to wall is %f,     Speed is %f#",now_ping,now_speed);
        }
        else if (status == 14){
            xbee.printf("back right,        Distance to wall is %f,     Speed is %f#",now_ping,now_speed);
        }
        else if (status == 15){
            xbee.printf("Identify object distance_2 is %f#",distance_2);
        }
        else if (status == 16){
            xbee.printf("Identify object distance_3 is %f#",distance_3);
        }
        wait(1);
    }
}
void encoder_turn(float speed ,int left,int step,int mode,float t){
    led_mode = 1;
    LEDQueue.call(led_function);
    //status = (left == 1)?3:4;
    status = (left == 1)?(speed>0)?3:13:(speed>0)?4:14;
    wait(0.05);
    now_speed = abs(speed);
    encoder_right.reset();
    encoder_left.reset();
    Turntimer.reset();
    Turntimer.start();
    if(left == 1){
        car.turn(speed,1);
        if(mode==0){
            while(1){
                if(encoder_right.get_steps()>=step){
                    break;
                }
                wait(0.05);
            }
        }
        else if (mode == 1){
            while(1){
                if(Turntimer.read()>t){
                    break;
                }
                wait(0.05);
            }
        }  
    }
    else {
        car.turn(speed,-1);
        if(mode==0){
            while(1){
                if(encoder_left.get_steps()>=step){
                    break;
                }
                wait(0.05);
            }
        }
        else if (mode == 1){
            while(1){
                if(Turntimer.read()>t){
                    break;
                }
                wait(0.05);
            }
        }
    }
    car.stop();
    led_mode = 2;
    LEDQueue.call(led_function);
    status = 5;
    wait(0.05);
    
}
void led_function(){
    if(led_mode == 1){
        ledRED = 1;
    }
    else if (led_mode == 2){
        ledRED = 0;
    }
    else if(led_mode == 3){
        while(led_mode == 3){
            ledRED = 1;
            wait(0.3);
            ledRED = 0;
            wait(0.3);
        }
    }
}
void ping_function(){
    while(1){
        now_ping = (float)ping;
        wait(0.05);
    }
}
void cal_result(){
    if(distance_1<20){
        sprintf(object,"Square");
    }
    else{
        if(distance_2-distance_3<3||distance_3-distance_2<3){
            if(distance_2>30||distance_3>30){
                sprintf(object,"square minus regular triangle");
            }
            else {
                sprintf(object,"Regular triangle");
            }
        }
        else{
            sprintf(object,"Right triangle");
        }
    }
    wait(0.2);
    status = 11;
    wait(0.05);
    
}
void identify_object(int t){
    if(t == 1){
        distance_1 = now_ping;
        wait(0.2);
        status = 12;
        wait(0.05);
    }
    else if(t == 2){
        distance_2 = now_ping;
        wait(0.2);
        status = 15;
        wait(0.05);
    }
    else if(t == 3){
        distance_3 = now_ping;
        wait(0.2);
        status = 16;
        wait(0.05);
    }
}
void identify_number(){
    led_mode = 3;
    wait(0.05);
    LEDQueue.call(led_function);
    char s[21];
    status = 7;
    sprintf(s,"numberclassification");
    uart.puts(s);
    int i = 0;
    datamatrixTimer.reset();
    datamatrixTimer.start();
    while(1){
        if(datamatrixTimer.read()>=3){
            break;
        }
        if(uart.readable()){
            char recv = uart.getc();
            //pc.printf("%c",recv);
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
    wait(0.05);
    led_mode = 2;
    LEDQueue.call(led_function);
    //pc.printf("%s\r\n",number);
}
void identify_data_matrix(){
    led_mode = 3;
    wait(0.05);
    LEDQueue.call(led_function);
    char s[21];
    status =6;
    sprintf(s,"data_matrix_classify");
    uart.puts(s);
    //float rotation;
    datamatrixTimer.reset();
    datamatrixTimer.start();
    int i = 0;
    while(1){
        if(datamatrixTimer.read()>=7){
            payload[0] = 'f';
            payload[1] = 'a';
            payload[2] = 'i';
            payload[3] = 'l';
            payload[4] = '\0';
            break;
        }
        if(uart.readable()){
            char recv = uart.getc();
            //pc.printf("%c",recv);
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
    datamatrixTimer.reset();
    datamatrixTimer.start();
    while(1){
        if(datamatrixTimer.read()>=7){
            rotation_char[0] = 'f';
            rotation_char[1] = 'a';
            rotation_char[2] = 'i';
            rotation_char[3] = 'l';
            rotation_char[4] = '\0';
            break;
        }
        if(uart.readable()){
            char recv = uart.getc();
            //pc.printf("%c",recv);
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
    //pc.printf("%s , %s\r\n",payload,rotation_char);
    led_mode = 2;
    LEDQueue.call(led_function);
    status = 8;
    wait(0.05);
    /*
    if(strcmp("fail",payload) == 0){
        //pc.printf("fail\r\n");
    }
    else {
        //pc.printf("%s , %f\r\n",payload,atof(rotation_char));
    }*/
}
void straight2(float speed,float least_time,float distance,int ping_use,int step){
    led_mode = 1;
    wait(0.05);
    LEDQueue.call(led_function);
    now_speed = speed;
    status = (speed>0)?1:2;
    leastTimer.reset();
    leastTimer.start();
    encoder_left.reset();
    encoder_right.reset();
    car.goStraight(speed);
    if(ping_use == 0){
        while(1){
            if(leastTimer.read()>least_time){
                break;
            }
            wait(0.01);
        }
    }
    else if (ping_use == 1){
        if(speed>0){
            while(1){
                if(now_ping<distance){
                    break;
                }
                wait(0.05);
            }
        }
        else if(speed<0){
            while(1){
                if(now_ping>distance){
                    break;
                }
                wait(0.05);
            }
        }
    }
    else if (ping_use ==2){
        while(1){
            if(encoder_right.get_steps()>=step){
                break;
            }
            wait(0.05);
        }
    }
    car.stop();
    led_mode = 2;
    LEDQueue.call(led_function);
    status = 5;
    wait(0.05);
}
void straight(float dist_upperbound,float dist_lowerbound,float speed,float least_time,int ping_use){
    led_mode = 1;
    LEDQueue.call(led_function);

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
        wait(0.01);
    }
    if(ping_use == 1){
        while(1){
            for(int i=0;i<10;i++){
                if(i==9){
                    //p[i] = (float)ping;
                    p[i] = now_ping;
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
    }
    else if(ping_use==0){
        car.stop();
    }
    led_mode = 2;
    LEDQueue.call(led_function);
    status = 5;
}
void turn_left_or_right(float dist_upperbound,float dist_lowerbound,float speed,int factor,float least_time,int ping_use){
    led_mode = 1;
    LEDQueue.call(led_function);
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
        wait(0.01);
    }
    if(ping_use==1){
        while(1){
            for(int i=0;i<10;i++){
                if(i==9){
                    //p[i] = (float)ping;
                    p[i] = now_ping;
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
    }
    else if(ping_use==0){
        car.stop();
    }
    led_mode = 2;
    LEDQueue.call(led_function);
    status = 5;
}



void call_end_car(){
    if(debounc_sw3.read()>1){
      endQueue.call(end_car);
      debounc_sw3.reset();
      debounc_sw3.start();
   }
}
void call_Car_Go(){
    if(debounc_sw2.read()>1){
      carQueue.call(Car_Go);
      debounc_sw2.reset();
      debounc_sw2.start();
   }
}
void end_car(){
    status = 10;
}
void Car_Go(){
    
    straight2(150,0,25,1,0);
    wait(0.5);
    //identify_data_matrix();
    //wait(1.2);
    encoder_turn(70,1,21,0,0);
    wait(0.5);
    //-----------------------------------------------mission 1-----------------------------------------
    straight2(70,0,25,1,0);
    wait(1);
    straight2(-70,0,0,2,30);
    wait(0.5);
    encoder_turn(-70,1,22,0,0);
    wait(0.5);
    straight2(-150,0,40,1,0);
    wait(0.5);
    identify_number();
    wait(1.2);
    straight2(150,0,25,1,0);
    wait(0.5);
    encoder_turn(70,-1,24,0,0);
    wait(0.5);
    straight2(150,0,0,2,32);
    wait(0.5);
    //-----------------------------------------------mission 1-----------------------------------------
    encoder_turn(70,-1,21,0,0);
    wait(0.5);
    straight2(150,0,25,1,0);
    wait(0.5);
    encoder_turn(70,-1,25,0,0);
    //-----------------------------------------------mission 2-----------------------------------------
    straight2(150,0,0,2,47);
    wait(0.5);
    encoder_turn(70,-1,25,0,0);
    wait(0.5);
    straight2(150,0,0,2,30);
    //identify_data_matrix();
    //wait(1.2);
    identify_object(1);
    wait(1.2);
    encoder_turn(-70,-1,6,0,0);
    identify_object(2);
    wait(1.2);
    encoder_turn(70,-1,6,0,0);
    encoder_turn(-70,1,6,0,0);
    identify_object(3);
    wait(1.2);
    encoder_turn(70,1,6,0,0);
    cal_result();
    wait(1.2);
    straight2(-150,0,0,2,30);
    wait(0.5);
    encoder_turn(-70,-1,22,0,0);
    wait(0.5);
    straight2(150,0,25,1,0);
    wait(0.5);
    
    //-----------------------------------------------mission 2-----------------------------------------
    encoder_turn(70,-1,25,0,0);
    wait(0.5);
    straight2(150,0,0,2,300);
    wait(0.5);
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
    number[0] = '0';
    number[1] = '\0';
    led = 1;
/*--------------------------------------------------------------*/
    //printf("a");
    /*
    xbee.baud(9600);
    
    char xbee_reply[4];
    // XBee setting
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
    */
/*--------------------------------------------------------------*/
    debounc_sw2.reset();
    debounc_sw2.start();
    debounc_sw3.reset();
    debounc_sw3.start();
    uart.baud(9600);
    carThread.start(callback(&carQueue,&EventQueue::dispatch_forever));
    xbeeThread.start(callback(&xbeeQueue,&EventQueue::dispatch_forever));
    endThread.start(callback(&endQueue,&EventQueue::dispatch_forever));
    pingThread.start(callback(&pingQueue,&EventQueue::dispatch_forever));
    LEDThread.start(callback(&LEDQueue,&EventQueue::dispatch_forever));
    sw2.rise(call_Car_Go);
    sw3.rise(call_end_car);
    pingQueue.call(ping_function);

    led_mode = 2;
    LEDQueue.call(led_function);
    xbeeQueue.call(xbeeSend);

    
    
}