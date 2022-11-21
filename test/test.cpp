#ifndef NATIVE
#include <Arduino.h>
#endif

#include <unity.h>

#include <string>

#include <B64.h>
#include <Satellite.hpp>

void setUp(void)
{
    // set stuff up here
}

void tearDown(void)
{
    // clean stuff up here
}

void test_encode_b64()
{
    std::string exp = "VEVTVCAxMjM0CmFuZCBuZXcgbGluZQ==";
    std::string act = B64::encode("TEST 1234\nand new line");
    TEST_ASSERT_EQUAL_STRING(exp.data(), act.data());
}

void test_decode_b64()
{
    std::string exp = "TEST 1234\nand new line";
    std::string act = B64::decode("VEVTVCAxMjM0CmFuZCBuZXcgbGluZQ==");
    TEST_ASSERT_EQUAL_STRING(exp.data(), act.data());
}

void test_maintainConnection_beginOK()
{
    Satellite cs("1234", "Test", 4, 2);

    TEST_ASSERT_FALSE_MESSAGE(cs.isActive(), "is not active");
    TEST_ASSERT_FALSE_MESSAGE(cs.isConnected(), "is not connected");

    std::string input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.4\n";
    int ret = cs.maintainConnection(0, input.data());
    ret = cs.maintainConnection(0);

    TEST_ASSERT_FALSE_MESSAGE(cs.isActive(), "is not active");
    TEST_ASSERT_TRUE_MESSAGE(cs.isConnected(), "is connected");
}

void test_maintainConnection_noData()
{
    Satellite cs("1234", "Test", 4, 2);
    std::string input = "PON";
    int ret = cs.maintainConnection(0);
    TEST_ASSERT_EQUAL_INT32(0, ret);
    ret = cs.maintainConnection(0, input.data());
    TEST_ASSERT_EQUAL_INT32(0, ret);
}

int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_encode_b64);
    RUN_TEST(test_decode_b64);

    RUN_TEST(test_maintainConnection_beginOK);
    RUN_TEST(test_maintainConnection_noData);

    UNITY_END();
}
