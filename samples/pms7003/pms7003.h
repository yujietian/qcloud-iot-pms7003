#ifndef _PMS7003_H_
#define _PMS7003_H_

#include <stdio.h>
#include <stdint.h>
#include <string.h>

#define PM25 1
#define DATA_LEN 13
#define BUFF_LEN 256
#define TIMEOUT_RECV_SEC  2
#define TIMEOUT_RECV_USEC 0
#define UART_DEVICE "/dev/ttyUSB0"
#define UART_BIT_RATE 9600

#define PROTO_START_1           0x42
#define PROTO_START_2           0x4d
#define PROTO_CMD_DATA_MODE     0xe1
#define PROTO_CMD_READ_DATA     0xe2
#define PROTO_CMD_WORK_MODE     0xe4
#define PROTO_DATA_MODE_READ    0x0000
#define PROTO_DATA_MODE_REPORT  0x0001
#define PROTO_WORK_MODE_SLEEP   0x0000
#define PROTO_WORK_MODE_ACTIVE  0x0001

#pragma pack(1)
typedef struct {
    uint8_t recvBuff[BUFF_LEN];
    int32_t uartHandle;
    int32_t recvLen;
}pms7003Info_t;

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

void pms7003Init(int8_t mode);
void pms7003Exit(void);

int16_t pms7003GetValue(int8_t dataType);

#endif
