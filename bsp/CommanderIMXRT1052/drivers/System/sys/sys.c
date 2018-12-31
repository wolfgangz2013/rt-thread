#include "sys.h"
//////////////////////////////////////////////////////////////////////////////////	 
//������ֻ��ѧϰʹ�ã�δ��������ɣ��������������κ���;
//ALIENTEK I.MX RT1052������
//ϵͳʱ�ӳ�ʼ��	
//����ԭ��@ALIENTEK
//������̳:www.openedv.com
//��������:2017/12/10
//�汾��V1.0
//��Ȩ���У�����ؾ���
//Copyright(C) ������������ӿƼ����޹�˾ 2014-2024
//All rights reserved
//********************************************************************************
//�޸�˵��
//��
////////////////////////////////////////////////////////////////////////////////// 

//ARM PLL(PLL1)����,����Ϊ1200MHz
const clock_arm_pll_config_t armPllConfig = 	
{
	//ȡֵ��Χ54-108.
	//Fout=Fin*div_selcet/2=24*100/2=1200Mhz;
	.loopDivider = 100,		
	.src = 0, 
};

//SYS PLL(PLL2)���ã�����Ϊ528MHz
const clock_sys_pll_config_t sysPllConfig = 
{
	//SYS PLL��Ƶ���ӣ���ѡ��ѡ�����£�
    //Fout = Fin * ( 20 + loopDivider*2 + numerator / denominator) = 24*(20+1*2+0/1)=24*22=528MHz
	.loopDivider = 1,
  .numerator = 0,
  .denominator = 1,
	.src = 0, 
};

//USB1 PLL(PLL3)���ã�����Ϊ480MHz
const clock_usb_pll_config_t usb1PllConfig = 
{
	//USB1 PLL��Ƶ���ӣ���ѡ��ѡ�����£�
	//0: Fout=Fref*20=24*20=480M;
	//1: Fout=Fref*22=24*22=528M
	.loopDivider = 0,
	.src = 0,  
};


const clock_arm_pll_config_t armPllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 100,                       /* PLL loop divider, Fout = Fin * 50 */
        .src = 0,                                 /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
    };
const clock_sys_pll_config_t sysPllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 1,                         /* PLL loop divider, Fout = Fin * ( 20 + loopDivider*2 + numerator / denominator ) */
        .numerator = 0,                           /* 30 bit numerator of fractional loop divider */
        .denominator = 1,                         /* 30 bit denominator of fractional loop divider */
        .src = 0,                                 /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
    };
const clock_usb_pll_config_t usb1PllConfig_BOARD_BootClockRUN =
    {
        .loopDivider = 0,                         /* PLL loop divider, Fout = Fin * 20 */
        .src = 0,                                 /* Bypass clock source, 0 - OSC 24M, 1 - CLK1_P and CLK1_N */
    };

//ϵͳ��λ   
void Sys_Soft_Reset(void)
{   
	SCB->AIRCR =0X05FA0000|(u32)0x04;	  
} 	




void BOARD_BootClockRUN(void){
    /* Init RTC OSC clock frequency. */
    CLOCK_SetRtcXtalFreq(32768U);
    /* Enable 1MHz clock output. */
    XTALOSC24M->OSC_CONFIG2 |= XTALOSC24M_OSC_CONFIG2_ENABLE_1M_MASK;
    /* Use free 1MHz clock output. */
    XTALOSC24M->OSC_CONFIG2 &= ~XTALOSC24M_OSC_CONFIG2_MUX_1M_MASK;
    /* Set XTAL 24MHz clock frequency. */
    CLOCK_SetXtalFreq(24000000U);
    /* Enable XTAL 24MHz clock source. */
    CLOCK_InitExternalClk(0);
    /* Enable internal RC. */
    CLOCK_InitRcOsc24M();
    /* Switch clock source to external OSC. */
    CLOCK_SwitchOsc(kCLOCK_XtalOsc);
    /* Set Oscillator ready counter value. */
    CCM->CCR = (CCM->CCR & (~CCM_CCR_OSCNT_MASK)) | CCM_CCR_OSCNT(127);
    /* Setting PeriphClk2Mux and PeriphMux to provide stable clock before PLLs are initialed */
    CLOCK_SetMux(kCLOCK_PeriphClk2Mux, 1); /* Set PERIPH_CLK2 MUX to OSC */
    CLOCK_SetMux(kCLOCK_PeriphMux, 1);     /* Set PERIPH_CLK MUX to PERIPH_CLK2 */
    /* Setting the VDD_SOC to 1.5V. It is necessary to config AHB to 600Mhz. */
    DCDC->REG3 = (DCDC->REG3 & (~DCDC_REG3_TRG_MASK)) | DCDC_REG3_TRG(0x12);
    /* Waiting for DCDC_STS_DC_OK bit is asserted */
    while (DCDC_REG0_STS_DC_OK_MASK != (DCDC_REG0_STS_DC_OK_MASK & DCDC->REG0))
    {
    }
    /* Init ARM PLL. */
    CLOCK_InitArmPll(&armPllConfig_BOARD_BootClockRUN);
    /* In SDK projects, SDRAM (configured by SEMC) will be initialized in either debug script or dcd.
     * With this macro SKIP_SYSCLK_INIT, system pll (selected to be SEMC source clock in SDK projects) will be left unchanged.
     * Note: If another clock source is selected for SEMC, user may want to avoid changing that clock as well.*/
#ifndef SKIP_SYSCLK_INIT
    /* Init System PLL. */
    CLOCK_InitSysPll(&sysPllConfig_BOARD_BootClockRUN);
    /* Init System pfd0. */
    CLOCK_InitSysPfd(kCLOCK_Pfd0, 27);
    /* Init System pfd1. */
    CLOCK_InitSysPfd(kCLOCK_Pfd1, 16);
    /* Init System pfd2. */
    CLOCK_InitSysPfd(kCLOCK_Pfd2, 24);
    /* Init System pfd3. */
    CLOCK_InitSysPfd(kCLOCK_Pfd3, 32);
    /* Disable pfd offset. */
    CCM_ANALOG->PLL_SYS &= ~CCM_ANALOG_PLL_SYS_PFD_OFFSET_EN_MASK;
#endif
    /* In SDK projects, external flash (configured by FLEXSPI) will be initialized by dcd.
     * With this macro XIP_EXTERNAL_FLASH, usb1 pll (selected to be FLEXSPI clock source in SDK projects) will be left unchanged.
     * Note: If another clock source is selected for FLEXSPI, user may want to avoid changing that clock as well.*/
#if !(defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
    /* Init Usb1 PLL. */
    CLOCK_InitUsb1Pll(&usb1PllConfig_BOARD_BootClockRUN);
    /* Init Usb1 pfd0. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd0, 12);
    /* Init Usb1 pfd1. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd1, 13);
    /* Init Usb1 pfd2. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd2, 17);
    /* Init Usb1 pfd3. */
    CLOCK_InitUsb1Pfd(kCLOCK_Pfd3, 19);
#endif
    /* DeInit Audio PLL. */
    CLOCK_DeinitAudioPll();
    /* Bypass Audio PLL. */
    CLOCK_SetPllBypass(CCM_ANALOG, kCLOCK_PllAudio, 1);
    /* Set divider for Audio PLL. */
    CCM_ANALOG->MISC2 &= ~CCM_ANALOG_MISC2_AUDIO_DIV_LSB_MASK;
    CCM_ANALOG->MISC2 &= ~CCM_ANALOG_MISC2_AUDIO_DIV_MSB_MASK;
    /* Enable Audio PLL output. */
    CCM_ANALOG->PLL_AUDIO |= CCM_ANALOG_PLL_AUDIO_ENABLE_MASK;
    /* DeInit Video PLL. */
    CLOCK_DeinitVideoPll();
    /* Bypass Video PLL. */
    CCM_ANALOG->PLL_VIDEO |= CCM_ANALOG_PLL_VIDEO_BYPASS_MASK;
    /* Set divider for Video PLL. */
    CCM_ANALOG->MISC2 = (CCM_ANALOG->MISC2 & (~CCM_ANALOG_MISC2_VIDEO_DIV_MASK)) | CCM_ANALOG_MISC2_VIDEO_DIV(0);
    /* Enable Video PLL output. */
    CCM_ANALOG->PLL_VIDEO |= CCM_ANALOG_PLL_VIDEO_ENABLE_MASK;
    /* DeInit Enet PLL. */
    CLOCK_DeinitEnetPll();
    /* Bypass Enet PLL. */
    CLOCK_SetPllBypass(CCM_ANALOG, kCLOCK_PllEnet, 1);
    /* Set Enet output divider. */
    CCM_ANALOG->PLL_ENET = (CCM_ANALOG->PLL_ENET & (~CCM_ANALOG_PLL_ENET_DIV_SELECT_MASK)) | CCM_ANALOG_PLL_ENET_DIV_SELECT(1);
    /* Enable Enet output. */
    CCM_ANALOG->PLL_ENET |= CCM_ANALOG_PLL_ENET_ENABLE_MASK;
    /* Enable Enet25M output. */
    CCM_ANALOG->PLL_ENET |= CCM_ANALOG_PLL_ENET_ENET_25M_REF_EN_MASK;
    /* DeInit Usb2 PLL. */
    CLOCK_DeinitUsb2Pll();
    /* Bypass Usb2 PLL. */
    CLOCK_SetPllBypass(CCM_ANALOG, kCLOCK_PllUsb2, 1);
    /* Enable Usb2 PLL output. */
    CCM_ANALOG->PLL_USB2 |= CCM_ANALOG_PLL_USB2_ENABLE_MASK;
    /* Set AHB_PODF. */
    CLOCK_SetDiv(kCLOCK_AhbDiv, 0);
    /* Set IPG_PODF. */
    CLOCK_SetDiv(kCLOCK_IpgDiv, 3);
    /* Set ARM_PODF. */
    CLOCK_SetDiv(kCLOCK_ArmDiv, 1);
    /* Set preperiph clock source. */
    CLOCK_SetMux(kCLOCK_PrePeriphMux, 3);
    /* Set periph clock source. */
    CLOCK_SetMux(kCLOCK_PeriphMux, 0);
    /* Set PERIPH_CLK2_PODF. */
    CLOCK_SetDiv(kCLOCK_PeriphClk2Div, 0);
    /* Set periph clock2 clock source. */
    CLOCK_SetMux(kCLOCK_PeriphClk2Mux, 1);
    /* Set PERCLK_PODF. */
    CLOCK_SetDiv(kCLOCK_PerclkDiv, 0);
    /* Set per clock source. */
    CLOCK_SetMux(kCLOCK_PerclkMux, 1);
    /* Set USDHC1_PODF. */
    CLOCK_SetDiv(kCLOCK_Usdhc1Div, 1);
    /* Set Usdhc1 clock source. */
    CLOCK_SetMux(kCLOCK_Usdhc1Mux, 0);
    /* Set USDHC2_PODF. */
    CLOCK_SetDiv(kCLOCK_Usdhc2Div, 1);
    /* Set Usdhc2 clock source. */
    CLOCK_SetMux(kCLOCK_Usdhc2Mux, 0);
    /* In SDK projects, SDRAM (configured by SEMC) will be initialized in either debug script or dcd.
     * With this macro SKIP_SYSCLK_INIT, system pll (selected to be SEMC source clock in SDK projects) will be left unchanged.
     * Note: If another clock source is selected for SEMC, user may want to avoid changing that clock as well.*/
#ifndef SKIP_SYSCLK_INIT
    /* Set SEMC_PODF. */
    CLOCK_SetDiv(kCLOCK_SemcDiv, 3);
    /* Set Semc alt clock source. */
    CLOCK_SetMux(kCLOCK_SemcAltMux, 0);
    /* Set Semc clock source. */
    CLOCK_SetMux(kCLOCK_SemcMux, 0);
#endif
    /* In SDK projects, external flash (configured by FLEXSPI) will be initialized by dcd.
     * With this macro XIP_EXTERNAL_FLASH, usb1 pll (selected to be FLEXSPI clock source in SDK projects) will be left unchanged.
     * Note: If another clock source is selected for FLEXSPI, user may want to avoid changing that clock as well.*/
#if !(defined(XIP_EXTERNAL_FLASH) && (XIP_EXTERNAL_FLASH == 1))
    /* Set FLEXSPI_PODF. */
    CLOCK_SetDiv(kCLOCK_FlexspiDiv, 1);
    /* Set Flexspi clock source. */
    CLOCK_SetMux(kCLOCK_FlexspiMux, 0);
#endif
    /* Set CSI_PODF. */
    CLOCK_SetDiv(kCLOCK_CsiDiv, 1);
    /* Set Csi clock source. */
    CLOCK_SetMux(kCLOCK_CsiMux, 0);
    /* Set LPSPI_PODF. */
    CLOCK_SetDiv(kCLOCK_LpspiDiv, 4);
    /* Set Lpspi clock source. */
    CLOCK_SetMux(kCLOCK_LpspiMux, 2);
    /* Set TRACE_PODF. */
    CLOCK_SetDiv(kCLOCK_TraceDiv, 3);
    /* Set Trace clock source. */
    CLOCK_SetMux(kCLOCK_TraceMux, 2);
    /* Set SAI1_CLK_PRED. */
    CLOCK_SetDiv(kCLOCK_Sai1PreDiv, 3);
    /* Set SAI1_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_Sai1Div, 1);
    /* Set Sai1 clock source. */
    CLOCK_SetMux(kCLOCK_Sai1Mux, 0);
    /* Set SAI2_CLK_PRED. */
    CLOCK_SetDiv(kCLOCK_Sai2PreDiv, 3);
    /* Set SAI2_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_Sai2Div, 1);
    /* Set Sai2 clock source. */
    CLOCK_SetMux(kCLOCK_Sai2Mux, 0);
    /* Set SAI3_CLK_PRED. */
    CLOCK_SetDiv(kCLOCK_Sai3PreDiv, 3);
    /* Set SAI3_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_Sai3Div, 1);
    /* Set Sai3 clock source. */
    CLOCK_SetMux(kCLOCK_Sai3Mux, 0);
    /* Set LPI2C_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_Lpi2cDiv, 0);
    /* Set Lpi2c clock source. */
    CLOCK_SetMux(kCLOCK_Lpi2cMux, 0);
    /* Set CAN_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_CanDiv, 1);
    /* Set Can clock source. */
    CLOCK_SetMux(kCLOCK_CanMux, 2);
    /* Set UART_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_UartDiv, 0);
    /* Set Uart clock source. */
    CLOCK_SetMux(kCLOCK_UartMux, 0);
    /* Set LCDIF_PRED. */
    CLOCK_SetDiv(kCLOCK_LcdifPreDiv, 1);
    /* Set LCDIF_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_LcdifDiv, 4);
    /* Set Lcdif pre clock source. */
    CLOCK_SetMux(kCLOCK_LcdifPreMux, 5);
    /* Set SPDIF0_CLK_PRED. */
    CLOCK_SetDiv(kCLOCK_Spdif0PreDiv, 1);
    /* Set SPDIF0_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_Spdif0Div, 7);
    /* Set Spdif clock source. */
    CLOCK_SetMux(kCLOCK_SpdifMux, 3);
    /* Set FLEXIO1_CLK_PRED. */
    CLOCK_SetDiv(kCLOCK_Flexio1PreDiv, 1);
    /* Set FLEXIO1_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_Flexio1Div, 7);
    /* Set Flexio1 clock source. */
    CLOCK_SetMux(kCLOCK_Flexio1Mux, 3);
    /* Set FLEXIO2_CLK_PRED. */
    CLOCK_SetDiv(kCLOCK_Flexio2PreDiv, 1);
    /* Set FLEXIO2_CLK_PODF. */
    CLOCK_SetDiv(kCLOCK_Flexio2Div, 7);
    /* Set Flexio2 clock source. */
    CLOCK_SetMux(kCLOCK_Flexio2Mux, 3);
    /* Set Pll3 sw clock source. */
    CLOCK_SetMux(kCLOCK_Pll3SwMux, 0);
    /* Set lvds1 clock source. */
    CCM_ANALOG->MISC1 = (CCM_ANALOG->MISC1 & (~CCM_ANALOG_MISC1_LVDS1_CLK_SEL_MASK)) | CCM_ANALOG_MISC1_LVDS1_CLK_SEL(0);
    /* Set clock out1 divider. */
    CCM->CCOSR = (CCM->CCOSR & (~CCM_CCOSR_CLKO1_DIV_MASK)) | CCM_CCOSR_CLKO1_DIV(0);
    /* Set clock out1 source. */
    CCM->CCOSR = (CCM->CCOSR & (~CCM_CCOSR_CLKO1_SEL_MASK)) | CCM_CCOSR_CLKO1_SEL(1);
    /* Set clock out2 divider. */
    CCM->CCOSR = (CCM->CCOSR & (~CCM_CCOSR_CLKO2_DIV_MASK)) | CCM_CCOSR_CLKO2_DIV(0);
    /* Set clock out2 source. */
    CCM->CCOSR = (CCM->CCOSR & (~CCM_CCOSR_CLKO2_SEL_MASK)) | CCM_CCOSR_CLKO2_SEL(18);
    /* Set clock out1 drives clock out1. */
    CCM->CCOSR &= ~CCM_CCOSR_CLK_OUT_SEL_MASK;
    /* Disable clock out1. */
    CCM->CCOSR &= ~CCM_CCOSR_CLKO1_EN_MASK;
    /* Disable clock out2. */
    CCM->CCOSR &= ~CCM_CCOSR_CLKO2_EN_MASK;
    /* Set SystemCoreClock variable. */
    SystemCoreClock = BOARD_BOOTCLOCKRUN_CORE_CLOCK;
}

//����RT1052��ϵͳʱ��
void RT1052_Clock_Init(void)
{
  CLOCK_SetXtalFreq(24000000U);			//оƬ������24MHz
  CLOCK_SetRtcXtalFreq(32768U);			//RTC����32.728KHz

	//��ʼ��ʱ�ӽ׶�������Ϊ���ٵ�24Mʱ�ӣ�
	//��������½�PLL1����Ϊ1200MHz����Ƶ����Ϊ600MHz
	CLOCK_SetMux(kCLOCK_PeriphClk2Mux,1);   //����PERIPH_CLK2��ʱ��ԴΪOSCʱ�ӣ�Ҳ����24M���� 
	CLOCK_SetMux(kCLOCK_PeriphMux,1);       //����PERIPH_CLKΪPERIPH_CLK2=24MHz 

	//����VDD_SOC��ѹΪ1.25V�������Ļ�AHB�Ϳ��Ե�600MHz
  DCDC->REG3=(DCDC->REG3&(~DCDC_REG3_TRG_MASK))|DCDC_REG3_TRG(0x12);

  //�ȴ�DCDC���ȶ�����DCDC->REG0�Ĵ�����bit31(STS_DC_OK)Ϊ1
  while(DCDC_REG0_STS_DC_OK_MASK!=(DCDC_REG0_STS_DC_OK_MASK&DCDC->REG0)){};

	//���������е�1200Mhz��528Mhz��480Mhz��Ƶ�ʾ�����PLL1Ϊ1200Mhz�����õ�
	//��Ϊ�������������PLL1�ģ���Ҫ�������������õ�24MHz��������
  CLOCK_InitArmPll(&armPllConfig); 		//����PLL1(ARM PLL)Ϊ1200MHz
  CLOCK_InitSysPll(&sysPllConfig);		//����PLL2(SYS PLL)Ϊ528MHz
  CLOCK_InitUsb1Pll(&usb1PllConfig); 	//����PLL3(USB1 PLL)Ϊ480MHz

  CLOCK_SetDiv(kCLOCK_ArmDiv,1); 		    //ARM PODF 2��Ƶ����ѡ��0~7�ֱ��Ӧ1~8��Ƶ��1200Mhz/2=600MHz
  CLOCK_SetMux(kCLOCK_PrePeriphMux,3); 	//����PRE_PERIPH_CLK��ʱ��ԴΪPLL1����ARM PODF��Ƶ���ʱ��
											//����PRE_PERIPH_CLK=PLL1/ARM PODF=1200MHz/2=600MHz
  CLOCK_SetMux(kCLOCK_PeriphMux,0);    	//����PERIPH_CLKΪPRE_PERIPH_CLK=600MHz 
        
  CLOCK_SetDiv(kCLOCK_AhbDiv,0); 		    //AHB_CLK_ROOT=PERIPH_CLK/1=600/1=600Mhz
  CLOCK_SetDiv(kCLOCK_IpgDiv,3); 		    //IPG_CLK_ROOT=AHB_CLK_ROOT/4=600/4=150MHz
	CLOCK_SetDiv(kCLOCK_PerclkDiv,1); 	  //PER_CLK_ROOT=IPG_CLK_ROOT/2=150/2=75MHz

    
  //����PLL2(SYS PLL)����PFD
  CLOCK_InitSysPfd(kCLOCK_Pfd0,27);       //PLL2_PFD0=528*18/27=352Mhz
  CLOCK_InitSysPfd(kCLOCK_Pfd1,16);       //PLL2_PFD1=528*18/16=594Mhz
  CLOCK_InitSysPfd(kCLOCK_Pfd2,24);       //PLL2_PFD2=528*18/24=396Mhz
  CLOCK_InitSysPfd(kCLOCK_Pfd3,32);       //PLL2_PFD3=528*18/32=297Mhz
    
  //����PLL3(USB1 PLL)����PFD
  CLOCK_InitUsb1Pfd(kCLOCK_Pfd0,15);      //PLL3_PFD0=480*18/12=720Mhz
  CLOCK_InitUsb1Pfd(kCLOCK_Pfd1,13);      //PLL3_PFD1=480*18/13=664.62Mhz
  CLOCK_InitUsb1Pfd(kCLOCK_Pfd2,17);      //PLL3_PFD2=480*18/17=508.24Mhz
  CLOCK_InitUsb1Pfd(kCLOCK_Pfd3,19);      //PLL3_PFD3=480*18/19=454.74Mhz
    
	//һ��7��PLL�������ʼ����PLL1(ARM PLL)��PLL2(SYS PLL)��USB1 PLL(USB1 PLL)
	//�����ĸ�PLL�������ʵ��ʹ�����������ʼ����
  CLOCK_DeinitAudioPll();	//�ر�AUDIO PLL (PLL4)
  CLOCK_DeinitVideoPll();	//�ر�VIDEO PLL (PLL5)
  CLOCK_DeinitEnetPll();	//�ر�ENET  PLL (PLL6)
  CLOCK_DeinitUsb2Pll();	//�ر�USB2  PLL (PLL7)
  

	//����24MHz����ΪPITʱ��
	/* Configure PIT divider */   
  //CLOCK_SetMux(kCLOCK_PerclkMux, 1U); /* Set PERCLK_CLK source to OSC_CLK*/
  //CLOCK_SetDiv(kCLOCK_PerclkDiv, 0U); /* Set PERCLK_CLK divider to 1 */
	
	//�����ں�ʱ�ӣ��˺�������ȫ�ֱ���SystemCoreClock��ֵ��
	//SystemCoreClock�����ں�ʱ�ӣ�������������ô�ʱ�Ӿ���600MHz
	SystemCoreClockUpdate(); 
    	
	//ʹ��SNVSʱ��
	CLOCK_EnableClock(kCLOCK_IomuxcSnvs); 	
    
  CLOCK_SetMux(kCLOCK_SemcMux,0); //PERIPH_CLK��ΪSEMCʱ��Դ,PERIPH_CLK=600M
  CLOCK_SetDiv(kCLOCK_SemcDiv,3); //����SEMCʱ����PERIPH_CLK�ķ�Ƶ�õ�����SEMC Clock=600/4=150Mhz
	

}

//����RT1052��Ҫ�����Ĵ洢��
//����Բ��ִ洢�������MPU����,������ܵ��³��������쳣 
//Cortex-M7�ں˵�MPUһ����16��region��ÿ��region����ʼ��ַ�����Ժʹ�С�ȶ��������ã�
//MPU����ϸ��Ϣ�Լ��Ĵ���������ο���Cortex-M7����ֲ�(ST�ٷ�)���еĵ�4.6��
void MPU_Memory_Protection(void)
{
	SCB_DisableICache();						//�ر�I Cache
	SCB_DisableDCache();						//�ر�D Cache 
	ARM_MPU_Disable();							//�ر�MPU
 
	//����0~8���ã�RBAR�Ĵ�������region����ַ��BASR����region�����Ժʹ�С
	MPU->RBAR=ARM_MPU_RBAR(0,0XC0000000);											//region 0,��ʼ��ַ0XC000 0000
	MPU->RASR=ARM_MPU_RASR(0,ARM_MPU_AP_FULL,2,0,0,0,0,ARM_MPU_REGION_SIZE_512MB);	//��ִ��,ȫ����,�豸,��ֹ����,��ֹcache,��ֹ����,��ֹ��region,512MB��С

	MPU->RBAR=ARM_MPU_RBAR(1,0X80000000);											//region 1,��ʼ��ַ0X8000 0000,SDRAM�׵�ַ
	MPU->RASR=ARM_MPU_RASR(0,ARM_MPU_AP_FULL,2,0,0,0,0,ARM_MPU_REGION_SIZE_1GB);	//��ִ��,ȫ����,�豸,��ֹ����,��ֹcache,��ֹ����,��ֹ��region,1GB��С

	MPU->RBAR=ARM_MPU_RBAR(2,0X60000000);											//region 2,��ʼ��ַ0X6000 0000,FlexSPI�׵�ַ	
	MPU->RASR=ARM_MPU_RASR(0,ARM_MPU_AP_FULL,0,0,1,1,0,ARM_MPU_REGION_SIZE_512MB);	//��ִ��,ȫ����,д��,��ֹ����,����cache,������,��ֹ��region,512MB��С

	MPU->RBAR=ARM_MPU_RBAR(3,0X00000000);											//region 3,��ʼ��ַ0X0000 0000,ITCM�׵�ַ(����ROMCP,DTCM,OCRAM)
	MPU->RASR=ARM_MPU_RASR(0,ARM_MPU_AP_FULL,2,0,0,0,0,ARM_MPU_REGION_SIZE_1GB);	//��ִ��,ȫ����,�豸,��ֹ����,��ֹcache,��ֹ����,��ֹ��region,1GB��С

	MPU->RBAR=ARM_MPU_RBAR(4,0X00000000);											//region 4,��ʼ��ַ0X0000 0000,ITCM�׵�ַ,��ͬ��ַ������,region��ĸ���С��
	MPU->RASR=ARM_MPU_RASR(0,ARM_MPU_AP_FULL,0,0,1,1,0,ARM_MPU_REGION_SIZE_128KB);	//��ִ��,ȫ����,д��,��ֹ����,����cache,������,��ֹ��region,128KB��С

	MPU->RBAR=ARM_MPU_RBAR(5,0X20000000);											//region 5,��ʼ��ַ0X2000 0000,DTCM�׵�ַ,��ͬ��ַ������,region��ĸ���С��
	MPU->RASR=ARM_MPU_RASR(0,ARM_MPU_AP_FULL,0,0,1,1,0,ARM_MPU_REGION_SIZE_128KB);	//��ִ��,ȫ����,д��,��ֹ����,����cache,������,��ֹ��region,128KB��С

	MPU->RBAR=ARM_MPU_RBAR(6,0X20200000);											//region 6,��ʼ��ַ0X2020 0000,OCRAM�׵�ַ,��ͬ��ַ������,region��ĸ���С��
	MPU->RASR=ARM_MPU_RASR(0,ARM_MPU_AP_FULL,0,1,1,0,0,ARM_MPU_REGION_SIZE_256KB);	//��ִ��,ȫ����,д��,������,����cache,��ֹ����,��ֹ��region,256KB��С

	MPU->RBAR=ARM_MPU_RBAR(7,0X80000000);											//region 7,��ʼ��ַ0X8000 0000,SDRAM�׵�ַ,��ͬ��ַ������,region��ĸ���С��
	MPU->RASR=ARM_MPU_RASR(0,ARM_MPU_AP_FULL,0,0,1,1,0,ARM_MPU_REGION_SIZE_2MB);	//��ִ��,ȫ����,д��,��ֹ����,����cache,������,��ֹ��region,2MB��С 
	
	ARM_MPU_Enable(MPU_CTRL_PRIVDEFENA_Msk);	//ʹ��MPU,��������region
	SCB_EnableDCache();							//ʹ��D Cache
	SCB_EnableICache();							//ʹ��I Cache
}

//�ж����ȼ���������
//grpx: ���ȼ��飬��ѡ������5���е�һ��
//NVIC_PRIORITYGROUP_0: 0λ��ռ���ȼ�
//                      4λ�����ȼ�
//NVIC_PRIORITYGROUP_1: 1λ��ռ���ȼ�
//                      3λ�����ȼ�
//NVIC_PRIORITYGROUP_2: 2λ��ռ���ȼ�
//                      2λ�����ȼ�
//NVIC_PRIORITYGROUP_3: 3λ��ռ���ȼ�
//                      1λ�����ȼ�
//NVIC_PRIORITYGROUP_4: 4λ��ռ���ȼ�
//                      0λ�����ȼ�
//
void RT1052_NVIC_SetPriorityGrouping(u8 grpx)
{
	NVIC_SetPriorityGrouping((uint32_t)grpx);
}

//�����ж����ȼ�
//IRQn:	�ж�Դ
//prep����ռ���ȼ�
//subp�������ȼ�
void RT1052_NVIC_SetPriority(IRQn_Type IRQn,u32 prep,u32 subp)
{ 
	u32 prioritygroup = 0x00;
  
	prioritygroup=NVIC_GetPriorityGrouping();
	NVIC_SetPriority(IRQn,NVIC_EncodePriority(prioritygroup,prep,subp));
}

//����������ƫ�Ƶ�ַ
//vtab:��ַ
//offset:ƫ����		 
void MY_NVIC_SetVectorTable(u32 vtab,u32 offset)	 
{ 	   	  
	SCB->VTOR=vtab|(offset&(u32)0xFFFFFE00);	//����NVIC��������ƫ�ƼĴ���,VTOR��9λ����,��[8:0]������
}

//THUMBָ�֧�ֻ������
//�������·���ʵ��ִ�л��ָ��WFI  
__asm void WFI_SET(void)
{
	WFI;		  
}
//�ر������ж�(���ǲ�����fault��NMI�ж�)
__asm void INTX_DISABLE(void)
{
	CPSID   I
	BX      LR	  
}
//���������ж�
__asm void INTX_ENABLE(void)
{
	CPSIE   I
	BX      LR  
}
//����ջ����ַ
//addr:ջ����ַ
__asm void MSR_MSP(u32 addr) 
{
	MSR MSP, r0 			//set Main Stack value
	BX r14
}
