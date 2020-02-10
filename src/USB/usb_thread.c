#include <zephyr.h>
#include <stdio.h>
#include <sys/byteorder.h>
#include <sys/ring_buffer.h>
#include <posix/time.h>
#include <posix/sys/time.h>
#include "../constants.h"

#include <drivers/uart.h>
#include "usb_thread.h"
#include "usb_cdc_acm.h"

#define USB_THREAD_PRIORITY    1
/* Size of stack area used by each thread */
#define USB_THREAD_STACKSIZE   2048
K_THREAD_STACK_DEFINE(USB_THREAD_STACK_AREA, USB_THREAD_STACKSIZE);

/* Thread Structure */
static struct k_thread thread_name;

typedef struct {
    uint8_t  packet_type;
    uint8_t  packet_data[1000];
    uint16_t packet_len;
} data_buffer;


/**
 * Send Data to USB
 */
void
send_data_to_usb(data_buffer *usb_outgoing_data, uint8_t itteration_id) {

    memset(usb_outgoing_data->packet_data, 0, 1000);

    usb_outgoing_data->packet_len = 0;
    usb_outgoing_data->packet_data[usb_outgoing_data->packet_len++] = 0x2B;
    usb_outgoing_data->packet_data[usb_outgoing_data->packet_len++] = 'A';

    usb_outgoing_data->packet_data[usb_outgoing_data->packet_len++] =
        itteration_id;

    usb_outgoing_data->packet_len = usb_outgoing_data->packet_len + 926;

    usb_outgoing_data->packet_data[usb_outgoing_data->packet_len++] = 0x2A;
    usb_outgoing_data->packet_data[usb_outgoing_data->packet_len++] = 0x0D;
    usb_outgoing_data->packet_data[usb_outgoing_data->packet_len++] = 0x0A;

    /* Put data in ring buffer */
    ring_buf_put(&usb_data_ringbuf, usb_outgoing_data->packet_data,
                 usb_outgoing_data->packet_len);

    /* Enable TX interrupt in IER */
    uart_irq_tx_enable(usb_dev);
}

/* Entry function of thread */
void
usb_thread(void *arg1, void *arg2, void *arg3)
{
    printk("USB Thread started\n");

    signal_data_buffer incoming_data_buffer;

    data_buffer usb_outgoing_data;
    uint8_t itteration_id = 0;

    while (1) {

        /**
         * Receive a message from a message queue
         *
         * Wait if queue is empty
         */
        k_msgq_get(&data_rx_msgq, &incoming_data_buffer, K_FOREVER);


        send_data_to_usb(&usb_outgoing_data, itteration_id++);
        if (itteration_id == UINT8_MAX) {
            itteration_id = 0;

            printk("Sent 255 packets!\n");
        }
    }
}


/**
 * Start USB Thread
 */
k_tid_t usb_thread_id;
void
start_usb_thread(void)
{

    printk("Created USB Thread !\n");

    /* Creating a thread & running it without any delay*/
    usb_thread_id = k_thread_create(&thread_name, USB_THREAD_STACK_AREA,
                                    K_THREAD_STACK_SIZEOF(
                                        USB_THREAD_STACK_AREA),
                                    usb_thread,
                                    NULL, NULL, NULL,
                                    USB_THREAD_PRIORITY, 0, K_NO_WAIT);

    /* Initialise USB CDC */
    init_cdc_acm();
}