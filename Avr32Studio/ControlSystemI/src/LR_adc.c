/*
 * LR_adc.c
 */


#include "gpio.h"
#include "adc.h"

volatile avr32_adc_t *adc = &AVR32_ADC;


void LR_adc_init_channel(unsigned char chan)
{
  const gpio_map_t ADC =
  {
      {AVR32_ADC_AD_0_PIN + chan, 0}
  };
  gpio_enable_module(ADC,sizeof(ADC) / sizeof(ADC[0]));
}

void LR_adc_init()
{
  AVR32_ADC.mr |= 0x1 << AVR32_ADC_MR_PRESCAL_OFFSET;
  adc_configure(adc);
}

void LR_adc_enable_channel(unsigned char chan)
{
  adc_enable(adc,chan);
}

void LR_adc_start()
{
  adc_start(adc);
}

unsigned long LR_adc_get(unsigned char chan)
{
  return adc_get_value(adc, chan);
}

void LR_adc_disable_channel(unsigned char chan)
{
  adc_disable(adc,chan);
}
