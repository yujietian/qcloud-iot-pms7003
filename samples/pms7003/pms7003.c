#include <sys/time.h>
#include <sys/select.h>
#include <arpa/inet.h>

#include "hal_uart.h"
#include "pms7003.h"

pms7003Info_t pms7003;

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

void pms7003Init(int8_t mode)
{
    int fd = 0;
    int i = 0;
    int result = 0;
    fd_set recvFDs;
    struct timeval timeOut;
    pms7003Ctl_t pms7003Ctl;
    pms7003Data_t *pmsData = NULL;

    pms7003.uartHandle = serial_open(UART_DEVICE, UART_BIT_RATE, 8, 'N', 1);
    if(pms7003.uartHandle < 0) {
        printf("Failed to open %s\n", UART_DEVICE);
    }

    if(mode == 0) {
        pms7003Ctl.startChar1 = PROTO_START_1;
        pms7003Ctl.startChar2 = PROTO_START_2;
        pms7003Ctl.cmd = PROTO_CMD_DATA_MODE;
        pms7003Ctl.data = exchangeWord(PROTO_DATA_MODE_READ);
        pms7003Ctl.checkCode = exchangeWord(getCheckCode((uint8_t *)&pms7003Ctl, sizeof(pms7003Ctl_t)));
        showBuff((uint8_t *)&pms7003Ctl, sizeof(pms7003Ctl_t));

        result = serial_write(pms7003.uartHandle, (uint8_t *)&pms7003Ctl, sizeof(pms7003Ctl_t));
        if(result > 0) {
            while(1) {
                FD_ZERO(&recvFDs);
                FD_SET(pms7003.uartHandle, &recvFDs);
                timeOut.tv_sec = TIMEOUT_RECV_SEC;
                timeOut.tv_usec = TIMEOUT_RECV_USEC;

                if(select(pms7003.uartHandle+1, &recvFDs, NULL, NULL, &timeOut) > 0) {
                    if(FD_ISSET(pms7003.uartHandle, &recvFDs)) {
                        memset(pms7003.recvBuff, 0, BUFF_LEN);
                        pms7003.recvLen = serial_read(pms7003.uartHandle, pms7003.recvBuff, BUFF_LEN);
                        if(pms7003.recvLen > 0) {
                            showBuff(pms7003.recvBuff, sizeof(pms7003Data_t));
                            pmsData = (pms7003Data_t *)pms7003.recvBuff;
                            printf("checkCode/getCheckCode = %x,%x\n", exchangeWord(pmsData->checkCode), getCheckCode(pms7003.recvBuff, sizeof(pms7003Data_t)));
                            if(exchangeWord(pmsData->checkCode) == getCheckCode(pms7003.recvBuff, sizeof(pms7003Data_t))) {
                                printf("PM2.5 Value is %d\n", exchangeWord(pmsData->data[PM25]));
                            }
                        }
                    }
                }
            }
        }
    }
    //printf("uart handle is %d\n", uartHandl);
}

void pms7003Exit(void)
{
    serial_close(pms7003.uartHandle);
}

int16_t pms7003GetValue(int8_t dataType)
{
    int8_t  recvBuff[BUFF_LEN];
    int32_t recvLen;
    fd_set recvFDs;
    int ret = 0;
    int result = 0;
    struct timeval timeOut;
    pms7003Data_t *pmsData = NULL;
    pms7003Ctl_t pms7003Ctl;
    int16_t value = 0;

    pms7003Ctl.startChar1 = PROTO_START_1;
    pms7003Ctl.startChar2 = PROTO_START_2;
    pms7003Ctl.cmd = PROTO_CMD_READ_DATA;
    //pms7003Ctl.data = exchangeWord(PROTO_DATA_MODE_READ);
    pms7003Ctl.checkCode = exchangeWord(getCheckCode((uint8_t *)&pms7003Ctl, sizeof(pms7003Ctl_t)));
    result = serial_write(pms7003.uartHandle, (uint8_t *)&pms7003Ctl, sizeof(pms7003Ctl_t));
    if(result > 0) {
        while(1) {
            FD_ZERO(&recvFDs);
            FD_SET(pms7003.uartHandle, &recvFDs);
            timeOut.tv_sec = TIMEOUT_RECV_SEC;
            timeOut.tv_usec = TIMEOUT_RECV_USEC;
            
            ret = select(pms7003.uartHandle+1, &recvFDs, NULL, NULL, &timeOut);
            if(ret == 0) {
                printf("recv timeout %d.%d\n", timeOut.tv_sec, timeOut.tv_usec);
                break;
            } else if(ret == -1) {
                break;
            } else {
                if(FD_ISSET(pms7003.uartHandle, &recvFDs)) {
                    memset(recvBuff, 0, BUFF_LEN);
                    recvLen = serial_read(pms7003.uartHandle, recvBuff, BUFF_LEN);
                    if(recvLen > 0) {
                        showBuff(recvBuff, sizeof(pms7003Data_t));
                        pmsData = (pms7003Data_t *)recvBuff;
                        printf("checkCode/getCheckCode = %x,%x\n", exchangeWord(pmsData->checkCode), getCheckCode(recvBuff, sizeof(pms7003Data_t)));
                        if(exchangeWord(pmsData->checkCode) == getCheckCode(recvBuff, sizeof(pms7003Data_t))) {
                            value = exchangeWord(pmsData->data[dataType]);
                            printf("PM2.5 Value is %d\n", value);
                            break;
                        }
                    }
                }
            }
        }
    }

    return value;
}
