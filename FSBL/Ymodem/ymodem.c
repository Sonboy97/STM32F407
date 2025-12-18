#include "ymodem.h"
#include "ymodem_conf.h"
#include <stdlib.h>
#include <string.h>

/**
 * @brief 128字节数据包大小
 * @note Ymodem协议定义的128字节包大小
 */
#define YMODEM_PACKET_SIZE_128     128

/**
 * @brief 1024字节数据包大小
 * @note Ymodem协议定义的1K字节包大小
 */
#define YMODEM_PACKET_SIZE_1K      1024

/*-----------------------------------------------------------------------------
 * 内部函数声明
 *-----------------------------------------------------------------------------*/
static ymodem_result_t send_byte(uint8_t byte);
static ymodem_result_t receive_byte(uint8_t *byte, uint32_t timeout);
static ymodem_result_t receive_packet(ymodem_handle_t *handle, uint16_t *size);
static ymodem_result_t process_file_header(ymodem_handle_t *handle, uint16_t size);
static ymodem_result_t process_file_footer(ymodem_handle_t *handle, uint16_t size);
static ymodem_result_t process_data_packet(ymodem_handle_t *handle, uint32_t size);
static ymodem_result_t process_eot(ymodem_handle_t *handle);

/*-----------------------------------------------------------------------------
 * CRC16计算函数实现
 *-----------------------------------------------------------------------------*/
static uint16_t crc16_update(uint16_t crc_in, uint8_t byte)
{
    uint32_t crc = crc_in;
    uint32_t in = byte | 0x100;

    do {
        crc <<= 1;
        in <<= 1;
        if(in & 0x100)
            ++crc;
        if(crc & 0x10000)
            crc ^= 0x1021;
    } while(!(in & 0x10000));

    return crc & 0xffffu;
}

uint16_t ymodem_calc_crc16(const uint8_t *data, uint32_t length)
{
    uint16_t crc = 0;
    
    /* 遍历所有数据字节 */
    while (length--) {
        crc = crc16_update(crc, *data++);
    }

    crc = crc16_update(crc, 0);
    crc = crc16_update(crc, 0);
    
    return crc;
}

/*-----------------------------------------------------------------------------
 * 发送单个字节
 *-----------------------------------------------------------------------------*/
static ymodem_result_t send_byte(uint8_t byte)
{
    return (ymodem_uart_putc(byte) == 0) ? YMODEM_OK : YMODEM_ERROR_SEND;
}

/*-----------------------------------------------------------------------------
 * 接收单个字节（带超时）
 *-----------------------------------------------------------------------------*/
static ymodem_result_t receive_byte(uint8_t *byte, uint32_t timeout)
{
    return (ymodem_uart_getc(byte, timeout) == 0) ? YMODEM_OK : YMODEM_ERROR_TIMEOUT;
}

/*-----------------------------------------------------------------------------
 * 处理文件头包
 *-----------------------------------------------------------------------------*/
static ymodem_result_t process_file_header(ymodem_handle_t *handle, uint16_t size)
{
    uint32_t idx = 0;
    char *endptr;

    /* 检查包序号 */
    if(handle->header_buffer[1] != 0)
    {
        return YMODEM_ERROR_SEQUENCE;
    }
    
    /* 解析文件名 */
    memset(handle->filename, 0x00, sizeof(handle->filename));
    if(strlen((char *)&handle->data_buffer[idx]) < sizeof(handle->filename))
    {
        memcpy(handle->filename, &handle->data_buffer[idx], strlen((char *)&handle->data_buffer[idx]));
    }
    else
    {
        memcpy(handle->filename, &handle->data_buffer[idx], sizeof(handle->filename)-1);
    }

    idx = strlen((char *)&handle->data_buffer[idx]) + 1;

    if(idx >= size)
    {
        return YMODEM_ERROR_PACKET;
    }
    
    /* 解析文件大小 */
    handle->filesize = strtol((char *)&handle->data_buffer[idx], &endptr, 10);

    idx += (uint32_t)endptr - (uint32_t)&handle->data_buffer[idx] + 1;

    if(idx >= size)
    {
        return YMODEM_ERROR_PACKET;
    }

    /* 检查文件大小是否超出限制 */
    if (handle->filesize > YMODEM_MAX_FILE_SIZE)
    {
        return YMODEM_ERROR_OVERSIZE;
    }
    
    /* 擦除Flash目标区域 */
    if (ymodem_flash_erase(handle->flash_start_addr, handle->filesize) != 0)
    {
        return YMODEM_ERROR_FLASH;
    }
    
    /* 重置写入地址 */
    handle->offset_addr = 0;
    
    /* 更新包序号 */
    handle->current_seq = 1;
    
    return YMODEM_OK;
}

/*-----------------------------------------------------------------------------
 * 处理尾包
 *-----------------------------------------------------------------------------*/
static ymodem_result_t process_file_footer(ymodem_handle_t *handle, uint16_t size)
{
    /* 检查包序号 */
    if (handle->header_buffer[1] != 0)
    {
        return YMODEM_ERROR_SEQUENCE;
    }

    /* 检查数据 */
    for (uint32_t i = 0; i < size; ++i)
    {
        if (handle->data_buffer[i] != 0x00)
        {
            return YMODEM_ERROR_PACKET;
        }
    }

    return YMODEM_OK;
}

/*-----------------------------------------------------------------------------
 * 处理数据包
 *-----------------------------------------------------------------------------*/
static ymodem_result_t process_data_packet(ymodem_handle_t *handle, uint32_t size)
{
    uint32_t write_size = size;
    
    /* 检查包序号 */
    if(handle->header_buffer[1] != handle->current_seq)
    {
        return YMODEM_ERROR_SEQUENCE;
    }
    
    /* 如果是最后一个包，可能需要截断 */
    if (handle->filesize - handle->bytes_received < write_size) 
    {
        write_size = handle->filesize - handle->bytes_received;
    }
    
    /* 写入Flash */
    if (ymodem_flash_write(handle->flash_start_addr + handle->offset_addr, handle->data_buffer, write_size) != 0) 
    {
        return YMODEM_ERROR_FLASH;
    }
    
    /* 更新状态信息 */
    handle->offset_addr += write_size;
    handle->bytes_received += write_size;
    handle->packets_received++;

    /* 更新包序号 */
    handle->current_seq = handle->current_seq >= 255 ? 1 : handle->current_seq + 1;
    
    return YMODEM_OK;
}

/*-----------------------------------------------------------------------------
 * 处理传输结束
 *-----------------------------------------------------------------------------*/
static ymodem_result_t process_eot(ymodem_handle_t *handle)
{
    if(handle->bytes_received != handle->filesize)
    {
        return YMODEM_ERROR_CHECK;
    }

    return YMODEM_OK;
}

/*-----------------------------------------------------------------------------
 * 接收数据包
 *-----------------------------------------------------------------------------*/
static ymodem_result_t receive_packet(ymodem_handle_t *handle, uint16_t *size)
{
    uint16_t crc_received, crc_calculated;
    
    /* 读取包起始字符 */
    if (receive_byte(&handle->header_buffer[0], YMODEM_PACKET_TIMEOUT_MS) != YMODEM_OK)
    {
        return YMODEM_ERROR_TIMEOUT;
    }
    
    /* 包起始字符 */
    if(handle->header_buffer[0] == YMODEM_SOH)
    {
        *size = YMODEM_PACKET_SIZE_128;
    }
    else if(handle->header_buffer[0] == YMODEM_STX)
    {
        *size = YMODEM_PACKET_SIZE_1K;
    }
    else if(handle->header_buffer[0] == YMODEM_EOT || handle->header_buffer[0] == YMODEM_CA)
    {
        return YMODEM_OK;
    }
    else
    {
        return YMODEM_ERROR_PACKET;
    }
    
    /* 读取包序号和补码 */
    for (uint32_t i = 1; i < 3; i++)
    {
        if (receive_byte(&handle->header_buffer[i], YMODEM_PACKET_TIMEOUT_MS) != YMODEM_OK)
        {
            return YMODEM_ERROR_TIMEOUT;
        }
    }
    
    /* 验证包序号：序号 + 补码应该等于0xFF */
    if ((uint8_t)(handle->header_buffer[1] + handle->header_buffer[2]) != 0xFF) 
    {
        return YMODEM_ERROR_SEQUENCE;
    }
    
    /* 读取数据内容 */
    for (uint32_t i = 0; i < *size; i++)
    {
        if (receive_byte(&handle->data_buffer[i], YMODEM_PACKET_TIMEOUT_MS) != YMODEM_OK)
        {
            return YMODEM_ERROR_TIMEOUT;
        }
    }
    
    /* 读取CRC校验值 */
    for (uint32_t i = 0; i < 2; i++)
    {
        if (receive_byte(&handle->crc_buffer[i], YMODEM_PACKET_TIMEOUT_MS) != YMODEM_OK)
        {
            return YMODEM_ERROR_TIMEOUT;
        }
    }
    
    crc_received = (handle->crc_buffer[0] << 8) | handle->crc_buffer[1];
    
    /* 计算并验证CRC */
    crc_calculated = ymodem_calc_crc16(handle->data_buffer, *size);
    if (crc_received != crc_calculated)
    {
        return YMODEM_ERROR_CRC;
    }
    
    return YMODEM_OK;
}

/*-----------------------------------------------------------------------------
 * 初始化Ymodem句柄
 *-----------------------------------------------------------------------------*/
ymodem_result_t ymodem_init(ymodem_handle_t *handle, uint32_t flash_start_addr)
{
    /* 参数检查 */
    if (!handle) {
        return YMODEM_ERROR_PARAM;
    }
    
    /* 设置传输参数 */
    handle->flash_start_addr = flash_start_addr;
    
    /* 初始化状态变量 */
    handle->state = YMODEM_STATE_IDLE;
    handle->current_seq = 0;
    
    /* 初始化文件信息 */
    handle->filename[0] = '\0';
    handle->filesize = 0;
    
    /* 初始化统计信息 */
    handle->bytes_received = 0;
    handle->packets_received = 0;
    handle->retransmissions = 0;
    
    /* 设置当前写入地址 */
    handle->offset_addr = 0;;
    
    return YMODEM_OK;
}

/*-----------------------------------------------------------------------------
 * 启动Ymodem文件接收
 *-----------------------------------------------------------------------------*/
ymodem_result_t ymodem_receive(ymodem_handle_t *handle)
{
    ymodem_result_t result;
    uint16_t packet_size;
    
    /* 参数检查 */
    if (!handle)
    {
        return YMODEM_ERROR_PARAM;
    }
    
    /* 初始化传输状态 */
    handle->state = YMODEM_STATE_WAIT_HEADER;

    /* 主接收循环 */
    while (handle->state != YMODEM_STATE_COMPLETE && handle->state != YMODEM_STATE_ERROR)
    {
        /* 发送初始'C'字符启动CRC模式传输 */
        if (handle->state == YMODEM_STATE_WAIT_HEADER)
        {
            send_byte(YMODEM_C);
        }
        
        /* 接收到包 */
        result = receive_packet(handle, &packet_size);
        if (result == YMODEM_OK) 
        {
            if (handle->header_buffer[0] == YMODEM_SOH || handle->header_buffer[0] == YMODEM_STX) 
            {
                /* 数据帧 */
                if (handle->header_buffer[1] == 0)
                {
                    /* 文件头包 */
                    if (handle->state == YMODEM_STATE_WAIT_HEADER)
                    {
                        result = process_file_header(handle, packet_size);
                        if (result == YMODEM_OK)
                        {
                            handle->state = YMODEM_STATE_RECEIVING;
                            send_byte(YMODEM_ACK);
                            send_byte(YMODEM_C);
                        }
                    }
                    /* 结束包 */
                    else if (handle->state == YMODEM_STATE_WAIT_END)
                    {
                        result = process_file_footer(handle, packet_size);
                        if (result == YMODEM_OK)
                        {
                            handle->state = YMODEM_STATE_COMPLETE;
                            send_byte(YMODEM_ACK);
                        }
                    }
                }
                else
                {
                    /* 数据包 */
                    if (handle->state == YMODEM_STATE_RECEIVING)
                    {
                        result = process_data_packet(handle, packet_size);
                        if (result == YMODEM_OK) 
                        {
                            send_byte(YMODEM_ACK);
                        }
                        if(handle->filesize == handle->bytes_received)
                        {
                            handle->state = YMODEM_STATE_WAIT_EOT1;
                        }
                    }
                }
            }
            else if (handle->header_buffer[0] == YMODEM_EOT)
            {
                /* 传输结束 */
                if (handle->state == YMODEM_STATE_WAIT_EOT1)
                {
                    handle->state = YMODEM_STATE_WAIT_EOT2;
                    send_byte(YMODEM_NAK);
                }
                else if (handle->state == YMODEM_STATE_WAIT_EOT2)
                {
                    result = process_eot(handle);
                    if (result == YMODEM_OK) 
                    {
                        handle->state = YMODEM_STATE_WAIT_END;
                        send_byte(YMODEM_ACK);
                        send_byte(YMODEM_C);
                    }
                }
            }
            else if(handle->header_buffer[0] == YMODEM_CA)
            {
                /* 传输被取消 */
                handle->state = YMODEM_STATE_ERROR;
                result = YMODEM_ERROR_CANCEL;
            }
        }
        
        if (result == YMODEM_OK)
        {
            if(handle->state == YMODEM_STATE_COMPLETE)
            {
                break;
            }
        }
        else if (result == YMODEM_ERROR_TIMEOUT)
        {
            if(handle->state != YMODEM_STATE_WAIT_HEADER)
            {
                send_byte(YMODEM_CA);
                send_byte(YMODEM_CA);
                break;
            }
        }
        else if (result == YMODEM_ERROR_CRC || result == YMODEM_ERROR_SEQUENCE || result == YMODEM_ERROR_PACKET)
        {
            /* 包接收错误，请求重传 */
            if (handle->retransmissions < YMODEM_MAX_RETRIES)
            {
                handle->retransmissions += 1;
                send_byte(YMODEM_NAK);
            }
            else
            {
                send_byte(YMODEM_CA);
                send_byte(YMODEM_CA);
                break;
            }
        }
        else
        {
            send_byte(YMODEM_CA);
            send_byte(YMODEM_CA);
            break;
        }
    }

    return result;
}
