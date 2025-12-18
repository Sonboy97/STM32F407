#ifndef YMODEM_CONF_H
#define YMODEM_CONF_H

#ifdef __cplusplus
extern "C" {
#endif

/*-----------------------------------------------------------------------------
 * Ymodem协议库配置文件
 * 用户应根据具体硬件平台和应用需求修改这些配置
 *-----------------------------------------------------------------------------*/

/**
 * @brief 单包接收超时时间（毫秒）
 * @note 接收单个数据包的最大等待时间
 */
#define YMODEM_PACKET_TIMEOUT_MS   1000

/**
 * @brief 最大重试次数
 * @note 传输失败时的最大重试次数
 */
#define YMODEM_MAX_RETRIES         10

/**
 * @brief 最大文件大小（字节）
 * @note 限制接收文件的最大尺寸，防止内存溢出
 */
#define YMODEM_MAX_FILE_SIZE       ((64 + 128) * 1024)  /* 64 + 128 KB */

/**
 * @brief FLASH起始SECTOR
 * @note STM32要檫除的第一个SECTOR
 */
#define YMODEM_FLASH_FIRST_SECTOR  4

/**
 * @brief FLASH的SECTOR数量
 * @note STM32要檫除的SECTOR数量
 */
#define YMODEM_FLASH_SECTORS_NUM   2

#ifdef __cplusplus
}
#endif

#endif /* YMODEM_CONF_H */
