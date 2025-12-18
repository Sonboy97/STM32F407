#include "ymodem.h"
#include "ymodem_conf.h"
#include "usart.h"

/*-----------------------------------------------------------------------------
 * 硬件抽象层实现
 * 用户需要根据实际硬件平台实现这些函数
 *-----------------------------------------------------------------------------*/

/**
 * @brief 串口发送单个字节
 * @param c 要发送的字节
 * @return 成功返回0，失败返回非0
 * @note 用户需要实现具体的串口发送逻辑
 */
int ymodem_uart_putc(uint8_t c)
{
    if (HAL_UART_Transmit(&huart1, &c, 1, 100) == HAL_OK)
    {
        return 0;
    }
    return -1;
}

/**
 * @brief 串口接收单个字节（带超时）
 * @param c 接收到的字节存储位置
 * @param timeout_ms 超时时间（毫秒）
 * @return 成功返回0，超时返回-1
 * @note 用户需要实现带超时的串口接收逻辑
 */
int ymodem_uart_getc(uint8_t *c, uint32_t timeout_ms)
{
    if (HAL_UART_Receive(&huart1, c, 1, timeout_ms) == HAL_OK)
    {
        return 0;
    }
    return -1;
}

/**
 * @brief Flash擦除函数
 * @param addr 要擦除的起始地址
 * @param size 要擦除的大小（字节）
 * @return 成功返回0，失败返回非0
 * @note 用户需要根据目标芯片的Flash控制器实现此函数
 */
int ymodem_flash_erase(uint32_t addr, uint32_t size)
{
    HAL_StatusTypeDef status;
    
    HAL_FLASH_Unlock();
    
    FLASH_EraseInitTypeDef erase_init;
    uint32_t sector_error;
    
    // 计算需要擦除的扇区
    erase_init.TypeErase = FLASH_TYPEERASE_SECTORS;
    erase_init.Sector = YMODEM_FLASH_FIRST_SECTOR;
    erase_init.NbSectors = YMODEM_FLASH_SECTORS_NUM;
    erase_init.VoltageRange = FLASH_VOLTAGE_RANGE_3;
    
    status = HAL_FLASHEx_Erase(&erase_init, &sector_error);
    
    HAL_FLASH_Lock();
    
    return (status == HAL_OK) ? 0 : -1;
}

/**
 * @brief Flash编程函数
 * @param addr 编程起始地址
 * @param data 要编程的数据指针
 * @param size 数据大小（字节）
 * @return 成功返回0，失败返回非0
 * @note 用户需要根据目标芯片的Flash编程特性实现此函数
 */
int ymodem_flash_write(uint32_t addr, const uint8_t *data, uint32_t size)
{
    HAL_StatusTypeDef status = HAL_OK;
    
    HAL_FLASH_Unlock();
    
    for (uint32_t i = 0; i < size; i += 4) {
        uint32_t word_data = *(uint32_t*)(data + i);
        status = HAL_FLASH_Program(FLASH_TYPEPROGRAM_WORD, addr + i, word_data);
        if (status != HAL_OK) {
            break;
        }
    }
    
    HAL_FLASH_Lock();
    
    return (status == HAL_OK) ? 0 : -1;
}
