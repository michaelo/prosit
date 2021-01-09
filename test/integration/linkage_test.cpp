#include "app.h"
#include "test.h"

TEST(LinkageTest, GetVersion) {
    EXPECT_STRNE("", app_version());
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}