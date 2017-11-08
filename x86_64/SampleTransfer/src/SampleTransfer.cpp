#include <stdio.h>
#include <string.h>
#include <stdlib.h>
#include <stdbool.h>
#include <stdint.h>
#include <string.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/stat.h>
#include <errno.h>
#include <sys/time.h>
#include <iostream>
#include <iomanip>
#include <chrono>

#include "iconv.h"
#include "SampleTransfer.h"

#include "mrloopbf_release.h"

#define BUFSIZE  4096
unsigned char count = 0;

uint8_t check_idx_tx = 1;
uint8_t check_idx_rx = 0;

unsigned long long int tx_cnt = 0;
unsigned long long int fileSize = 0;
unsigned int retransmit_cnt = 0;
float old_speed = 0.0;

uint8_t *recvBuf, *transferBuf;

bool start_idx = false;

std::string myFile;

FILE *tx_fp, *rx_fp;


int main(int argc, char *argv[])
{
	int comm;

	

	comm = getopt(argc, argv, "rt");
	if(comm != -1)
	{
		/*close debug message*/
		ML_HiddenDebugMsg();
		
		/*open devices*/
		int status = ML_Init();
		
		switch(status)
		{
			case 0:
				ML_SetSpeed(7);
				break;
			case 1:
				printf("no devices\n");
				return 0;
				break;
			case 2:
				printf("connect fail\n");
				return 0;
				break;
			default:
				break;
		}

		int USB_gen = ML_GetDevGen();

		switch(USB_gen){
			case 0:
				printf("USB check failed\n");
				break;
			case 1:
				printf("USB gen is 1.1\n");
				break;
			case 2:
				printf("USB gen is 2.0\n");
				break;
			case 3:
				printf("USB gen is 2.1\n");
				break;
			case 4:
				printf("USB gen is 3.0\n");
				break;
			case 5:
				printf("USB gen is 3.1\n");
				break;
			default:
				break;
		}
		
		/*Create tx, rx buffer, this size is 4096 byte*/
		transferBuf = (uint8_t*) malloc(BUFSIZE);
	        recvBuf = (uint8_t*) malloc(BUFSIZE);

		switch(comm)
		{
			case 'r':
			case 'R':
				/*receive set STA mode*/
				ML_SetMode(2);
				start_idx = true;
				//pthread_create(&thread, NULL, Tx, NULL);
				Rx();
				break;
			case 't':
			case 'T':
				if (std::string(argv[2]) == "-f"){
					myFile = std::string(argv[3]);
					std::cout << "Sending file " << myFile << std::endl;
				} else {
					myFile = "./sample.jpg";
					std::cout << "sending sample picture" << std::endl;
				}
				/*transfer set PCP mode*/
				ML_SetMode(1);
				start_idx = true;
				// pthread_create(&thread, NULL, Rx, NULL);
				Tx();
				break;
		}
	}
	else{
		printf("Command error\n");
	}

	if(recvBuf != NULL)	
		free(recvBuf);
	if(transferBuf != NULL)
		free(transferBuf);
	
	return 0;
}

void Tx()
{
	/*The Tx() is send sample.jpg to receive*/
	uint8_t * data = (uint8_t*)malloc(4000);
	tx_fp = fopen(myFile.c_str(), "r+");
	int data_length = 0;

	fseek (tx_fp , 0 , SEEK_END);
	fileSize = ftell (tx_fp);
  	rewind (tx_fp);

  	std::cout << "Filesize: " << fileSize/1000000 << "MB" << std::endl;

	if(tx_fp == NULL)
	{
		start_idx = false;
		printf("Can not open file\n");
	}

	bool send_header = false;
	memcpy(data,&fileSize,sizeof(fileSize));
	memcpy(data+sizeof(fileSize), myFile.c_str(), myFile.length());

	while(!send_header)
	{
		std::cout << "sending header!\n";
		// std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() <<std::endl;
		if(tx_header(data, myFile.length()+sizeof(fileSize), 50) == 1)
		{

			send_header = true;
			std::cout << "Header is send!\n";
			std::cout.flush();
			
		}
	}

	data_length = fread(data, 1, 4000, tx_fp);
	std::chrono::steady_clock::time_point begin1 = std::chrono::steady_clock::now();
	std::chrono::steady_clock::time_point end;
	
	while(start_idx)
	{
		// std::cout << "Time difference = " << std::chrono::duration_cast<std::chrono::nanoseconds> (end - begin).count() <<std::endl;
		if(CheckPktTx(data, data_length, 50) == 1)
		{

			if(data_length < 4000)
			{
				start_idx = false;
				end = std::chrono::steady_clock::now();
				std::cout << "\ntotal speed = " << std::setw(5) << ((float)fileSize*8.0)/std::chrono::duration_cast<std::chrono::microseconds>(end - begin1).count() << " Mbps                                  \n";
				std::cout << "number of retransmitted packets: " << retransmit_cnt << std::endl;
				std::cout.flush();
				fclose(tx_fp);
				
				return;
			}else{
				data_length = fread(data, 1, 4000, tx_fp);
			}
			
		} else {

		}
		
	}
	
}

void Rx()
{
    uint8_t * data = (uint8_t*)malloc(4000);
        memset(data, 1, 4000);
    int length;
    
    char * filename;
    bool received_header = false;
    std::cout<< "receiving header\n";
    std::cout.flush();
    while(!received_header)
        {

        length = rx_header(data, 50);
        std::cout << "Header packet received!\n";
        std::cout.flush();
        if(length > 0)
        {
            std::cout << "hello!\n";
            std::cout.flush();
            filename = new char[length-8];
            memcpy(filename, (const char *)(data+8), length-8);
            memcpy(&fileSize,data,8);
            received_header = true;
            std::cout<< "received header from: " << filename  << " length: " <<fileSize << std::endl;
            std::cout.flush();
        }
        
    }

    rx_fp = fopen((char*)filename, "w+");
    if(rx_fp == NULL)
    {
        fclose(rx_fp);
        rx_fp = fopen(filename, "w+");
    }
    
    while(start_idx)
        {
        length = CheckPktRx(data, 50);
        if(length > 0)
        {
            fwrite(data, 1, length, rx_fp);
            if(length < 4000)
            {
                fclose(rx_fp);
                start_idx = false;
                return;
            }
        }
        }
}

int CheckPktTx(uint8_t* data_buf, int data_length, int retryLimit)
{
	int status;
	int length = BUFSIZE;
	int pkt_length = BUFSIZE;

	/*put app index*/
	transferBuf[3] = 0xE5;
	/*push packet check index*/
	transferBuf[15] = check_idx_tx;

	/*copy data length*/
	memcpy(transferBuf + 16, &data_length, sizeof(int));
	
	/*copy data*/
	memcpy(transferBuf + 20, data_buf, data_length);

	for(int i = 0; i < 4; i++)
	{
		/*The trnasfer packet last 4 byte need to set value 0x00. If set other value the RF packet to lead to error. */
		transferBuf[4092+i] = 0x00;
	}

	float speed = 0.0;
	float alpha = 0.9995;

	std::chrono::steady_clock::time_point end;
	std::chrono::steady_clock::time_point begin = std::chrono::steady_clock::now();

	for(int i = 0; i< retryLimit; i++){
		status = ML_Transfer(transferBuf, pkt_length);
	        if(status > 0)
        	{
			/*Before use ML_Receiver need to set length. In this sample is receive 4096 data. */
			length = BUFSIZE;
			/*wait to receive rx ack and check packet index. */
			status = ML_Receiver(recvBuf, &length);
		        if(status > 0)
				{
					// std::cout << "\r transferred";
                	if(recvBuf[15] == check_idx_tx)
					{
			            check_idx_tx++;
			            tx_cnt++;

			            std::cout << "\rTransfer progress: " <<  std::setw(3) << ((tx_cnt*400000)/fileSize) << "% \t";
			            end = std::chrono::steady_clock::now();
			            // alpha*old_speed + (1.0f-alpha)*
						speed = alpha*old_speed + (1.0f-alpha)*32000.0f/std::chrono::duration_cast<std::chrono::microseconds>(end - begin).count();
						begin = std::chrono::steady_clock::now();
						old_speed = speed;
						std::cout << "current speed = " << std::setw(5) << speed << " Mbps                                  \r";
						std::cout.flush();
						
						return 1;
					} else {
						retransmit_cnt++;
						std::cout << "\rTransfer progress: " <<  std::setw(3) << ((tx_cnt*400000)/fileSize) << "% \t";
						std::cout << "Connection lost, incorrect ACK...            \r";
						std::cout.flush();
						return 0;
					}
		        } else {
		        	std::cout << "\rTransfer progress: " <<  std::setw(3) << ((tx_cnt*400000)/fileSize) << "% \t";
		        	end = std::chrono::steady_clock::now();
			            // alpha*old_speed + (1.0f-alpha)*
					speed =  0.0f;
					begin = std::chrono::steady_clock::now();
					old_speed = speed;
					std::cout << "current speed = " << std::setw(5) << speed << " Mbps    ";
		        	std::cout << "Connection lost, waiting for ACK " << retransmit_cnt << "\r";
		        	std::cout.flush();
		        	retransmit_cnt++;
		        }
			} else {
				std::cout << "\rTransfer progress: " <<  std::setw(3) << ((tx_cnt*400000)/fileSize) << "% \t";
		        std::cout << "Connection lost, transfer incomplete            \r";
		        std::cout.flush();
		        retransmit_cnt++;
			}
    	}

	printf("retry limit\n");
	return 0;
}

int CheckPktRx(uint8_t* data_buf,int retryLimit)
{
	int status;
	int length = BUFSIZE;
	int data_length;
	int pkt_length = BUFSIZE;
	int ack_count = 0;

	for(int i = 0; i< retryLimit; i++)
	{
		/*Before use ML_Receiver need to set length. In this sample is receive 4096 data. */
		length = BUFSIZE;
		status = ML_Receiver(recvBuf, &length);
	        if(status > 0)
        	{
			if(check_idx_rx != recvBuf[15] && recvBuf[3] == 0xE5)
		        {
                		check_idx_rx = recvBuf[15];
				/*push receive packet index to ack buffer*/
		                transferBuf[15] = recvBuf[15];
		                memcpy(&data_length, recvBuf+16, sizeof(int));
				if(data_length > 4000 || data_length <0)
					return 0;
				else{
		        		memcpy(data_buf, recvBuf+20, data_length);
				
					/* Send back ack */
                			status = ML_Transfer(transferBuf, pkt_length);

			                if(status > 0)
			                {
						/* return data length*/
	     					return data_length;
		                	}
				}
			 }else{
				/*handle repeat packet*/
				transferBuf[15] = check_idx_rx;
				/*Please do not often same back repeat packet ack. In this sample set ack count is 16. */
		                if(ack_count == 0 ){
					status = ML_Transfer(transferBuf, pkt_length);
			                if(status < 0)
			                {
						printf("USB error\n");
                        			return 0;
			                }
			                ack_count = 16;
		                }
		                ack_count--;
			}
	        }
	}
	printf("retry limit\n");
	return 0;	

	
}

int rx_header(uint8_t* data_buf,int retryLimit)
{
    int status;
    int length = BUFSIZE;
    int data_length;
    int pkt_length = BUFSIZE;
    int ack_count = 0;

    for(int i = 0; i< retryLimit; i++)
    {
        /*Before use ML_Receiver need to set length. In this sample is receive 4096 data. */
        length = BUFSIZE;
        status = ML_Receiver(recvBuf, &length);
            if(status > 0)
            {
            if(check_idx_rx != recvBuf[15] && recvBuf[3] == 0xE6)
                {
                    std::cout << "Header packet deteceted!\n";
                    std::cout.flush();
                        check_idx_rx = recvBuf[15];
                /*push receive packet index to ack buffer*/
                        transferBuf[15] = recvBuf[15];
                        memcpy(&data_length, recvBuf+16, sizeof(int));
                if(data_length > 4000 || data_length <0)
                    return 0;
                else{
                        memcpy(data_buf, recvBuf+20, data_length);
                        std::cout << "file name written into buffer!\n";
                        std::cout.flush();
                    /* Send back ack */
                            status = ML_Transfer(transferBuf, pkt_length);

                            if(status > 0)
                            {
                        /* return data length*/
                            return data_length;
                            }
                }
             }else{
                /*handle repeat packet*/
                transferBuf[15] = check_idx_rx;
                /*Please do not often same back repeat packet ack. In this sample set ack count is 16. */
                        if(ack_count == 0 ){
                    status = ML_Transfer(transferBuf, pkt_length);
                            if(status < 0)
                            {
                        printf("USB error\n");
                                    return 0;
                            }
                            ack_count = 16;
                        }
                        ack_count--;
            }
            }
    }
    printf("retry limit\n");
    return 0;   
}

int tx_header(uint8_t* data_buf, int data_length, int retryLimit)
{
	int status;
	int length = BUFSIZE;
	int pkt_length = BUFSIZE;

	/*put app index*/
	transferBuf[3] = 0xE6;
	/*push packet check index*/
	transferBuf[15] = check_idx_tx;

	/*copy data length*/
	memcpy(transferBuf + 16, &data_length, sizeof(int));

	long long int file_size;
	memcpy(&file_size,data_buf,8);
	
	char * filename = new char[data_length-8];
	memcpy(filename,(const char *)data_buf+8,data_length-8);
	std::cout << "sending filename: " << filename << std::endl;
	std::cout << "The lenght of this file is " << file_size << " bytes" << std::endl;
	/*copy data*/
	memcpy(transferBuf + 20, data_buf, data_length);

	for(int i = 0; i < 4; i++)
	{
		/*The trnasfer packet last 4 byte need to set value 0x00. If set other value the RF packet to lead to error. */
		transferBuf[4092+i] = 0x00;
	}
	
	for(int i = 0; i< retryLimit; i++){
		status = ML_Transfer(transferBuf, pkt_length);
        if(status > 0) {
			/*Before use ML_Receiver need to set length. In this sample is receive 4096 data. */
			length = BUFSIZE;
			/*wait to receive rx ack and check packet index. */
			status = ML_Receiver(recvBuf, &length);
	        if(status > 0){
				// std::cout << "\r transferred";
            	if(recvBuf[15] == check_idx_tx){
		            check_idx_tx++;
		            tx_cnt++;
					return 1;
				} else {
					return 0;
				}
	        } 
		} 
    }

	printf("retry limit\n");
	return 0;
}