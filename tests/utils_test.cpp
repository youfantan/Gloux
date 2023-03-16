#include "gloux/utils.h"
int main() {
    using namespace gloux::utils;
    ThreadPool<64> pool;
    auto future = pool.add([](int a, int b) -> int {
        return a*b;
    }, 20, 45);
    future.wait();
    int i = future.get();
}
