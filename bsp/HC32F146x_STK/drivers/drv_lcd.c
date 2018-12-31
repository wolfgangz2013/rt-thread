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
/** \file drv_lcd.c
 **
 **   - 2018-1-4        Jason       The first version.
 **
 ******************************************************************************/
#include <rthw.h>
#include <rtthread.h>
#ifdef RT_USING_DEVICE
#include <rtdevice.h>
#endif
#include "ddl.h"
#include "lcd.h"
#ifdef RT_USING_LCD

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
 
/* Element assignment in a nixietube */
/*****************
        A                
     ------
    |      |
   F|   G  | B
    |______|
    |      |
   E|      | C
    |______|
        D
First seg include F G E P
Second seg include A B C D
******************/

/* Pin to LCD ram */
/********************************************************
    |----LCD-GDC04212------|   |----------LCDC---------|
    |   pin 12  --- SEG8    |   |   pin 55  --- SEG 20  |
    |   pin 11  --- SEG7    |   |   pin 46  --- SEG 13  |
    |   pin 10  --- SEG6    |   |   pin 40  --- SEG 7   |
    |   pin 9   --- SEG5    |   |   pin 39  --- SEG 6   |
    |   pin 8   --- SEG4    |   |   pin 29  --- SEG 5   |
    |   pin 7   --- SEG3    |   |   pin 26  --- SEG 2   |
    |   pin 6   --- SEG2    |   |   pin 25  --- SEG 1   |
    |   pin 5   --- SEG1    |   |   pin 24  --- SEG 0   |
    |   pin 4   --- COM3    |   |   pin 35  --- COM3    |
    |   pin 3   --- COM2    |   |   pin 36  --- COM2    |
    |   pin 2   --- COM1    |   |   pin 37  --- COM1    |
    |   pin 1   --- COM0    |   |   pin 38  --- COM0    |
    |-----------------------|   |-----------------------|
********************************************************/

#define COM0                0x01
#define COM1                0x02
#define COM2                0x04
#define COM3                0x08

#define A                   COM0
#define B                   COM1
#define C                   COM2
#define D                   COM3
#define E                   COM2
#define F                   COM0
#define G                   COM1

#define P1                  COM3
#define P2                  COM3
#define P3                  COM3
#define P4                  COM3

#define SEG1                0
#define SEG2                1
#define SEG3                2
#define SEG4                5
#define SEG5                6
#define SEG6                7
#define SEG7                13
#define SEG8                20

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
typedef struct content
{
    rt_uint8_t first_seg_content;
    rt_uint8_t second_seg_content; 
} content_t;

typedef struct group
{
    rt_uint8_t first_seg_idx;
    rt_uint8_t second_seg_idx;
} group_t;

typedef struct hc32_lcd
{
    struct rt_device        parent;
    char                    *name;
} hc32_lcd_t;

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
const content_t lcd_content[] =            
{
    {E | F,         A | B | C | D,},    /* 0 */
    {0x0,           B | C,},            /* 1 */
    {E | G,         A | B | D,},        /* 2 */
    {G,             A | B | C | D,},    /* 3 */
    {F | G,         B | C,},            /* 4 */
    {F | G,         A | C | D,},        /* 5 */
    {E | F | G,     A | C | D,},        /* 6 */
    {0x0,           A | B | C,},        /* 7 */
    {E | F | G,     A | B | C | D,},    /* 8 */
    {F | G,         A | B | C | D,},    /* 9 */        
};

const group_t lcd_group[] =
{
    {SEG1, SEG2},
    {SEG3, SEG4},
    {SEG5, SEG6},
    {SEG7, SEG8},
};

static hc32_lcd_t lcd_instance =
{
    .name = RT_LCD_DEVICE_NAME,
};
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/

static rt_err_t rt_lcd_init(rt_device_t device)
{
    stc_lcd_cfg_t  stcLcdInitconfig;
    en_result_t  enresult =  Error;

    DDL_ZERO_STRUCT(stcLcdInitconfig);

    /* LCD clock selected internal RC. */
    stcLcdInitconfig.enLcdClk = LcdClkLICR;
    /* The frame refresh rate select 128hz. */
    stcLcdInitconfig.enLcdRefreshRate =  LcdRefreshRate512Hz;
    /* LCD working mode set,include com?bias?duty?waveform?set. */
    stcLcdInitconfig.enLcdWorkMode  = Lcd_4Com_1Div4Duty_1Div3bias_TypeB_Max40Seg;

    /* LCD drive circuit select,in R. */
    stcLcdInitconfig.stcLcdDriveCircuit.enDriveCircuit = LcdBiasCircuitInRes;
    /* LCD contrast select,Contrast 100%. */
    stcLcdInitconfig.stcLcdDriveCircuit.stcLcdInrCfg.enContrast = LcdContrast100per;
    /* LCD Charge time select,Charging time 1 CLK. */
    stcLcdInitconfig.stcLcdDriveCircuit.stcLcdInrCfg.enChargingTime = LcdChargeTime1Clk;
    /* LCD Charge mode select,Quick charging mode. */
    stcLcdInitconfig.stcLcdDriveCircuit.stcLcdInrCfg.enChargingMode = LcdChargeModeLargeCur;
    /* LCD drive power select Normal. */
    stcLcdInitconfig.enLcdDriveCap = LcdDriveCapabilityNormal;

    /* LCD Port config. */
    stcLcdInitconfig.stcSeg0_Seg7Config.SEGE0 = ENABLE;
    stcLcdInitconfig.stcSeg0_Seg7Config.SEGE1 = ENABLE;
    stcLcdInitconfig.stcSeg0_Seg7Config.SEGE2 = ENABLE;
    stcLcdInitconfig.stcSeg0_Seg7Config.SEGE5 = ENABLE;
    stcLcdInitconfig.stcSeg0_Seg7Config.SEGE6 = ENABLE;
    stcLcdInitconfig.stcSeg0_Seg7Config.SEGE7 = ENABLE;
    stcLcdInitconfig.stcSeg8_Seg15Config.SEGE13 = ENABLE;
    stcLcdInitconfig.stcSeg16_Seg23Config.SEGE20 = ENABLE;

    /* According to the initialization configuration structure LCD registers. */
    enresult = Lcd_InitConfig( &stcLcdInitconfig );

    return enresult;
}

static rt_err_t rt_lcd_open(rt_device_t device, rt_uint16_t oflag)
{
    Lcd_Cmd(ENABLE);
    
    return RT_EOK;
}

static rt_err_t rt_lcd_close(rt_device_t device)
{
    Lcd_Cmd(DISABLE);

    return RT_EOK;
}

static rt_size_t rt_lcd_write(rt_device_t device, rt_off_t pos, const void* buffer, rt_size_t size)
{
    char *ptr = (char*) buffer;

    RT_ASSERT(ARRAY_SZ(lcd_group) > pos);
    
    if(*ptr >= '0' && *ptr <= '9')
    {
        LCD_WriteDdramIndexByte(lcd_group[pos].first_seg_idx, lcd_content[*ptr - '0'].first_seg_content);
        LCD_WriteDdramIndexByte(lcd_group[pos].second_seg_idx, lcd_content[*ptr - '0'].second_seg_content);
    }
    
    return (rt_size_t) ptr - (rt_size_t) buffer;
}

int rt_hw_lcd_init(void)
{
        hc32_lcd_t *lcd = RT_NULL;

        /* get uart device */
        lcd = &lcd_instance;

        /* device initialization */
        lcd->parent.type = RT_Device_Class_Miscellaneous;
        
        /* device interface */
        lcd->parent.init 	    = rt_lcd_init;
        lcd->parent.open 	    = rt_lcd_open;
        lcd->parent.close       = rt_lcd_close;
        lcd->parent.read 	    = RT_NULL;
        lcd->parent.write       = rt_lcd_write;
        lcd->parent.control     = RT_NULL;
        lcd->parent.user_data   = RT_NULL;

        rt_device_register(&lcd->parent, lcd->name, RT_DEVICE_FLAG_WRONLY);
    return 0;
}
INIT_BOARD_EXPORT(rt_hw_lcd_init);

#endif /* RT_USING_LCD */
