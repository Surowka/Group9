#include <project.h>
#include <stdio.h>
#include "Motor.h"
#include "Ultra.h"
#include "Nunchuk.h"
#include "Reflectance.h"
#include "I2C_made.h"
#include "Gyro.h"
#include "Accel_magnet.h"
#include "IR.h"
#include "Ambient.h"
#include "Beep.h"

int rread(void);

//battery level//
/*
int main()
{
    CyGlobalIntEnable; 
    UART_1_Start();
    ADC_Battery_Start();        
    int16 adcresult = 0;
    float volts = 0.0;

    printf("\nBoot\n");

    //BatteryLed_Write(1); // Switch led on 
    BatteryLed_Write(0); // Switch led off 
    //uint8 button;
    //button = SW1_Read(); // read SW1 on pSoC board
    for(;;)
    {        
        ADC_Battery_StartConvert();
        if(ADC_Battery_IsEndConversion(ADC_Battery_WAIT_FOR_RESULT)) {   // wait for get ADC converted value
            adcresult = ADC_Battery_GetResult16();
            volts = ADC_Battery_CountsTo_Volts(adcresult);               // convert value to Volts
        
            // If you want to print value
            printf("%d %f\r\n",adcresult, volts);
        }
        CyDelay(500);       
    }
 }   
*/


//reflectance//
int main()
{
    struct sensors_ ref;
    struct sensors_ dig;
    CyGlobalIntEnable; 
    UART_1_Start();
    
  
    sensor_isr_StartEx(sensor_isr_handler);
    CyDelay(1000);  
    motor_start();    
    reflectance_start();
    
    int const Sensor_max = 23999, time = 1, left_max = Sensor_max - 5000, right_max = Sensor_max - 5000;
    int speedl, speedr, error_left, error_right, max_speed = 255, last_er=0, last_el=0;
    int const kp = max_speed + 85 ,kd = kp/5;
    int IR_val = 0;
    int flag = 0;
    int state = 0; 
    int temp = 4;
    int inside = 0; //0 = not in the ring, else = in the ring;
    // state = what we are doing; Temp = what we need to do;
    IR_led_Write(1);
    for(;;)
    {
        reflectance_read(&ref);
        reflectance_digital(&dig);
        //stopping condition check
        if (dig.l3 == 0 && dig.l1 == 0 && dig.r1 == 0 && dig.r3 == 0) {
            temp = 0;  
        } else temp = 4;
        // if what we have to do  != what we are doing, reset
        if (state != temp) {        
            if (state == 0) flag++;
            state = temp;
        }
        // Stopping at black lines
        if (state == 0 && (flag == 1)) {           
            motor_stop();
            do {
                IR_val = get_IR();
            } while (!IR_val);
            motor_start();
// cut here to change from line to sumo and vice versa --------------------------
            motor_forward(max_speed,200);
        } else 
        // stopping at finish line
        if (state == 0 && (flag == 3)) {
            motor_stop();
        } else {
            error_left = (Sensor_max - ref.l1);
            error_right = (Sensor_max - ref.r1);
            speedr = max_speed 
                     - (kp* error_left)/left_max 
                     + kd*(error_left - last_el)/left_max;
            speedl = max_speed 
                     - (kp* error_right)/right_max 
                     + kd*(error_right - last_er)/right_max;
            // restriction of motor speed
            if (speedl > max_speed) speedl = max_speed;
            if (speedr > max_speed) speedr = max_speed;
            if (speedl < -max_speed) speedl = -max_speed;
            if (speedr < -max_speed) speedr = -max_speed; 
            move(speedr,speedl,time);
            last_er = error_right;
            last_el = error_left;
        }    
// cut till here to change mode ---------------------------------------------
        CyDelay(time);
    }
}   
// @ sumo
/*
motor_forward(max_speed,400);
            inside = 1;
        } else 
        if (!inside) {
            error_left = (Sensor_max - ref.l1);
            error_right = (Sensor_max - ref.r1);
            speedr = max_speed 
                     - (kp* error_left)/left_max 
                     + kd*(error_left - last_el)/left_max;
            speedl = max_speed 
                     - (kp* error_right)/right_max 
                     + kd*(error_right - last_er)/right_max;
            // restriction of motor speed
            if (speedl > max_speed) speedl = max_speed;
            if (speedr > max_speed) speedr = max_speed;
            if (speedl < -max_speed) speedl = -max_speed;
            if (speedr < -max_speed) speedr = -max_speed;  
            move(speedr,speedl,time);
            last_er = error_right;
            last_el = error_left;
        } else
        if ((dig.r3 == 0 || dig.r1 == 0 || dig.l1==0 || dig.l3==0) && inside) {
            motor_backward(200,250*time);
            turn_right(50,255,250*time);
        } else 
        if (dig.r1==1 && dig.l1==1 && inside){
            turn_right(max_speed,max_speed,500*time);
            motor_forward(max_speed,250*time);
        }     
        
*/

// @line follower
/*
           
*/

/* Don't remove the functions below */
int _write(int file, char *ptr, int len)
{
    (void)file; /* Parameter is not used, suppress unused argument warning */
	int n;
	for(n = 0; n < len; n++) {
        if(*ptr == '\n') UART_1_PutChar('\r');
		UART_1_PutChar(*ptr++);
	}
	return len;
}

int _read (int file, char *ptr, int count)
{
    int chs = 0;
    char ch;
 
    (void)file; /* Parameter is not used, suppress unused argument warning */
    while(count > 0) {
        ch = UART_1_GetChar();
        if(ch != 0) {
            UART_1_PutChar(ch);
            chs++;
            if(ch == '\r') {
                ch = '\n';
                UART_1_PutChar(ch);
            }
            *ptr++ = ch;
            count--;
            if(ch == '\n') break;
        }
    }
    return chs;
}
/* [] END OF FILE */
