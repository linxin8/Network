#include "../src/serialization.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(Serialization, coder)
{
    SerializationCoder coder;
    coder.push(1l);
    coder.push(233.233);
    coder.push(-12345l);
    coder.push(-12345.123456);
    coder.push("hello world");
    coder.push("my test");
    coder.push("1");
    coder.push(1l);
    coder.push("22");
    SerializationDecoder decoder{coder.extractData()};
    GTEST_ASSERT_EQ(decoder.hasNext(), true);
    GTEST_ASSERT_EQ(decoder.peakNextType(), SerializationType::Int);
    GTEST_ASSERT_EQ(decoder.getNextInt(), 1);
    GTEST_ASSERT_EQ(decoder.getNextDouble(), 233.233);
    GTEST_ASSERT_EQ(decoder.getNextInt(), -12345l);
    GTEST_ASSERT_EQ(decoder.peakNextType(), SerializationType::Double);
    GTEST_ASSERT_EQ(decoder.getNextDouble(), -12345.123456);
    GTEST_ASSERT_EQ(decoder.peakNextType(), SerializationType::String);
    GTEST_ASSERT_EQ(decoder.getNextString(), "hello world");
    GTEST_ASSERT_EQ(decoder.getNextString(), "my test");
    GTEST_ASSERT_EQ(decoder.getNextString(), "1");
    GTEST_ASSERT_EQ(decoder.getNextInt(), 1);
    GTEST_ASSERT_EQ(decoder.getNextString(), "22");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
