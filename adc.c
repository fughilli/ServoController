#include "adc.h"

#include <msp430.h>

#include "simple_io.h"

static uint8_t ADC_PINS[] = { 3, 5 };
static uint8_t adc_input_index;

adc_t* adc;

//void adc_init(adc_t* _adc)
//{
//    adc = _adc;
//#if NUM_ADC_CHANNELS
//    ADC10CTL0 = SREF_0 + ADC10SR + REFBURST;
//    ADC10CTL1 = ADC10DIV_0 + ADC10SSEL_0 + CONSEQ_0 + SHS_0;
//    ADC10AE0 = 0;
//
//    for(i = 0; i < NUM_ADC_CHANNELS; i++)
//    {
//        set_pin_analog_input(ADC_PINS[i]);
//        pot_pos[i] = 0;
//    }
//
//
//    select_analog_channel(ADC_PINS[0]);
//
//    ADC10CTL0 |= ADC10IE + ADC10ON + ENC;
//#endif
//}
//
//__attribute__((__interrupt__(ADC10_VECTOR)))
//static void handle_adc() {
//#if NUM_ADC_CHANNELS
//    ADC10CTL0 &= ~ENC;
//    // Sample 10-bit value from ADC10MEM
//    ADC10CTL0 |= ENC;
//#endif
//}

void adc_init(adc_t* _adc)
{
    adc = _adc;
    adc_input_index = 0;

    /* Configure ADC10 with:
     * Vcc/Vss reference
     * 64x ADC10CLK cycles per conversion
     * 50ksps reference buffer
     * continuous conversion
     * turn on the ADC10
     */
    ADC10CTL0 = SREF_0 | ADC10SHT_3 | ADC10SR | ADC10IE | MSC | ADC10ON;

    ADC10CTL1 = INCH_0 | SHS_0 | ADC10DIV_7 | ADC10SSEL_3 | CONSEQ_2;

    for(uint8_t i = 0; i < NUM_ADC_CHANNELS; i++)
    {
        set_pin_analog_input(ADC_PINS[i]);
        adc->val[i] = 0;
    }

    select_analog_channel(ADC_PINS[adc_input_index]);

    ADC10CTL0 |= ADC10SC | ENC;
}

__attribute__((__interrupt__(ADC10_VECTOR)))
void ISR_adc10()
{
    ADC10CTL0 &= ~ADC10IFG;

    adc->val[adc_input_index++] = ADC10MEM;

    if(adc_input_index == NUM_ADC_CHANNELS)
        adc_input_index = 0;

    select_analog_channel(ADC_PINS[adc_input_index]);
}
