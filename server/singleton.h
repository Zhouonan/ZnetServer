#ifndef __ZNS_SINGLETON_H__
#define __ZNS_SINGLETON_H__

namespace ZnetServer {
    /**
     * @brief 单例模式封装类
     * @details T 类型
     *          X “Tag” 类型，用于区分多个不同的单例
     *          N 数字索引
     */
    template<class T, class X = void, int N = 0>
    class Singleton {
    public:
        static T* GetInstance() {
            static T v;
            return &v;
        }    
    };
}

#endif