/*
**********************************************************************************************************************
*
*						           the Embedded Secure Bootloader System
*
*
*						       Copyright(C), 2006-2014, Allwinnertech Co., Ltd.
*                                           All Rights Reserved
*
* File    :
*
* By      :
*
* Version : V2.00
*
* Date	  :
*
* Descript:
**********************************************************************************************************************
*/
#include <common.h>
#include <private_boot0.h>
#include <asm/arch/clock.h>
#include <asm/arch/timer.h>
#include <asm/arch/uart.h>
#include <asm/arch/dram.h>

extern const boot0_file_head_t  BT0_head;

static void print_version(void);

void __attribute__((weak)) bias_calibration(void)
{
	return;
}
/*******************************************************************************
*函数名称: Boot0_C_part
*函数原型：void Boot0_C_part( void )
*函数功能: Boot0中用C语言编写的部分的主流程
*入口参数: void
*返 回 值: void
*备    注:
*******************************************************************************/
void main( void )
{
	__u32 status;
	__s32 dram_size;
	int	index = 0;
	int   ddr_aotu_scan = 0;

    __u32 fel_flag;

	bias_calibration();
    timer_init();

    sunxi_serial_init( BT0_head.prvt_head.uart_port, (void *)BT0_head.prvt_head.uart_ctrl, 6 );
	if( BT0_head.prvt_head.enable_jtag )
    {
    	boot_set_gpio((normal_gpio_cfg *)BT0_head.prvt_head.jtag_gpio, 6, 1);
    }
	printf("HELLO! BOOT0 is starting!\n");
	print_version();

#ifdef CONFIG_ARCH_SUN7I
	reset_cpux(1);
#endif
    fel_flag = rtc_region_probe_fel_flag();
    if(fel_flag == SUNXI_RUN_EFEX_FLAG)
    {
        rtc_region_clear_fel_flag();
    	printf("eraly jump fel\n");
    	reset_pll();
    	__msdelay(10);

    	boot0_jump(FEL_BASE);
    }

	mmu_setup();

    ddr_aotu_scan = 0;
//	dram_para_display();
	dram_size = init_DRAM(ddr_aotu_scan, (void *)BT0_head.prvt_head.dram_para);
	if(dram_size)
	{
	    //mdfs_save_value();
		printf("dram size =%d\n", dram_size);
	}
	else
	{
		printf("initializing SDRAM Fail.\n");
		mmu_turn_off( );

		reset_pll();
		boot0_jump(FEL_BASE);
	}
#if defined(CONFIG_ARCH_SUN9IW1P1)
	__msdelay(100);
#endif

#ifdef CONFIG_ARCH_SUN7I
    check_super_standby_flag();
#endif

	status = load_boot1();

	printf("Ready to disable icache.\n");

	mmu_turn_off( );                               // disable instruction cache

	if( status == 0 )
	{
		//跳转之前，把所有的dram参数写到boot1中
		set_dram_para((void *)&BT0_head.prvt_head.dram_para, dram_size);
		printf("Jump to secend Boot.\n");

		boot0_jump(CONFIG_SYS_TEXT_BASE);		  // 如果载入Boot1成功，跳转到Boot1处执行
	}
	else
	{
//		disable_watch_dog( );                     // disable watch dog
		reset_pll();
		printf("Jump to Fel.\n");
		boot0_jump(FEL_BASE);                     // 如果载入Boot1失败，将控制权交给Fel
	}
}
/*
************************************************************************************************************
*
*                                             function
*
*    name          :
*
*    parmeters     :
*
*    return        :
*
*    note          :
*
*
************************************************************************************************************
*/
static void print_version(void)
{
	printf("boot0 version : %s\n", BT0_head.boot_head.platform + 2);

	return;
}

