#include "includes.h"

#define ADC_CH_NUM	5
static unsigned short adc_value[ADC_CH_NUM];

void adc_init(void)
{
	dma_single_data_parameter_struct dma_single_data_parameter;
	
	rcu_periph_clock_enable(RCU_GPIOC);
    rcu_periph_clock_enable(RCU_ADC0);
	rcu_periph_clock_enable(RCU_DMA1);
	
	adc_clock_config(ADC_ADCCK_PCLK2_DIV6);

	gpio_mode_set(GPIOB, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_0);//ain3 in8
	gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_1);//VKK in11
	gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_2);//BAT in12
	gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_4);//ain2 in14
	gpio_mode_set(GPIOC, GPIO_MODE_ANALOG, GPIO_PUPD_NONE, GPIO_PIN_5);//ain1 in15

    dma_deinit(DMA1, DMA_CH0);
    
    dma_single_data_parameter.periph_addr = (unsigned int)(&ADC_RDATA(ADC0));
    dma_single_data_parameter.periph_inc = DMA_PERIPH_INCREASE_DISABLE;
    dma_single_data_parameter.memory0_addr = (unsigned int)(adc_value);
    dma_single_data_parameter.memory_inc = DMA_MEMORY_INCREASE_ENABLE;
    dma_single_data_parameter.periph_memory_width = DMA_PERIPH_WIDTH_16BIT;
    dma_single_data_parameter.direction = DMA_PERIPH_TO_MEMORY;
    dma_single_data_parameter.number = ADC_CH_NUM;
    dma_single_data_parameter.priority = DMA_PRIORITY_HIGH;
    dma_single_data_mode_init(DMA1, DMA_CH0, &dma_single_data_parameter);
    dma_channel_subperipheral_select(DMA1, DMA_CH0, DMA_SUBPERI0);

    dma_circulation_enable(DMA1, DMA_CH0);
    dma_channel_enable(DMA1, DMA_CH0);

	/* ADC mode config */
    adc_sync_mode_config(ADC_SYNC_MODE_INDEPENDENT);
    /* ADC contineous function disable */
    adc_special_function_config(ADC0, ADC_CONTINUOUS_MODE, ENABLE);
    /* ADC scan mode disable */
    adc_special_function_config(ADC0, ADC_SCAN_MODE, ENABLE);
    /* ADC data alignment config */
    adc_data_alignment_config(ADC0, ADC_DATAALIGN_RIGHT);
    
    /* ADC channel length config */
    adc_channel_length_config(ADC0, ADC_REGULAR_CHANNEL, ADC_CH_NUM);
    /* ADC regular channel config */
    adc_regular_channel_config(ADC0, 0, ADC_CHANNEL_8, ADC_SAMPLETIME_28);
    adc_regular_channel_config(ADC0, 1, ADC_CHANNEL_11, ADC_SAMPLETIME_28);
    adc_regular_channel_config(ADC0, 2, ADC_CHANNEL_12, ADC_SAMPLETIME_28);
    adc_regular_channel_config(ADC0, 3, ADC_CHANNEL_14, ADC_SAMPLETIME_28);
    adc_regular_channel_config(ADC0, 4, ADC_CHANNEL_15, ADC_SAMPLETIME_28);
    /* ADC trigger config */
    adc_external_trigger_source_config(ADC0, ADC_REGULAR_CHANNEL, ADC_EXTTRIG_REGULAR_T0_CH0); 
    adc_external_trigger_config(ADC0, ADC_REGULAR_CHANNEL, EXTERNAL_TRIGGER_DISABLE);
    
    /* ADC DMA function enable */
    adc_dma_request_after_last_enable(ADC0);
    adc_dma_mode_enable(ADC0);
    
    /* enable ADC interface */
    adc_enable(ADC0);
    /* wait for ADC stability */
    delayms(1);
    /* ADC calibration and reset calibration */
    adc_calibration_enable(ADC0);
    
    /* enable ADC software trigger */
    adc_software_trigger_enable(ADC0, ADC_REGULAR_CHANNEL);
}

unsigned short adc_get_data(unsigned char ch)
{
	return ((adc_value[ch]*3300)>>12);
}

