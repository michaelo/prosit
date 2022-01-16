
TEST(GenericTest, extract_login_from_uri)
{
    char input1[] = "http://plain.com/"; // no user/pass
    char input2[] = "http://user1:pass1@plain.com/"; // got user/pass
    char input2s[] = "https://user2:pass2@plain.com/"; // got user/pass
    char input3[] = "http://user@plain.com/"; // incomplete, error

    ASSERT_FALSE(extract_login_from_uri(input1, NULL, 0, NULL, 0));
    char username[32];
    char password[32];
    ASSERT_TRUE(extract_login_from_uri(input2, username, sizeof(username), password, sizeof(password)));
    ASSERT_STREQ(username, "user1");
    ASSERT_STREQ(password, "pass1");
    ASSERT_TRUE(extract_login_from_uri(input2s, username, sizeof(username), password, sizeof(password)));
    ASSERT_STREQ(username, "user2");
    ASSERT_STREQ(password, "pass2");
    ASSERT_FALSE(extract_login_from_uri(input3, NULL, 0, NULL, 0));
}

TEST(GenericTest, mask_login_from_uri)
{
    char input1[] = "http://plain.com/"; // no user/pass
    char input2[] = "https://user2:pass2@plain.com/"; // got user/pass
    char input3[] = "https://plain.com:123/hei@deg"; // no user/pass at proper location

    mask_login_from_uri(input1, sizeof(input1));
    ASSERT_STREQ(input1, "http://plain.com/");
    mask_login_from_uri(input2, sizeof(input2));
    ASSERT_STREQ(input2, "https://*:*@plain.com/");
    mask_login_from_uri(input3, sizeof(input3));
    ASSERT_STREQ(input3, "https://plain.com:123/hei@deg");
}
