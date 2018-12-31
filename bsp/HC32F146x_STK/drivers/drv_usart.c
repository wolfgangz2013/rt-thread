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
/** \file drv_uart.c
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
#include "uart.h"
#include "gpio.h"
#ifdef RT_USING_UART

/*******************************************************************************
 * Local pre-processor symbols/macros ('#define')
 ******************************************************************************/
#define UART_RX_BUFSZ 32

/*******************************************************************************
 * Local type definitions ('typedef')
 ******************************************************************************/
typedef struct hc32_uart
{
    struct rt_device        parent;
    char                    *name;
    struct rt_ringbuffer    rx_rb;
    rt_uint8_t              uart_chn;
    rt_uint8_t rx_buffer    [UART_RX_BUFSZ];
} hc32_uart_t;

/*******************************************************************************
 * Global variable definitions (declared in header file with 'extern')
 ******************************************************************************/

/*******************************************************************************
 * Local function prototypes ('static')
 ******************************************************************************/

/*******************************************************************************
 * Local variable definitions ('static')
 ******************************************************************************/
#ifdef RT_USING_UART5
static hc32_uart_t uart5 =
{
    .name = RT_CONSOLE_DEVICE_NAME,
    .uart_chn = UARTCH5,
};
#endif
/*******************************************************************************
 * Function implementation - global ('extern') and local ('static')
 ******************************************************************************/
static void uart_gpio_config(void)
{
#ifdef RT_USING_UART5
    Gpio_SetFunc_SIN5_0(0u);
    Gpio_SetFunc_SOT5_0(0u);
#endif
}

#ifdef RT_USING_UART5
static void uart5_rx_isr(void)
{
    uint8_t c = 0;
    hc32_uart_t *uart = &uart5;
    
    /* enter interrupt */
    rt_interrupt_enter();
    
    if(Uart_GetStatus(uart->uart_chn,UartRxFull))
    {
        Uart_ReceiveData(uart->uart_chn, &c);
        rt_ringbuffer_putchar_force(&(uart->rx_rb), (rt_uint8_t)c);
        /* invoke callback */
        if(uart->parent.rx_indicate != RT_NULL)
        {
            uart->parent.rx_indicate(&uart->parent, rt_ringbuffer_data_len(&uart->rx_rb));
        }    
    }
    
    /* leave interrupt */
    rt_interrupt_leave();
}
#endif

static rt_err_t rt_uart_init(rt_device_t device)
{
    hc32_uart_t *uart = RT_NULL;
    stc_uart_config_t uart_config;
    stc_uart_irq_cb_t uart_isr;
    
    RT_ASSERT(device != RT_NULL);
    
    DDL_ZERO_STRUCT(uart_config);
    DDL_ZERO_STRUCT(uart_isr);
    
    uart = (hc32_uart_t *)device;
    
    /* config gpio */
    uart_gpio_config();
    
    /* config uart */
    uart_config.bTouchNvic = TRUE;
    uart_config.enBitDirection = UartDataLsbFirst;
    uart_config.enDataLength = UartEightBits;
    uart_config.enParity = UartParityNone;
    uart_config.enStopBit = UartOneStopBit;
    uart_config.u32BaudRate = 115200;
    
    /* hoot rx isr */
    uart_isr.pfnRxIrqCb = uart5_rx_isr;
    uart_config.pstcIrqCb = &uart_isr;
    
    /* init uart */
    Uart_Init(uart->uart_chn, &uart_config);
    
    return RT_EOK;
}

static rt_err_t rt_uart_open(rt_device_t device, rt_uint16_t oflag)
{
    hc32_uart_t *uart = RT_NULL;
    
    RT_ASSERT(device != RT_NULL);
    
    uart = (hc32_uart_t *)device;

    if(device->flag & RT_DEVICE_FLAG_INT_RX)
    {
        Uart_EnableIrq(uart->uart_chn, UartRxIrq);
    }

    if(device->flag & RT_DEVICE_FLAG_RDWR || device->flag & RT_DEVICE_FLAG_RDONLY)
    {
        Uart_EnableFunc(uart->uart_chn, UartRx);
    }

    if(device->flag & RT_DEVICE_FLAG_RDWR || device->flag & RT_DEVICE_FLAG_WRONLY)
    {
        Uart_EnableFunc(uart->uart_chn, UartTx);
    }
    
    return RT_EOK;
}

static rt_err_t rt_uart_close(rt_device_t device)
{
    hc32_uart_t *uart = RT_NULL;
    
    RT_ASSERT(device != RT_NULL);
    
    uart = (hc32_uart_t *)device;

    if (device->flag & RT_DEVICE_FLAG_INT_RX)
    {
        Uart_DisableIrq(uart->uart_chn, UartRxIrq);
    }
    
    Uart_EnableFunc(uart->uart_chn, UartRx);
    Uart_EnableFunc(uart->uart_chn, UartTx);

    return RT_EOK;
}

static rt_size_t rt_uart_read(rt_device_t device, rt_off_t pos, void* buffer, rt_size_t size)
{
    rt_size_t length;
    rt_base_t level;
    hc32_uart_t *uart = RT_NULL;
    
    RT_ASSERT(device != RT_NULL);
    
    uart = (hc32_uart_t *)device;

    /* disable interrupt */
    level = rt_hw_interrupt_disable();
    
    length = rt_ringbuffer_get(&(uart->rx_rb), buffer, size);
    
    /* enable interrupt */
    rt_hw_interrupt_enable(level);

    return length;
}

static rt_size_t rt_uart_write(rt_device_t device, rt_off_t pos, const void* buffer, rt_size_t size)
{
    char *ptr = (char*) buffer;
    hc32_uart_t *uart = RT_NULL;
    
    RT_ASSERT(device != RT_NULL);
    
    uart = (hc32_uart_t *)device;
    
    if(device->open_flag & RT_DEVICE_FLAG_STREAM)
    {
        /* stream mode */
        while(size--)
        {
            if(*ptr == '\n')
            {
                while(TRUE != Uart_GetStatus(uart->uart_chn,UartTxEmpty));
                Uart_SendData(uart->uart_chn, '\r'); 
            }
            
            while(TRUE != Uart_GetStatus(uart->uart_chn,UartTxEmpty));
            Uart_SendData(uart->uart_chn, *ptr++); 
        }
    }  
    else
    {
        while(size--)
        {
            while(TRUE != Uart_GetStatus(uart->uart_chn,UartTxEmpty));
            Uart_SendData(uart->uart_chn, *ptr++); 
        }
    }

    return (rt_size_t) ptr - (rt_size_t) buffer;
}

int rt_hw_usart_init(void)
{
#ifdef RT_USING_UART5
    {
        hc32_uart_t *uart = RT_NULL;

        /* get uart device */
        uart = &uart5;

        /* device initialization */
        uart->parent.type = RT_Device_Class_Char;

        rt_ringbuffer_init(&(uart->rx_rb), uart->rx_buffer, sizeof(uart->rx_buffer));
        
        /* device interface */
        uart->parent.init 	    = rt_uart_init;
        uart->parent.open 	    = rt_uart_open;
        uart->parent.close      = rt_uart_close;
        uart->parent.read 	    = rt_uart_read;
        uart->parent.write      = rt_uart_write;
        uart->parent.control    = RT_NULL;
        uart->parent.user_data  = RT_NULL;

        rt_device_register(&uart->parent, uart->name, RT_DEVICE_FLAG_RDWR | RT_DEVICE_FLAG_INT_RX);
    }
#endif /* RT_USING_UART5 */
    return 0;
}
INIT_BOARD_EXPORT(rt_hw_usart_init);

#endif /* RT_USING_UART */
