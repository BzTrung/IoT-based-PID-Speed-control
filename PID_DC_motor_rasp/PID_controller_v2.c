#include <wiringPi.h>
#include <wiringPiSPI.h>
#include <stdio.h>
#include <stdint.h>
#include <math.h> // -lm
#include "unistd.h"
#include "string.h"
#include <stdlib.h>
#include <softPwm.h>
#include "mqtt_pub_sub.h"
#include <stdbool.h>

#define channel 0

#define IN1 11
#define IN2 13
#define ENA 12
#define EncoderA 16
#define EncoderB 18
#define BT_Up 37
#define BT_Down 35
#define BT_Stop 33
#define BT_MODE 31

#define CPR 220
#define clock 7
#define range 255
#define sampleT 100

uint32_t prevT = 0;
int prevPos = 0;
volatile int Pos_i = 0;
float Vel_f = 0;
float Vel_prev = 0;
float Setpoint ;
float err_windup = 0;
uint8_t flag = 1;
float Kp,Ki,Kd,Kb,Up,Ui,Ud,Ud_f,PID_term,Ud_f_p,alpha,err_p;
bool mode = 1;

void readEncoder(void)
{
    if(flag == 1)
    {
        int8_t b = digitalRead(EncoderB);
        if(b==1)
        {
            Pos_i++;
        }
        else
        {
            Pos_i--;
        }
    }
}

void PWMOutput(int dir, int Pwm)
{
    pwmWrite(ENA,Pwm);
    if(dir==1)
    {
        digitalWrite(IN1,HIGH);
        digitalWrite(IN2,LOW);
    }
    else if(dir == -1)
    {
        digitalWrite(IN1,LOW);
        digitalWrite(IN2,HIGH);
    }
    else
    {
        digitalWrite(IN1,LOW);
        digitalWrite(IN2,LOW);
    }
}


void Button(void)
{
    if(digitalRead(BT_Up))
    {
        Setpoint+=10;
        if(Setpoint >160) Setpoint = 160;
    }
    else if(digitalRead(BT_Down))
    {
        Setpoint-=10;
        if(Setpoint < -160) Setpoint = -160;
    }
    else if(digitalRead(BT_Stop))
    {
        Setpoint = 0;
    }
    else if(digitalRead(BT_MODE))
    {
        mode = !mode;
    }
}

void send_data(uint8_t address, uint8_t value)
{
    unsigned char data[2];
    data[0]= address;
    data[1]= value;
    wiringPiSPIDataRW(channel,data,2);
}

void Init_max7219(void)
{
    //set decode mode
    send_data(0x09,0x3F);
    //set intensity
    send_data(0x0A,0x03);
    //set scan limit
    send_data(0x0B,0x07);
    //no shuttdown
    send_data(0x0C,1);
    //display test off
    send_data(0x0f,0);
}

void LED_ouput(float x)
{
    
    char string[8];
    sprintf(string,"%0.2f",x);
    int j = 8-strlen(string)-1;

    for(int i=0; i<j;i++)
    {
        send_data(i+1,0x0f);
    }
        
    for(int i=0;i<strlen(string);i++)
    {
        if(string[strlen(string)-1-i]=='-')
        {
            send_data(j+1,10);
        }
        else if(string[strlen(string)-1-i]=='.')
        {
            string[strlen(string)-1-i-1] =  string[strlen(string)-1-i-1] + 0x80;
            j--;
        }
        else
        send_data(j+1,(int)string[strlen(string)-1-i]);
        j++;
    }
    while(j<6)
    {
        send_data(j+1,0x0f);
        j++;
    }
    send_data(7,0x09);
}




/* PI_THREAD(myThread2)
{
    
    while(1)
    {
        uint32_t currT = millis();
        
        float deltaT = ((float)(currT - prevT))/1.0e3;
        if(deltaT>=sampleT/1.0e3)
        {
            //read vel
            int Pos = 0;
            flag = 0;
            Pos = Pos_i;
            flag = 1;
            float Vel = ((Pos - prevPos)/deltaT)/CPR*60.0;
            prevPos = Pos;
            prevT = currT;

            // vel fillter
            Vel_f = 0.854*Vel_f + 0.0728*Vel + 0.0728*Vel_prev;
            Vel_prev = Vel;
            
            printf("%.2f\n", Vel_f);
            PWMOutput(1,255);
        }
        
    }
    return 0;
} */

PI_THREAD (LED_7_SEG)
{  
    wiringPiSPISetup(channel, 8000000);
    Init_max7219();
    while(1)
    {
        while(mode == 0)
        {
            send_data(8,0x67);
            LED_ouput(Kp);
            delay(1000);
            send_data(8,0x06);
            LED_ouput(Ki);
            delay(1000);
            send_data(8,0x3D);
            LED_ouput(Kd);
            delay(1000);
            send_data(8,0x5B);
            LED_ouput(Vel_f);
            delay(1000); 
        }
        while(mode == 1)
        {
            send_data(8,0x5B);
            LED_ouput(Vel_f);
            delay(100); 
        }
    }
    return 0;
	
}


PI_THREAD (PID_Compute)
{  
    uint32_t last_check;
    while(1)
    {
            
        uint32_t currT = millis();
        
        float deltaT = ((float)(currT - prevT))/1.0e3;
        if(deltaT>=sampleT/1.0e3)
        {
            //read vel
            int Pos = 0;
            flag = 0;
            Pos = Pos_i;
            flag = 1;
            float Vel = ((Pos - prevPos)/deltaT)/CPR*60.0;
            prevPos = Pos;
            prevT = currT;

            // vel fillter
            Vel_f = 0.854*Vel_f + 0.0728*Vel + 0.0728*Vel_prev;
            Vel_prev = Vel;
            
            //PID compute
            float err = Setpoint - Vel_f;
            Up = Kp*err;
            Ud = Kd*(err-err_p)/deltaT;
            Ud_f = alpha*Ud + (1-alpha)*Ud_f_p;
            Ui += Ki*err*deltaT;
            err_p = err;
            Ud_f_p = Ud_f;

            PID_term  = Up + Ui + Ud_f;
            
            if (PID_term > range) {
            Ui -= PID_term - range;
            PID_term = range;
            } else if (PID_term < -range) {
            Ui += -range - PID_term;
            PID_term = -range;
            }

            int dir  = 1;
            if (PID_term<0)
            {
                dir = -1;
            }
            PWMOutput(dir,(int)fabs(PID_term));

            /*printf("Set: %.2f\n", Setpoint);
            printf("Vel: %.2f\n", Vel_f);
            printf("dir: %d\n", dir);
            printf("PWM out: %d\n", (int)fabs(PID_term));
            printf("Kp: %.2f\n", Kp);*/

        }
      
        
    }
    return 0;
	
}

int on_message(void *context, char *topicName, int topicLen, MQTTClient_message *message) {
    char* payload = message->payload;
    printf("Received message: %s\n", payload);
    splitdata(payload,&PID_sub);
    Kp = PID_sub.Kp;
    Ki = PID_sub.Ki;
    Kd = PID_sub.Kd;
    Setpoint = PID_sub.Setpoint;
    MQTTClient_freeMessage(&message);
    MQTTClient_free(topicName);
    return 1;
}

int main(int argc, char* argv[])
{
    wiringPiSetupPhys();
    pinMode(IN1, OUTPUT); pinMode(IN2, OUTPUT);
    pinMode(EncoderA, INPUT); pinMode(EncoderB, INPUT);
    pinMode(BT_Up, INPUT); pinMode(BT_Down, INPUT); pinMode(BT_Stop, INPUT); pinMode(BT_MODE,INPUT);
    wiringPiISR(EncoderA,INT_EDGE_RISING,&readEncoder);
    wiringPiISR(BT_Up,INT_EDGE_RISING,&Button);
    wiringPiISR(BT_Down,INT_EDGE_RISING,&Button);
    wiringPiISR(BT_Stop,INT_EDGE_RISING,&Button);
    wiringPiISR(BT_MODE,INT_EDGE_RISING,&Button);
    //setup pwm0 ms_mode clock 7 range 255 ~10kHz
    pinMode(ENA,PWM_OUTPUT); pwmSetMode(PWM_MODE_MS);
    pwmSetClock(clock); pwmSetRange(range);
    PWMOutput(1,0);
    Kp = 0.8; Ki=1.17; Kd = 0; alpha = 0.9; Setpoint = 0; 

    MQTTClient client;
    MQTTClient_create(&client, ADDRESS, CLIENTID, MQTTCLIENT_PERSISTENCE_NONE, NULL);
    MQTTClient_connectOptions conn_opts = MQTTClient_connectOptions_initializer;
	MQTTClient_setCallbacks(client, NULL, NULL, on_message, NULL);
    
    int rc;
    if ((rc = MQTTClient_connect(client, &conn_opts)) != MQTTCLIENT_SUCCESS) {
        printf("Failed to connect, return code %d\n", rc);
        exit(-1);
    }
    //listen for operation
    MQTTClient_subscribe(client, SUB_TOPIC, 0);
    piThreadCreate (PID_Compute);
    piThreadCreate(LED_7_SEG);
    while(1)
    {
        char msg [20];
        sprintf(msg,"{\"Vel\":%0.2f,\"PWMoutput\":%d,\"Setpoint\":%0.2f,\"Kp\":%0.2f,\"Ki\":%0.2f,\"Kd\":%0.2f}",Vel_f,(int)fabs(PID_term),Setpoint,Kp,Ki,Kd);
        publish(client, PUB_TOPIC, msg);
        delay(100);
    }
    MQTTClient_disconnect(client, 1000);
    MQTTClient_destroy(&client);
    return rc;
}
    