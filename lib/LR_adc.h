#ifndef LR_ADC_H_
#define LR_ADC_H_



void LR_adc_init_channel(unsigned char chan);
void LR_adc_init();
void LR_adc_enable_channel(unsigned char chan);
void LR_adc_start();
unsigned long LR_adc_get(unsigned char chan);
void LR_adc_disable_channel(unsigned char chan);


#endif /* LR_ADC_H_ */
