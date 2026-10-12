/*
 * part5_defense.c — 阶段 05：同一漏洞的安全版本（源码层防御）
 *
 * part3_vulnerable.c 的漏洞是 strcpy(buffer, argv[1]) 无边界检查。
 * 本文件把同一个功能（把 argv[1] 拷进固定缓冲区）用三种安全写法重写，
 * 并保留一个"看起来安全其实仍错"的反面教材，讲清楚边界防御的坑。
 *
 * 防御分层（从源码到系统，层层独立，见 README §5）：
 *   本文件：源码层——边界检查 / 安全 API
 *   编译器：-fstack-protector（canary，见 canary_probe.c）
 *   链接器：-z noexecstack（NX，part4 的 shellcode 在这里会段错误而非执行）
 *   内核  ：ASLR（栈地址随机化，part4 的固定猜解几乎必然失败）
 *   语言  ：用 Rust/Go 等内存安全语言从根上消除这类 bug
 *
 * 用法：./defense [1-4] "待拷贝字符串"
 *   1  strncpy + 显式补 NUL（经典，但 strncpy 语义有坑）
 *   2  snprintf（推荐：永远 NUL 结尾，返回值可检测截断）
 *   3  运行时长度检查 + 拒绝服务（宁可不处理也不溢出）
 *   4  反面教材：strncpy(buffer, src, strlen(src)) —— 长度取错，照样溢出
 */
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

#define BUF_SIZE 128

/* ---------- 1. strncpy + 手动补结束符 ---------- */
static void safe_strncpy(const char *src)
{
    char buffer[BUF_SIZE];

    /* strncpy 的两个坑：
     *   a) 源 >= n 时不写结束符 → 必须自己补 buffer[n-1]='\0'
     *   b) 源 < n 时用 NUL 填满剩余（性能，非安全问题）
     * 记住 n 永远是“目标容量”，不是源长度（对比 case 4 的错误）。 */
    strncpy(buffer, src, sizeof(buffer) - 1);
    buffer[sizeof(buffer) - 1] = '\0';
    printf("[1] strncpy: %zu 字节入 %d 字节缓冲（%s）\n",
           strlen(buffer), BUF_SIZE,
           strlen(src) >= BUF_SIZE ? "已截断，返回地址安全" : "完整");
}

/* ---------- 2. snprintf（推荐） ---------- */
static void safe_snprintf(const char *src)
{
    char buffer[BUF_SIZE];

    /* snprintf 总是写 NUL 结尾；返回“本应写入的长度”，
     * 据此可判断是否发生截断——比 strncpy 更易用、更难错。 */
    int want = snprintf(buffer, sizeof(buffer), "%s", src);
    if (want < 0) { perror("snprintf"); return; }
    if ((size_t)want >= sizeof(buffer))
        printf("[2] snprintf: 输入 %d 字节 > 容量 %d，已安全截断\n", want, BUF_SIZE);
    else
        printf("[2] snprintf: %d 字节完整写入\n", want);
}

/* ---------- 3. 先检查长度，拒绝超长输入 ---------- */
static void safe_reject(const char *src)
{
    char buffer[BUF_SIZE];

    /* 最保守策略：超长就拒绝处理，不做任何“尽力而为”的截断。
     * 真实服务里这通常对应“返回 400 / 丢弃报文”，而不是崩溃。 */
    if (strlen(src) >= sizeof(buffer)) {
        fprintf(stderr, "[3] 拒绝: 输入 %zu 字节 >= 容量 %d，不处理\n",
                strlen(src), BUF_SIZE);
        return;
    }
    memcpy(buffer, src, strlen(src) + 1);   /* 已验证长度，memcpy 安全 */
    printf("[3] 通过长度校验，已处理 %zu 字节\n", strlen(buffer));
}

/* ---------- 4. 反面教材：n 取成了源长度 ---------- */
static void still_vulnerable(const char *src)
{
    char buffer[BUF_SIZE];

    puts("[4] 反面教材：strncpy(buffer, src, strlen(src)) —— 用了 strncpy 却照样溢出");
    puts("    教训：边界参数必须是“目标容量”，绝不能是“源长度”。");
    fflush(stdout);
    /* flaw: 第三参数取 strlen(src) 而非 sizeof(buffer)，
     * src 超长时 strncpy 会写满 strlen(src) 字节 → 溢出，
     * 和直接 strcpy 没有区别。编译器不会报错，canary 才拦得住。 */
    strncpy(buffer, src, strlen(src));
    buffer[sizeof(buffer) - 1] = '\0';
    printf("    copied %zu bytes\n", strlen(buffer));
}

int main(int argc, char **argv)
{
    if (argc < 3) {
        fprintf(stderr, "usage: %s [1-4] <string>\n", argv[0]);
        fprintf(stderr, "  1 strncpy  2 snprintf(推荐)  3 长度检查+拒绝  4 反面教材\n");
        return 1;
    }

    const char *src = argv[2];
    switch (atoi(argv[1])) {
    case 1: safe_strncpy(src);     break;
    case 2: safe_snprintf(src);    break;
    case 3: safe_reject(src);      break;
    case 4: still_vulnerable(src); break;
    default:
        fprintf(stderr, "unknown mode\n");
        return 1;
    }
    puts("[defense] 正常返回——返回地址未被破坏。");
    return 0;
}
