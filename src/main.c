#include <zephyr.h>
#include <sys/byteorder.h>

#include "USB/usb_cdc_acm.h"
#include "USB/usb_thread.h"
#include "constants.h"

K_MSGQ_DEFINE(data_rx_msgq, sizeof(signal_data_buffer), 5, 1);

void
main(void) {

    /**
     * Start Threads.
     */
    start_usb_thread();
}