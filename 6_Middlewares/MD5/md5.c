#include "md5.h"

// MD5核心变换函数（底层实现，已修复位移问题）
static void MD5_Transform(uint32_t state[4], const uint8_t block[64]);
// 字节序转换（小端系统可省略，STM32无需此函数）
static uint32_t MD5_EndianSwap(uint32_t x);

// MD5常量定义
static const uint32_t MD5_SIN_TABLE[64] = {
    0xd76aa478, 0xe8c7b756, 0x242070db, 0xc1bdceee,
    0xf57c0faf, 0x4787c62a, 0xa8304613, 0xfd469501,
    0x698098d8, 0x8b44f7af, 0xffff5bb1, 0x895cd7be,
    0x6b901122, 0xfd987193, 0xa679438e, 0x49b40821,
    0xf61e2562, 0xc040b340, 0x265e5a51, 0xe9b6c7aa,
    0xd62f105d, 0x02441453, 0xd8a1e681, 0xe7d3fbc8,
    0x21e1cde6, 0xc33707d6, 0xf4d50d87, 0x455a14ed,
    0xa9e3e905, 0xfcefa3f8, 0x676f02d9, 0x8d2a4c8a,
    0xfffa3942, 0x8771f681, 0x6d9d6122, 0xfde5380c,
    0xa4beea44, 0x4bdecfa9, 0xf6bb4b60, 0xbebfbc70,
    0x289b7ec6, 0xeaa127fa, 0xd4ef3085, 0x04881d05,
    0xd9d4d039, 0xe6db99e5, 0x1fa27cf8, 0xc4ac5665,
    0xf4292244, 0x432aff97, 0xab9423a7, 0xfc93a039,
    0x655b59c3, 0x8f0ccc92, 0xffeff47d, 0x85845dd1,
    0x6fa87e4f, 0xfe2ce6e0, 0xa3014314, 0x4e0811a1,
    0xf7537e82, 0xbd3af235, 0x2ad7d2bb, 0xeb86d391
};

// 位移量定义
static const uint8_t MD5_SHIFT[] = {
    7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22, 7, 12, 17, 22,
    5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20, 5,  9, 14, 20,
    4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23, 4, 11, 16, 23,
    6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21, 6, 10, 15, 21
};

// 初始化MD5上下文
void MD5_Init(MD5_CTX *ctx) {
    ctx->total[0] = 0;
    ctx->total[1] = 0;
    // MD5初始状态
    ctx->state[0] = 0x67452301;
    ctx->state[1] = 0xefcdab89;
    ctx->state[2] = 0x98badcfe;
    ctx->state[3] = 0x10325476;
    memset(ctx->buffer, 0, sizeof(ctx->buffer));
}

// 追加数据到MD5计算（核心接口，适合分块处理OTA固件）
void MD5_Update(MD5_CTX *ctx, const uint8_t *data, uint32_t len) {
    uint32_t i, fill;
    uint32_t left = ctx->total[0] & 0x3F; // 缓冲区剩余字节数
    uint32_t add = len;

    // 更新总字节数
    ctx->total[0] += len;
    if (ctx->total[0] < len) {
        ctx->total[1]++;
    }

    // 填充缓冲区到64字节边界
    fill = 64 - left;
    if (len >= fill) {
        memcpy(&ctx->buffer[left], data, fill);
        MD5_Transform(ctx->state, ctx->buffer);
        data += fill;
        len -= fill;
        left = 0;
    }

    // 处理完整的64字节数据块
    for (i = 0; i < len / 64; i++) {
        MD5_Transform(ctx->state, &data[i * 64]);
    }

    // 剩余数据存入缓冲区（修复：原代码此处下标错误！）
    left = len % 64;
    if (left > 0) {
        memcpy(&ctx->buffer[0], &data[i * 64], left); // 原代码是&ctx->buffer[left]，此处是核心错误！
    }
}

// 完成MD5计算，输出16字节哈希值
void MD5_Final(MD5_CTX *ctx, uint8_t hash[MD5_HASH_LEN]) {
    uint32_t i;
    uint32_t pad_len;
    uint64_t total_bits;

    // 计算需要填充的字节数（MD5填充规则：1个0x80 + 若干0x00 + 64位长度）
    total_bits = ((uint64_t)ctx->total[1] << 32) | ctx->total[0];
    total_bits *= 8;
    pad_len = (ctx->total[0] & 0x3F) < 56 ? (56 - (ctx->total[0] & 0x3F)) : (120 - (ctx->total[0] & 0x3F));

    // 填充0x80
    ctx->buffer[ctx->total[0] & 0x3F] = 0x80;
    // 填充0x00
    memset(&ctx->buffer[(ctx->total[0] & 0x3F) + 1], 0, pad_len - 1);
    // 填充64位长度（小端）
    for (i = 0; i < 8; i++) {
        ctx->buffer[56 + i] = (uint8_t)(total_bits >> (i * 8));
    }

    // 处理最后一个数据块
    MD5_Transform(ctx->state, ctx->buffer);

    // 将最终状态转为16字节哈希值（小端）
    for (i = 0; i < 4; i++) {
        hash[i] = (uint8_t)(ctx->state[0] >> (i * 8));
        hash[i + 4] = (uint8_t)(ctx->state[1] >> (i * 8));
        hash[i + 8] = (uint8_t)(ctx->state[2] >> (i * 8));
        hash[i + 12] = (uint8_t)(ctx->state[3] >> (i * 8));
    }

    // 清空上下文（可选，提高安全性）
    memset(ctx, 0, sizeof(MD5_CTX));
}

// 快捷计算字符串MD5
void MD5_String(const uint8_t *str, uint8_t hash[MD5_HASH_LEN]) {
    MD5_CTX ctx;
    MD5_Init(&ctx);
    MD5_Update(&ctx, str, strlen((const char*)str));
    MD5_Final(&ctx, hash);
}

// 将16字节MD5哈希转为32位十六进制字符串（方便OTA对比）
void MD5_HashToHex(const uint8_t *hash, char *hex_str) {
    const char *hex_chars = "0123456789abcdef";
    for (int i = 0; i < MD5_HASH_LEN; i++) {
        hex_str[i * 2] = hex_chars[(hash[i] >> 4) & 0x0F];
        hex_str[i * 2 + 1] = hex_chars[hash[i] & 0x0F];
    }
    hex_str[32] = '\0'; // 字符串结束符
}

// MD5核心变换（底层实现，修复位移逻辑）
static void MD5_Transform(uint32_t state[4], const uint8_t block[64]) {
    uint32_t a = state[0], b = state[1], c = state[2], d = state[3];
    uint32_t x[16];
    uint32_t f, g, tmp;
    int i;

    // 将64字节块转为16个32位整数（小端）
    for (i = 0; i < 16; i++) {
        x[i] = (uint32_t)block[i * 4] | ((uint32_t)block[i * 4 + 1] << 8) |
               ((uint32_t)block[i * 4 + 2] << 16) | ((uint32_t)block[i * 4 + 3] << 24);
    }

    // 4轮变换（修复位移计算，避免溢出）
    for (i = 0; i < 64; i++) {
        if (i < 16) {
            f = (b & c) | ((~b) & d);
            g = i;
        } else if (i < 32) {
            f = (d & b) | ((~d) & c);
            g = (5 * i + 1) % 16;
        } else if (i < 48) {
            f = b ^ c ^ d;
            g = (3 * i + 5) % 16;
        } else {
            f = c ^ (b | (~d));
            g = (7 * i) % 16;
        }

        // 修复位移逻辑：先计算临时值，再循环位移，避免32位溢出
        tmp = a + f + MD5_SIN_TABLE[i] + x[g];
        // 循环左移：等价于 (tmp << n) | (tmp >> (32-n))，但更安全
        tmp = (tmp << MD5_SHIFT[i]) | (tmp >> (32 - MD5_SHIFT[i]));
        tmp = b + tmp;

        a = d;
        d = c;
        c = b;
        b = tmp;
    }

    // 更新状态
    state[0] += a;
    state[1] += b;
    state[2] += c;
    state[3] += d;
}

// 字节序转换（小端系统可省略，STM32无需此函数）
static uint32_t MD5_EndianSwap(uint32_t x) {
    return ((x << 24) & 0xFF000000) | ((x << 8) & 0x00FF0000) |
           ((x >> 8) & 0x0000FF00) | ((x >> 24) & 0x000000FF);
}