/**
 * Sample app for USB CDC ACM class driver. The received data is echoed back
 * to the serial port.
 */
#include <errno.h>
#include <zephyr/types.h>
#include <stddef.h>
#include <stdlib.h>
#include <string.h>
#include <sys/byteorder.h>
#include <zephyr.h>
#include <usb/usb_device.h>
#include "usb_cdc_acm.h"
#include <drivers/uart.h>
#include <sys/ring_buffer.h>
#include "usb_thread.h"
#include "../constants.h"


struct device *usb_dev;

signal_data_buffer usb_data_buffer;

#define USB_RING_BUF_SIZE 18600
u8_t usb_ring_buffer[USB_RING_BUF_SIZE];

struct ring_buf usb_data_ringbuf;


/**
 * USB interrupt handler
 */
static void
interrupt_handler(struct device *dev)
{
    /* Start processing interrupts in ISR & check if any IRQs is pending*/
    while (uart_irq_update(dev) && uart_irq_is_pending(dev)) {

        /* Check if UART RX buffer has a received char */
        if (uart_irq_rx_ready(dev)) {

            /* Read data from UART FIFO */
            usb_data_buffer.packet_len = uart_fifo_read(dev,
                                                        usb_data_buffer.packet_data,
                                                        sizeof(usb_data_buffer.
                                                               packet_data));

            printk("Received UART Length :- %d\n",
                   usb_data_buffer.packet_len);
            usb_data_buffer.packet_type = 0;

            if (usb_data_buffer.packet_len > 0) {

                /* Put message in queue */
                k_msgq_put(&data_rx_msgq, &usb_data_buffer, K_NO_WAIT);
            }

        }

        /* Check if UART TX buffer can accept a new char */
        if (uart_irq_tx_ready(dev)) {
            u8_t buffer[64];
            int rb_len, send_len;

            /* Read data from a usb data from ring buffer */
            rb_len = ring_buf_get(&usb_data_ringbuf, buffer, sizeof(buffer));
            if (!rb_len) {
                /* Disable TX interrupt in IER */
                uart_irq_tx_disable(dev);

                usb_data_buffer.packet_data[0] = 1;
                usb_data_buffer.packet_len = 1;
                k_msgq_put(&data_rx_msgq, &usb_data_buffer, K_NO_WAIT);

                continue;
            }

            /* Fill UART FIFO with data received fronm USB ring buffer & return th number of bytes sent*/
            send_len = uart_fifo_fill(dev, buffer, rb_len);
            if (send_len < rb_len) {
                printk("------------ Drop %d bytes ------------ \n",
                       rb_len - send_len);
            }
        }
    }
}


/**
 * Initialise USB CDC
 */
void
init_cdc_acm(void)
{
    u32_t baudrate, dtr = 0U;

    /* Initialize a ring buffer */
    ring_buf_init(&usb_data_ringbuf, sizeof(usb_ring_buffer), usb_ring_buffer);

    /* Retrieve the device structure for a driver by name */
    usb_dev = device_get_binding("CDC_ACM_0");

    int ret = usb_enable(NULL);
    if (ret != 0) {
        printk("Failed to enable USB\n");
        return;
    }

    /* Waiting for DTR */
    while (true) {
        uart_line_ctrl_get(usb_dev, UART_LINE_CTRL_DTR, &dtr);
        if (dtr) {
            break;
        } else {
            /* Give CPU resources to low priority threads. */
            k_sleep(K_MSEC(100));
        }
    }

    /* They are optional, we use them to test the interrupt endpoint */
    uart_line_ctrl_set(usb_dev, UART_LINE_CTRL_DCD, 1);
    uart_line_ctrl_set(usb_dev, UART_LINE_CTRL_DSR, 1);

    /* Wait 1 sec for the host to do all settings */
    k_busy_wait(1000000);

    /* Get baudrate */
    uart_line_ctrl_get(usb_dev, UART_LINE_CTRL_BAUD_RATE, &baudrate);
    printk("Got Baudrate :- %d\n", baudrate);

    /* Set the IRQ callback function pointer */
    uart_irq_callback_set(usb_dev, interrupt_handler);

    /* Enable rx interrupts */
    uart_irq_rx_enable(usb_dev);
}