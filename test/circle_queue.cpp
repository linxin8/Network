#include "../src/circle_queue.h"
#include <gtest/gtest.h>
#include <iostream>
#include <type_traits>

class Object
{
public:
    Object(int value) : _value{value}
    {
        std::cout << "con " << _value << "\n";
        _count++;
    }
    Object(const Object& right) : _value{right._value}
    {
        std::cout << "copy con " << _value << "\n";
        _count++;
    }
    Object(Object&& right) : _value{right._value}
    {
        std::cout << "move con " << _value << "\n";
        _count++;
        right._value = -1;
    }
    Object& operator=(const Object&) = delete;
    ~Object()
    {
        std::cout << "decon " << _value << "\n";
        _count--;
        _value = -1;
    }
    static int _count;
    int        _value;
};

int Object::_count{};

TEST(CircleQueue, typeAliase)
{
    static_assert(
        std::is_same_v<CircleQueue<Object, 2>, _CircleQueue<Object, 2, false>>);
    static_assert(
        std::is_same_v<CircleQueue<int, 2>, _CircleQueue<int, 2, true>>);
}

TEST(CircleQueue, object)
{
    CircleQueue<Object, 233> queue;
    GTEST_ASSERT_EQ(queue.isEmpty(), true);
    GTEST_ASSERT_EQ(Object::_count, 0);
    Object obj{0};
    GTEST_ASSERT_EQ(Object::_count, 1);
    queue.push(obj);
    GTEST_ASSERT_EQ(queue.getFront()._value, 0);
    GTEST_ASSERT_EQ(Object::_count, 2);
    queue.push(Object{1});
    GTEST_ASSERT_EQ(Object::_count, 3);
    queue.take();
    GTEST_ASSERT_EQ(queue.getFront()._value, 1);
    GTEST_ASSERT_EQ(Object::_count, 2);
    queue.take();
    GTEST_ASSERT_EQ(Object::_count, 1);
}

TEST(CircleQueue, pod)
{
    CircleQueue<int, 233> queue;
    GTEST_ASSERT_EQ(queue.isEmpty(), true);
    queue.push(1);
    queue.push(2);
    queue.push(3);
    GTEST_ASSERT_EQ(queue.isEmpty(), false);
    GTEST_ASSERT_EQ(queue.getSize(), 3);
    GTEST_ASSERT_EQ(queue.take(), 1);
    GTEST_ASSERT_EQ(queue.getFront(), 2);
    queue.push(3);
    GTEST_ASSERT_EQ(queue.take(), 2);
    GTEST_ASSERT_EQ(queue.take(), 3);
    GTEST_ASSERT_EQ(queue.take(), 3);
    GTEST_ASSERT_EQ(queue.isEmpty(), true);
}

TEST(CircleQueue, circle)
{
    CircleQueue<int, 3> queue;
    GTEST_ASSERT_EQ(queue.isEmpty(), true);
    queue.push(1);
    queue.push(2);
    queue.push(3);
    GTEST_ASSERT_EQ(queue.isFull(), true);
    queue.pop();
    queue.push(4);
    GTEST_ASSERT_EQ(queue.isEmpty(), false);
    GTEST_ASSERT_EQ(queue.getSize(), 3);
    GTEST_ASSERT_EQ(queue.take(), 2);
    GTEST_ASSERT_EQ(queue.getFront(), 3);
    queue.push(5);
    GTEST_ASSERT_EQ(queue.take(), 3);
    GTEST_ASSERT_EQ(queue.take(), 4);
    GTEST_ASSERT_EQ(queue.take(), 5);
    GTEST_ASSERT_EQ(queue.isEmpty(), true);
}

TEST(CircleByteStreamQueue, byteStream)
{
    CircleByteStreamQueue queue{10};
    GTEST_ASSERT_EQ(queue.isEmpty(), true);
    queue.push("12345");
    GTEST_ASSERT_EQ(queue.isEmpty(), false);
    GTEST_ASSERT_EQ(std::strcmp(queue.getFirstAddress(), "12345"), 0);
    queue.popN(4);
    queue.push("12345");
    // 5 \0 x x 5 \0 1 2 3 4
    GTEST_ASSERT_EQ(*queue.getRowData(), '5');
    GTEST_ASSERT_EQ(std::strcmp(queue.getFirstAddress(), "5"), 0);
    queue.popN(2);
    GTEST_ASSERT_EQ(std::strncmp(queue.getFirstAddress(), "1234", 4), 0);
    GTEST_ASSERT_EQ(queue.getContinousSize(), 4);
    queue.popContinouData();
    GTEST_ASSERT_EQ(queue.getContinousSize(), 2);
    GTEST_ASSERT_EQ(std::strcmp(queue.getFirstAddress(), "5"), 0);
    // 5 \0 x x x x x x x x
    queue.push("1234");
    // 5 \0 1 2 3 4 \0 x x x
    queue.popN(4);
    // x x x x 3 4 \0 x x x
    queue.push("1234");
    // 4 \0 x x 3 4 \0 1 2 3
    queue.resetCapcity(20);
    // 3 4 \0 1 2 3 4 \0 . . .
    GTEST_ASSERT_EQ(queue.getContinousSize(), 8);
    GTEST_ASSERT_EQ(queue.getSize(), 8);
    GTEST_ASSERT_EQ(queue.getAvailableSize(), 12);
    GTEST_ASSERT_EQ(std::memcmp(queue.getFirstAddress(), "34\0001234\0", 8), 0);
    GTEST_ASSERT_EQ(queue.getFirstAddress(), queue.getRowData());
}

TEST(DynamicCircleByteStreamQueue, byteStream)
{
    DynamicCircleByteStreamQueue queue{10};
    GTEST_ASSERT_EQ(queue.isEmpty(), true);
    queue.push("12345");
    GTEST_ASSERT_EQ(queue.isEmpty(), false);
    GTEST_ASSERT_EQ(std::strcmp(queue.getFirstAddress(), "12345"), 0);
    queue.popN(4);
    queue.push("12345");
    // 5 \0 x x 5 \0 1 2 3 4
    GTEST_ASSERT_EQ(*queue.getRowData(), '5');
    GTEST_ASSERT_EQ(std::strcmp(queue.getFirstAddress(), "5"), 0);
    queue.popN(2);
    GTEST_ASSERT_EQ(std::strncmp(queue.getFirstAddress(), "1234", 4), 0);
    GTEST_ASSERT_EQ(queue.getContinousSize(), 4);
    queue.popContinouData();
    GTEST_ASSERT_EQ(queue.getContinousSize(), 2);
    GTEST_ASSERT_EQ(std::strcmp(queue.getFirstAddress(), "5"), 0);
    // 5 \0 x x x x x x x x
    queue.push("1234");
    // 5 \0 1 2 3 4 \0 x x x
    queue.popN(4);
    // x x x x 3 4 \0 x x x
    queue.push("1234");
    // 4 \0 x x 3 4 \0 1 2 3
    queue.reservePushSize(20);
    // 3 4 \0 1 2 3 4 \0 . . .
    GTEST_ASSERT_EQ(queue.getContinousSize(), 8);
    GTEST_ASSERT_EQ(queue.getSize(), 8);
    GTEST_ASSERT_EQ(std::memcmp(queue.getFirstAddress(), "34\0001234\0", 8), 0);
    GTEST_ASSERT_EQ(queue.getFirstAddress(), queue.getRowData());
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
