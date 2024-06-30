#include "includes.h"

void iwdg_init(void)
{
	/* enable IRC32K */
    rcu_osci_on(RCU_IRC32K);
    
    /* wait till IRC32K is ready */
    while(SUCCESS != rcu_osci_stab_wait(RCU_IRC32K)){
    }

	/* confiure FWDGT counter clock: 32KHz(IRC32K) / 64 = 0.5 KHz */
    fwdgt_config(2500,FWDGT_PSC_DIV64);
    
    /* After 5 seconds to generate a reset */
    fwdgt_enable();
}

void iwdg_feed(void)
{
	fwdgt_counter_reload();
}

