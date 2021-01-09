#include "app.h"
#include "test.h"

TEST(MainTest, do_nothing_with_empty_manifest) {
}

TEST(MainTest, test_https) {
    // Get file from plain http
    // Get unprotected file from https
    // Get basic auth protected http file
    // Get basic auth protected https file
    // Fails if no auth-details provided for auth protected file
}

TEST(MainTest, test_file) {
    // Copy local file to specific dest name
    // Copy local file to folder (keeps original filename)
    // Fails if src does not exist

}

TEST(MainTest, test_git) {
    // Clone git-repo from local path
    // Clone git-repo from remote path
    // Update existing clone
    // Fails if cloning non-existent repo
}


TEST(MainTest, test_manifest_load) {
    // Tests to verify robustness and integrity of manifest parser
}

int main(int argc, char** argv) {
  ::testing::InitGoogleTest(&argc, argv);
  return RUN_ALL_TESTS();
}