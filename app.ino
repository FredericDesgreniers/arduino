#include <avr/io.h>
#include <util/delay.h>
#include "serial.h"

#define F_CPU 1000000UL

void timer2_setup()
{
    TCCR2A |= (1 << COM2A1) | (1 << WGM21) | (1 << WGM20); //clear on compare match, continue count to OxFF
    TCCR2B |= (1 << CS22) | (1 << CS20);                   // prescale counter by 1024
    TCNT2 = 0;
    DDRD |= (255);
    OCR0A = 128; //initial compare match value

    DDRB |= 255;
}

void setCounter2To(int value)
{
    OCR2A = value;
}

void incrementCounter2By(int value)
{
    OCR2A += value;
}

void AD_setup()
{
    ADCSRA |= (1 << ADEN) | (1 << ADPS2); //enable with prescalar  at 16 (65kHz conversion clock speed)
    ADMUX |=  (1 << REFS0);             // Vcc ( 5v) conversion scale
}

int putDistanceSensorInADC()
{
    ADCSRA |= (1 << ADSC); //Start Conversion
    while (!(ADCSRA & (1 << ADIF)))
    {
    }                      //wait for ADIF to go to 0, indicating conversion complete.
    ADCSRA |= (1 << ADIF); //Reset ADIF to 1 for the next conversion
}

int transformDistanceToLight(int distance)
{
    return ADC%255;

    //Code below doesn't work well since the values seem to change depending on the place / arduino
    //So it's not used
    if(ADC < 100){
        return 255;
    }
    else if(ADC < 200)
    {
        return 128;
    }
    else
    {
        return 40;
    }
}

void runSensorPart()
{
    timer2_setup();
    
    AD_setup();
    while (1)
    {
        putDistanceSensorInADC();

        Serial_println(ADC);   // Print ADC conversion value
    
        setCounter2To(transformDistanceToLight(ADC));
        
        //setCounter2To(ADC);

        _delay_ms(100);
    }
}

void runSerialPart()
{
    timer2_setup();
    char buffer[4] = "255\0";

    while(1)
    {
        Serial_println("Enter light value (0-255):");

        int readIndex = 0;
        readLoop:
        while(readIndex < 4)
        {
            char inputValue = Serial_read();
            switch(inputValue)
            {
                case '\r':
                case '\n':
                {
                    buffer[readIndex] = '\0';
                    readIndex = 5; 
                    break;
                }

                default:{
                    buffer[readIndex] = inputValue;
                }
            }
            readIndex++;
        }
        int lightValue = atoi(buffer);
        setCounter2To(lightValue);
        Serial_print("Set light to ");
        Serial_println(buffer);

    }
}
int main(void)
{
    Serial_init();
    runSerialPart();
}

//OC0A is PWM output PD6.  This is our output pin for our PWM waveform.
