#include "gloux/io.h"

int main() {
    using namespace gloux::io;
    auto buffer = ByteBuffer();
    buffer.write(0, ByteBuffer(114514));
    int i = buffer.read(0, 4);
    buffer.write(0, 1919);
    i = buffer.read(0, 4);
    buffer.write(ByteBuffer(810));
    buffer.write(ByteBuffer(std::string("test string")));
    buffer.positive_order(false);
    std::string s = buffer.read(11);
    i = buffer.read(4);
    buffer.write(1000);
    buffer.positive_order(false);
    i = buffer.read<int>();
    ByteBuffer buffer1 = 2000;
    buffer1.positive_order(false);
    buffer1.position(4);
    int i1 = buffer1.read<int>();
}
