/*
 * File:   main.c
 * Author: olivier.sannier
 *
 * Created on January 2, 2017, 2:30 PM
 */


#include <xc.h>
#include "mcc_generated_files/mcc.h"
#define Light_On Light_SetHigh
#define Light_Off Light_SetLow

volatile int32_t elapsedHalfSeconds = 0;
volatile int32_t maxHalfSeconds = 60;
volatile bool mainVibrationDetected = false;

int32_t GetUserLightDuration()
{
    return 2 + (ADC1_GetConversion(LightDuration) >> 2);
}

void Vibration0Interrupt()
{
    // Main vibration detected, reset timer and restart counting half seconds
    mainVibrationDetected = true;
    elapsedHalfSeconds = 0;
    TMR0_Reload();
}

void TimerInterrupt()
{
    elapsedHalfSeconds++;
    
    if (elapsedHalfSeconds > maxHalfSeconds)
        Light_Off();
}

void main(void) {
    SYSTEM_Initialize();
    
    INT_SetInterruptHandler(Vibration0Interrupt);
    TMR0_SetInterruptHandler(TimerInterrupt);

    INTERRUPT_GlobalInterruptEnable();
    INTERRUPT_PeripheralInterruptEnable();
    
    while(1)
    {
        // Don't go to sleep, the timer would not work during
        // that time and thus lights would never go out
        /*SLEEP();
        __nop();*/
        
        if (mainVibrationDetected)
        {
            mainVibrationDetected = false;
            
            int32_t initialHalfSeconds = elapsedHalfSeconds; 
            bool vibration1Occurred = false;
            bool darknessOK = false;
            while (elapsedHalfSeconds == initialHalfSeconds)
            {
                vibration1Occurred = vibration1Occurred || !Vibration1_GetValue();
                darknessOK = darknessOK || Darkness_GetValue();
                
                if (vibration1Occurred && darknessOK)
                {
                    maxHalfSeconds = GetUserLightDuration();
                    Light_On();
                }
            }
        }
    }
    
    return;
}
