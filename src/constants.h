typedef struct {
	uint8_t packet_type;
	uint8_t packet_data[30];
	uint16_t packet_len;
} signal_data_buffer;

extern struct ring_buf ble_data_ringbuf;
extern struct k_msgq data_rx_msgq;
