#ifndef ADC_H
#define ADC_H

#include <stdint.h>

#define NUM_ADC_CHANNELS (2)

typedef struct
{
    uint16_t val[2];
} adc_t;

void adc_init(adc_t* _adc);

#endif // ADC_H
