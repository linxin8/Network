#include "../src/message.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(Message, test)
{
    Message message;
    message["fun"]  = "int add int int";
    message["arg0"] = "10100";
    message["arg1"] = "23333";
    LOG_DEBUG() << message.toString();
    auto str = message.toString();
    LOG_ASSERT(Message::containsMessage(str));
    auto m = Message::fromString(str);
    LOG_ASSERT(m["fun"] == "int add int int");
    LOG_ASSERT(m["arg0"] == "10100");
    LOG_ASSERT(m["arg1"] == "23333");
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
