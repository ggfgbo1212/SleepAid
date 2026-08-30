#ifndef __MD5_H
#define __MD5_H

#include <stdint.h>
#include <string.h>

// MD5输出的哈希值长度（16字节，128位）
#define MD5_HASH_LEN 16

// MD5上下文结构体（存储计算过程中的中间状态）
typedef struct {
    uint32_t total[2];          // 已处理的字节数（低32位/高32位）
    uint32_t state[4];          // MD5核心状态（A,B,C,D）
    uint8_t buffer[64];         // 64字节数据块缓冲区
} MD5_CTX;

// 函数声明
void MD5_Init(MD5_CTX *ctx);                       // 初始化MD5上下文
void MD5_Update(MD5_CTX *ctx, const uint8_t *data, uint32_t len);  // 追加数据计算
void MD5_Final(MD5_CTX *ctx, uint8_t hash[MD5_HASH_LEN]);          // 完成计算，输出哈希值

void MD5_String(const uint8_t *str, uint8_t hash[MD5_HASH_LEN]);   // 快捷计算字符串MD5（等于前面3个总作用）
void MD5_File(const uint8_t *file_path, uint8_t hash[MD5_HASH_LEN]);// （可选）计算文件MD5（需适配文件系统,例如FATFS）
void MD5_HashToHex(const uint8_t *hash, char *hex_str);            // 将16字节哈希转为32位十六进制字符串

#endif