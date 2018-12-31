/*******************************************************************************
 * Copyright (C) 2016, Huada Semiconductor Co.,Ltd All rights reserved.
 *
 * This software is owned and published by:
 * Huada Semiconductor Co.,Ltd ("HDSC").
 *
 * BY DOWNLOADING, INSTALLING OR USING THIS SOFTWARE, YOU AGREE TO BE BOUND
 * BY ALL THE TERMS AND CONDITIONS OF THIS AGREEMENT.
 *
 * This software contains source code for use with HDSC
 * components. This software is licensed by HDSC to be adapted only
 * for use in systems utilizing HDSC components. HDSC shall not be
 * responsible for misuse or illegal use of this software for devices not
 * supported herein. HDSC is providing this software "AS IS" and will
 * not be responsible for issues arising from incorrect user implementation
 * of the software.
 *
 * Disclaimer:
 * HDSC MAKES NO WARRANTY, EXPRESS OR IMPLIED, ARISING BY LAW OR OTHERWISE,
 * REGARDING THE SOFTWARE (INCLUDING ANY ACOOMPANYING WRITTEN MATERIALS),
 * ITS PERFORMANCE OR SUITABILITY FOR YOUR INTENDED USE, INCLUDING,
 * WITHOUT LIMITATION, THE IMPLIED WARRANTY OF MERCHANTABILITY, THE IMPLIED
 * WARRANTY OF FITNESS FOR A PARTICULAR PURPOSE OR USE, AND THE IMPLIED
 * WARRANTY OF NONINFRINGEMENT.
 * HDSC SHALL HAVE NO LIABILITY (WHETHER IN CONTRACT, WARRANTY, TORT,
 * NEGLIGENCE OR OTHERWISE) FOR ANY DAMAGES WHATSOEVER (INCLUDING, WITHOUT
 * LIMITATION, DAMAGES FOR LOSS OF BUSINESS PROFITS, BUSINESS INTERRUPTION,
 * LOSS OF BUSINESS INFORMATION, OR OTHER PECUNIARY LOSS) ARISING FROM USE OR
 * INABILITY TO USE THE SOFTWARE, INCLUDING, WITHOUT LIMITATION, ANY DIRECT,
 * INDIRECT, INCIDENTAL, SPECIAL OR CONSEQUENTIAL DAMAGES OR LOSS OF DATA,
 * SAVINGS OR PROFITS,
 * EVEN IF Disclaimer HAS BEEN ADVISED OF THE POSSIBILITY OF SUCH DAMAGES.
 * YOU ASSUME ALL RESPONSIBILITIES FOR SELECTION OF THE SOFTWARE TO ACHIEVE YOUR
 * INTENDED RESULTS, AND FOR THE INSTALLATION OF, USE OF, AND RESULTS OBTAINED
 * FROM, THE SOFTWARE.
 *
 * This software may be replicated in part or whole for the licensed use,
 * with the restriction that this Disclaimer and Copyright notice must be
 * included with each copy of this software, whether used in part or whole,
 * at all times.
 */
/******************************************************************************/
/** \file system_hc32f_m14x.c
 **
 ** A detailed description is available at
 ** @link SystemGroup System description @endlink
 **
 **   - 2016-5-16  1.0  Zhangxl First version for Device Driver Library of
 **     system config.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#include "base_types.h"
#include "hc32f_m14x.h"
#include "system_hc32f_m14x.h"
#include "clk.h"

/**
 *******************************************************************************
 ** \addtogroup SystemGroup
 ******************************************************************************/
//@{

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/
/**
 *******************************************************************************
 ** System Clock Frequency (Core Clock) Variable according CMSIS
 ******************************************************************************/
uint32_t SystemCoreClock = 32000000;

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/

/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
/**
 *******************************************************************************
 ** \brief  Setup the microcontroller system. Initialize the System and update
 **         the SystemCoreClock variable.
 **
 ******************************************************************************/
void SystemInit(void)
{
    uint32_t u32Timeout = CLK_TIMEOUT;
    
#if (HWWD_DISABLE)                                   /* HW Watchdog Disable */
    M0P_HWWDT->WDT_LCK = 0x1ACCE551u;                /* HW Watchdog Unlock */
    M0P_HWWDT->WDT_LCK = 0xE5331AAEu;
    M0P_HWWDT->WDT_CTL = 0u;                         /* HW Watchdog stop */
#endif

    /* Set division factor for base clock. */
    M0P_CLOCK->BC_DIV = 0x0;
    
    /* Set division factor for apb0 clock. */
    M0P_CLOCK->APB0_CTRL = 0x0;
    
    /* Set division factor for apb1 clock and enable it. */
    M0P_CLOCK->APB1_CTRL = 0x80;
    
    /* Set the range of external osc frequency. */
    M0P_OSC_CTL->HSXT_PWRCTL_f.HXSEL = 0;
    
    /* Set the waiting time for external osc stabilization. */
    M0P_CLOCK->CSWT = 0x82;
    
    /* Set the waiting time for pll. */
    M0P_CLOCK->PSWT = 0x3;
    
    /* Set for pll. */
    M0P_CLOCK->PLL_CTL1 = 0x4;    
    M0P_CLOCK->PLL_CTL2 = 0x8;
   
    /* Flash wait time. */
    M0P_FLASH_IF->FWCICR = 0x0;
    
    /* Enable high speed internal rc,external osc and pll. */
    M0P_CLOCK->CTRL = 0x13;
    
    /* Wait the pll stable. */
    while (FALSE == M0P_CLOCK->CLK_STB_f.PLL_RDY)
    {
        ;
    }
    
    /* Switch to pll. */
    M0P_CLOCK->CTRL_f.INPUT = ClkMPLL;
    
    /* Wait the system clock is switch to high speed external osc. */
    while (--u32Timeout > 0)
    {
        if (ClkMPLL == M0P_CLOCK->CLK_STB_f.SRC)
        {
            break;
        }
    }
    
    if(ClkMPLL != M0P_CLOCK->CLK_STB_f.SRC)
    {
        while(1)
        {
            ;
        }
    }
}

/**
 *******************************************************************************
 ** \brief  Update system base clock.
 *
 ******************************************************************************/
void SystemCoreClockUpdate(void)
{
    uint32_t u32Val = 0;
    uint8_t u8Val = M0P_CLOCK->BC_DIV_f.DIV;
    
    switch (M0P_CLOCK->CLK_STB_f.SRC)
    {
        case ClkHICR:
            u32Val = CLK_HICR_VAL;
            break;
        case ClkHSXT:
            u32Val = CLK_HSXT_VAL;
            break;
        case ClkMPLL:
            if(M0P_CLOCK->PLL_CTL2_f.PRS)
            {
                u32Val = CLK_HICR_VAL;
            }
            else
            {
                u32Val = CLK_HSXT_VAL;
            }
            u32Val *= M0P_CLOCK->PLL_CTL2_f.PLLN;
            break;
        case ClkLICR:
            u32Val = CLK_LICR_VAL;
            break;
        case ClkLSXT:
            u32Val = CLK_LSXT_VAL;
            break;
        default:
            break;
    }
    
    switch (u8Val)
    {
        case ClkBaseDiv1:
            break;
        case ClkBaseDiv2:
            u32Val = u32Val / 2;
            break;
        case ClkBaseDiv3:
            u32Val = u32Val / 3;
            break;
        case ClkBaseDiv4:
            u32Val = u32Val / 4;
            break;
        case ClkBaseDiv6:
            u32Val = u32Val / 6;
            break;
        case ClkBaseDiv8:
            u32Val = u32Val / 8;
            break;
        case ClkBaseDiv16:
            u32Val = u32Val / 16;
            break;
        default:
            break;
    }
    
    SystemCoreClock = u32Val;
}

//@} // SystemGroup

/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
