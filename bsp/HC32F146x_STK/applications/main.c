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
/** \file main.c
 **
 ** \brief This sample demonstrates how to set GPIO as output.
 **
 **   - 2017-08-14  1.0  chengy first version for Device Driver Library of GPIO.
 **
 ******************************************************************************/

/*******************************************************************************
 * Include files
 ******************************************************************************/
#define RTOS_NONE           (0)
#define RTOS_NANO           (1)
#define RTOS                RTOS_NANO //RTOS_NONE

#if (RTOS == RTOS_NANO)
#include <rtthread.h> 
#endif
#include "ddl.h"
#include "gpio.h"

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define LED3_PORT           (2)
#define LED3_PIN            (4)

#if (RTOS == RTOS_NANO)
#define CONSOLE_TEST
#define MSH_TEST
#define LCD_TEST
#endif

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

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
 ** \brief  Main function of project
 **
 ** \param  None
 **
 ** \retval int32_t return value, if needed
 **
 ******************************************************************************/
int32_t main(void)
{
    volatile uint8_t u8val = 0;

#ifdef CONSOLE_TEST
    rt_kprintf("Hello\n");
#endif
    Gpio_InitIO(LED3_PORT, LED3_PIN, GpioDirOut, FALSE, FALSE);

    while (1)
    {
        Gpio_SetIO(LED3_PORT, LED3_PIN, u8val);
        u8val = !u8val;
    #if (RTOS == RTOS_NONE)    
        delay1ms(1000);
    #else    
        rt_thread_delay(RT_TICK_PER_SECOND);
    #endif    
    }
}

#ifdef MSH_TEST

int msh_test(int argc, char** argv)
{
    int i = 0;
    rt_kprintf("arg len: %d\n", argc);
    for(i = 0;i < argc; i++)
    {
        rt_kprintf("argv[%d]: %s\n", i,argv[i]);
    }
   
    return 0;
}
#if defined(RT_USING_FINSH)
#include <finsh.h>
    #if defined(FINSH_USING_MSH)
        MSH_CMD_EXPORT_ALIAS(msh_test, test, msh test sample);
    #endif
#endif

#endif //MSH_TEST

#ifdef LCD_TEST
#include <string.h>
#define LCD_TEST_MAX_STRING_SIZE 4

int msh_lcd(int argc, char **argv)
{
    int len = 0;
    int i = 0;
    char *c = RT_NULL;
    rt_device_t dev = RT_NULL;

    if(argc != 2)
    {
        rt_kprintf("Please input one string(4 bytes at most) at least\n");
        return 1;
    }
    
    dev = rt_device_find(RT_LCD_DEVICE_NAME);
    if(dev == RT_NULL)
    {
        rt_kprintf("finsh: can not find device: %s\n", RT_LCD_DEVICE_NAME);
        return 1;
    }
    
    if(rt_device_open(dev, RT_DEVICE_FLAG_WRONLY) != RT_EOK)
    {
        rt_kprintf("finsh: can not open device: %s\n", RT_LCD_DEVICE_NAME);
        return 1;
    }
    
    len = strlen(argv[1]);
    len = (len > LCD_TEST_MAX_STRING_SIZE ? LCD_TEST_MAX_STRING_SIZE : len);
    c = &argv[1][0];
    
    for(i = 0; i < len; i++)
    {
        rt_device_write(dev, i, c++, 1);
    }
    
    return 0;
}
    
#if defined(RT_USING_FINSH)
#include <finsh.h>
    #if defined(FINSH_USING_MSH)
        MSH_CMD_EXPORT_ALIAS(msh_lcd, lcd, lcd sample);
    #endif
#endif

#endif //LCD_TEST
/*******************************************************************************
 * EOF (not truncated)
 ******************************************************************************/
