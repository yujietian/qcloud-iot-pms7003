#include <stdio.h>
#include <stdint.h>
#include <string.h>
#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>
#include "hal_uart.h"

#define PM25 1
#define DATA_LEN 13
#define BUFF_LEN 256
#define UART_DEVICE "/dev/ttyUSB0"

#pragma pack(1)
typedef struct {
    uint8_t  startChar1;
    uint8_t  startChar2;
    uint16_t length;
    uint16_t data[DATA_LEN];
    uint16_t checkCode;
}pms7003Data_t;

typedef struct {
    uint8_t  startChar1;
    uint8_t  startChar2;
    uint8_t  cmd;
    uint16_t data;
    uint16_t checkCode;
}pms7003Ctl_t;
#pragma pack()

uint16_t getCheckCode(uint8_t *data, uint16_t len)
{
    uint16_t i = 0;
    uint16_t checkCode = 0;

    if(NULL != data) {
        for(i=0; i<len-sizeof(uint16_t); i++) {
            checkCode += data[i];
        }
    }

    return checkCode;
}

uint16_t exchangeWord(uint16_t src)
{
    return ntohs(src);
}

void showBuff(uint8_t *buff, uint16_t len)
{
    uint16_t i = 0;

    printf("recvBuff:");
    for(i=0; i<len; i++) {
        printf("%x ", (uint8_t)buff[i]);
    }
    printf("\n");

}

int pms7003Init(void)
{
    char recvBuff[BUFF_LEN] = {0};
    int uartHandl = 0;
    //int fd = 0;
    //int i = 0;
    int recvLen = 0;
    fd_set recvFDs;
    pms7003Data_t *pmsData = NULL;
    struct timeval timeOut;

    uartHandl = serial_open(UART_DEVICE, 9600, 8, 'N', 1);
    if(uartHandl < 0) {
        printf("Failed to open %s\n", UART_DEVICE);
    }

    //printf("uart handle is %d\n", uartHandl);

    while(1) {
        FD_ZERO(&recvFDs);
        FD_SET(uartHandl, &recvFDs);

        //delay 1S
        timeOut.tv_sec = 1;
        timeOut.tv_usec = 0;
        if(select(uartHandl+1, &recvFDs, NULL, NULL, &timeOut) > 0) {
            if(FD_ISSET(uartHandl, &recvFDs)) {
                memset((uint8_t *)recvBuff, 0, BUFF_LEN);
                recvLen = serial_read(uartHandl, (unsigned char*)recvBuff, BUFF_LEN);
                if(recvLen > 0) {
                    showBuff((uint8_t *)recvBuff, sizeof(pms7003Data_t));
                    pmsData = (pms7003Data_t *)recvBuff;
                    printf("checkCode/getCheckCode = %x,%x\n", exchangeWord(pmsData->checkCode), getCheckCode((uint8_t *)recvBuff, sizeof(pms7003Data_t)));
                    if(exchangeWord(pmsData->checkCode) == getCheckCode((uint8_t *)recvBuff, sizeof(pms7003Data_t))) {
                        printf("PM2.5 Value is %d\n", exchangeWord(pmsData->data[PM25]));
                    }
                }
            }
        }
    }

    serial_close(uartHandl);

    return 0;
}
