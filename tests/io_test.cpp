#include "gloux/io.h"

void test_bytebuffer() {
    using namespace gloux::io;
    bytebuffer buffer;
    buffer.write(bytebuffer::make(114514));
    int i = 1919;
    buffer.write(bytebuffer::make(i));
    buffer.flip();
    auto data0 = buffer.read(4).get<int>();
    auto data1 = buffer.read(4).get<int>();
    std::string s = "hello world";
    bytebuffer b(s, false);
    b.write("a", 1, 0);
}

int main() {
    test_bytebuffer();
}