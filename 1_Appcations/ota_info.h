#ifndef __OTA_INFO_H__
#define __OTA_INFO_H__

#include <stdint.h>

/* ==================== 固件信息在 EEPROM(AT24C02, 256 字节) 里的布局 ====================
 *   0x00 ~ 0x3F  新固件信息：出厂预置，之后每次 OTA 成功后刷新，描述当前正在运行的固件
 *   0x40 ~ 0x7F  当前固件信息：本次 OTA 之前设备上原有的固件，也就是外部 flash 里备份的那份
 *   0x80 ~ 0xFF  预留
 * 每条记录 64 字节定长。出厂时 0x00 那条已经按实际固件写好，所以不做 magic 之类的有效性标志；
 * 用 fileSize 判断即可：落在 (0, OTA_APP_SIZE] 之外（空白 0xFF / 读失败 / 全 0）视为记录不可用。 */
#define EEPROM_ADDR_NEW_FW      0x00U
#define EEPROM_ADDR_CUR_FW      0x40U

/* ==================== 固件存放位置 ====================
 * 本工程（bootloader）与升级任务共用这几个地址，所以放头文件里，不要在 .c 里重复定义。
 *
 * 内部 flash 的 APP 区：F407VET6 共 512KB（0x08000000~0x08080000），预留前 400KB 给 bootloader
 * （当前 bootloader 固件实际约 375.5KB，留点余量），APP 区 = 0x08064000~0x08080000，共 112KB。
 * 注意 0x08064000 在扇区7内、不在扇区边界：擦除会连带擦掉扇区内 0x08060000~0x08063FFF 的
 * 空闲预留区（bootloader 止于 0x0805E000，不受影响），新固件必须 ≤ 112KB。 */
#define OTA_APP_ADDR            0x08064000U /* APP 区起始地址（bootloader 预留 400KB） */
#define OTA_APP_SIZE            0x1C000U    /* APP 区大小：512KB - 400KB = 112KB，超了装不下 */

/* 升级前把旧 APP 备份到外部 flash（W25Q64，8MB）的起始地址：只用最前面 112KB（= OTA_APP_SIZE）。
 * 备份长度取 EEPROM 里记录的当前固件 fileSize，所以擦除/写入都按扇区对齐往上取整。 */
#define W25Q64_BACKUP_ADDR      0x000000U

/* 固件信息记录：字段顺序/宽度对齐 UpgradeInfo，可直接 memcpy 云端下发的那几个字段。
 * version/md5 不一定以 '\0' 结尾（云端满 16/32 字节下发时就没有），用的时候按定长拷，
 * 不要 strlen/strcpy。 */
typedef struct FirmwareInfo{
    uint32_t fileSize;      /* 固件字节数，同时决定备份时从 flash 读多少；=0 表示这条记录不可用 */
    char     version[16];   /* 版本号，对应 UpgradeInfo.version */
    char     md5[32];       /* 32 位小写 hex，对应 UpgradeInfo.md5，无 '\0' */
    uint8_t  reserved[12];  /* 凑满 64 字节，留扩展 */
}FirmwareInfo;              /* 4 + 16 + 32 + 12 = 64 */

#endif /* __OTA_INFO_H__ */
