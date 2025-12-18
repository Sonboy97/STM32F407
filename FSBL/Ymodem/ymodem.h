#ifndef YMODEM_H
#define YMODEM_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * 头文件包含
 *-----------------------------------------------------------------------------*/
#include <stdint.h>
#include <stdbool.h>
#include <stddef.h>

/*-----------------------------------------------------------------------------
 * 协议常量定义
 *-----------------------------------------------------------------------------*/
#define YMODEM_SOH                  0x01    /**< 128字节数据包起始符 */
#define YMODEM_STX                  0x02    /**< 1024字节数据包起始符 */
#define YMODEM_EOT                  0x04    /**< 传输结束符 */
#define YMODEM_ACK                  0x06    /**< 确认应答 */
#define YMODEM_NAK                  0x15    /**< 否定应答 */
#define YMODEM_CA                   0x18    /**< 取消传输 */
#define YMODEM_C                    0x43    /**< CRC16模式指示符 */

/*-----------------------------------------------------------------------------
 * 错误码定义
 *-----------------------------------------------------------------------------*/
typedef enum {
    YMODEM_OK = 0,                  /**< 操作成功 */
    YMODEM_ERROR_TIMEOUT,           /**< 超时错误 */
    YMODEM_ERROR_CRC,               /**< CRC校验错误 */
    YMODEM_ERROR_SEQUENCE,          /**< 包序号错误 */
    YMODEM_ERROR_PACKET,            /**< 数据包格式错误 */
    YMODEM_ERROR_CANCEL,            /**< 传输被取消 */
    YMODEM_ERROR_FLASH,             /**< Flash操作错误 */
    YMODEM_ERROR_OVERSIZE,          /**< 文件大小超限 */
    YMODEM_ERROR_CHECK,             /**< 文件检查错误 */
    YMODEM_ERROR_SEND,              /**< 发送错误 */
    YMODEM_ERROR_PARAM,             /**< 参数错误 */
} ymodem_result_t;

/*-----------------------------------------------------------------------------
 * 传输状态定义
 *-----------------------------------------------------------------------------*/
typedef enum {
    YMODEM_STATE_IDLE,              /**< 空闲状态 */
    YMODEM_STATE_WAIT_HEADER,       /**< 等待文件头 */
    YMODEM_STATE_RECEIVING,         /**< 接收数据中 */
    YMODEM_STATE_WAIT_EOT1,         /**< 等待第一帧EOT */
    YMODEM_STATE_WAIT_EOT2,         /**< 等待第二帧EOT */
    YMODEM_STATE_WAIT_END,          /**< 等待结束 */
    YMODEM_STATE_COMPLETE,          /**< 传输完成 */
    YMODEM_STATE_ERROR              /**< 错误状态 */
} ymodem_state_t;

/*-----------------------------------------------------------------------------
 * Ymodem句柄结构体
 *-----------------------------------------------------------------------------*/
typedef struct {
    /* 传输控制参数 */
    ymodem_state_t state;           /**< 当前状态 */
    uint32_t flash_start_addr;      /**< Flash起始地址 */
    uint32_t offset_addr;           /**< 写入偏移地址 */
    uint8_t current_seq;            /**< 当前包序号 */

    /* 文件信息 */
    char filename[128];             /**< 文件名 */
    uint32_t filesize;              /**< 文件大小 */
    
    /* 统计信息 */
    uint32_t bytes_received;        /**< 总接收字节数 */
    uint32_t packets_received;      /**< 总接收包数 */
    uint32_t retransmissions;       /**< 重传次数 */
    
    /* 内部缓冲区 */
    uint8_t header_buffer[3];       /**< 帧头缓冲区 */
    uint8_t data_buffer[1024];      /**< 数据缓冲区 */
    uint8_t crc_buffer[2];          /**< CRC缓冲区 */
} ymodem_handle_t;

/*-----------------------------------------------------------------------------
 * 硬件接口函数声明
 * 用户必须在ymodem_port.c中实现这些函数
 *-----------------------------------------------------------------------------*/

/**
 * @brief 串口发送单个字节
 * @param c 要发送的字节
 * @return 成功返回0，失败返回非0
 */
extern int ymodem_uart_putc(uint8_t c);

/**
 * @brief 串口接收单个字节（带超时）
 * @param c 接收到的字节存储位置
 * @param timeout_ms 超时时间（毫秒）
 * @return 成功返回0，超时返回非0
 */
extern int ymodem_uart_getc(uint8_t *c, uint32_t timeout_ms);

/**
 * @brief Flash擦除函数
 * @param addr 要擦除的起始地址
 * @param size 要擦除的大小（字节）
 * @return 成功返回0，失败返回非0
 */
extern int ymodem_flash_erase(uint32_t addr, uint32_t size);

/**
 * @brief Flash编程函数
 * @param addr 编程起始地址
 * @param data 要编程的数据指针
 * @param size 数据大小（字节）
 * @return 成功返回0，失败返回非0
 */
extern int ymodem_flash_write(uint32_t addr, const uint8_t *data, uint32_t size);

/*-----------------------------------------------------------------------------
 * 公共API函数声明
 *-----------------------------------------------------------------------------*/

/**
 * @brief 初始化Ymodem句柄
 * @param handle Ymodem句柄指针
 * @param flash_start_addr Flash起始地址
 * @return 成功返回YMODEM_OK，失败返回错误码
 */
ymodem_result_t ymodem_init(ymodem_handle_t *handle, uint32_t flash_start_addr);

/**
 * @brief 启动Ymodem文件接收
 * @param handle Ymodem句柄指针
 * @return 成功返回YMODEM_OK，失败返回错误码
 * @note 此函数会阻塞直到传输完成或出错
 */
ymodem_result_t ymodem_receive(ymodem_handle_t *handle);

/**
 * @brief 计算CRC16校验值
 * @param data 数据指针
 * @param length 数据长度
 * @return CRC16校验值
 * @note 使用CRC-CCITT多项式(x^16 + x^12 + x^5 + 1)
 */
uint16_t ymodem_calc_crc16(const uint8_t *data, uint32_t length);

#ifdef __cplusplus
}
#endif

#endif /* YMODEM_H */
