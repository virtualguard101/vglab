/*
 * common.h - 通用定义
 *
 * 所有实验程序共用的函数指针表:脆弱目标与攻击端解耦,
 * 各自独立编译/独立进程运行(这是现代漏洞利用的标准形态:
 * 攻击者无法在目标进程里"顺手"帮忙).
 */
#ifndef BOF_LAB_COMMON_H
#define BOF_LAB_COMMON_H

/* 01/02 阶段:脆弱函数 */
typedef void (*overflow_fn_t)(const char *input);

/* 03 阶段:目标程序 */
typedef void (*vuln_entry_t)(int argc, char **argv);

/* 04 阶段:攻击驱动 */
typedef int (*exploit_entry_t)(int argc, char **argv);

/* 默认参数(与论文 exploit2/3/4 的 bsize/offset 思路对应) */
#define DEFAULT_BUF_SIZE  128   /* 03_vulnerable 的栈缓冲区大小 */
#define DEFAULT_BSIZE     256   /* 04_exploit 的 payload 总长 */
#define DEFAULT_OFFSET    0     /* 04_exploit 的栈偏移猜测 */
#define DEFAULT_STEP      16    /* 自动爆破步长:对齐宽度 */
#define DEFAULT_MAX_DISP  2048  /* 自动搜索半径 */

#endif /* BOF_LAB_COMMON_H */
