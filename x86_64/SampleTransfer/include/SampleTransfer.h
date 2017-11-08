#ifndef SAMPLETRANSFER_H
#define SAMPLETRANSFER_H

int CheckPktTx(uint8_t* data, int data_length, int retryLimit);
int CheckPktRx(uint8_t* data_buf,int retryLimit);
void Tx();
void Rx();

int rx_header(uint8_t* data_buf,int retryLimit);
int tx_header(uint8_t* data_buf, int data_length, int retryLimit);

#endif
