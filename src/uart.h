#pragma once

#include <stdint.h>

void uart_init(void);
const uint8_t* uart_get_rx_buffer_rdy(void);
uint8_t* uart_get_tx_buffer(void);
void uart_send_tx_buffer(uint8_t *tx_buffer, uint8_t ui8_len);
void usart1_reset_received_package (void);

extern bool MtrSerialRX();

#define UART_NUMBER_DATA_BYTES_TO_RECEIVE       29
#define UART_NUMBER_DATA_BYTES_TO_SEND          88

