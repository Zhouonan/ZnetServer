#ifndef __ZNETSERVER_UTIL_H__
#define __ZNETSERVER_UTIL_H__

#include <sys/types.h>
#include <unistd.h>
#include <sys/syscall.h>
#include <stdint.h>
#include <string>

namespace ZnetServer {

/**
 * @brief 获取当前线程ID
 * @return 返回线程ID
 */
pid_t GetThreadId() {
    return syscall(SYS_gettid);
}

/**
 * @brief 获取当前协程ID
 * @return 返回协程ID
 */
uint32_t GetFiberId();

}

#endif
