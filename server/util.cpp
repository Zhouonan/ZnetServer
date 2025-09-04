#include "util.h"
#include "fiber.h"
namespace ZnetServer {

// 由于GetThreadId已经在util.h中实现为内联函数，这里不需要再实现

uint32_t GetFiberId() {
    // 协程功能暂未实现，返回默认值0
    return 0;
}

} 