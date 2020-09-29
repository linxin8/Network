#include "../src/file.h"
#include "../src/core_dump.h"
#include <gtest/gtest.h>
#include <iostream>

TEST(File, wirte)
{
    WriteOnlyFile file{"test_txt"};
    char          buffer[100];
    for (int i = 0; i <= 100; i++)
    {
        sprintf(buffer, "%d", i);
        file.append(buffer, strlen(buffer));
    }
    std::cout << "wirte test_txt file with 1-100\n";
}

int main(int argc, char* argv[])
{
    testing::InitGoogleTest(&argc, argv);
    return RUN_ALL_TESTS();
}
