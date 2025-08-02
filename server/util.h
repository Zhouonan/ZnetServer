#ifndef __ZNETSERVER_UTIL_H__
#define __ZNETSERVER_UTIL_H__

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <string>
#include <algorithm>

namespace ZnetServer {

/**
 * @brief 获取当前线程ID
 * @return 返回线程ID
 */
inline pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

/**
 * @brief 获取当前协程ID
 * @return 返回协程ID
 */
uint32_t GetFiberId();

/**
 * @brief 将字符串转换为小写
 * @param str 要转换的字符串
 * @return 转换后的字符串
 */
inline std::string to_lower(const std::string& str) {
    std::string lower_str = str;
    std::transform(lower_str.begin(), lower_str.end(), lower_str.begin(), ::tolower);
    return lower_str;
}

}

#endif
