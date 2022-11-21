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

    TEST_ASSERT_EQUAL_STRING("Disconnected", cs.getState().data());

    std::string input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.4\n";
    cs.maintainConnection(0, input.data());
    cs.maintainConnection(0);

    TEST_ASSERT_EQUAL_STRING("Pending", cs.getState().data());

    TEST_ASSERT_FALSE_MESSAGE(cs.isActive(), "is not active");
    TEST_ASSERT_TRUE_MESSAGE(cs.isConnected(), "is connected");
    TEST_ASSERT_EQUAL_STRING("ADD-DEVICE DEVICEID=1234 PRODUCT_NAME=\"Test\" KEYS_TOTAL=4 KEYS_PER_ROW=2 BITMAPS=0 COLORS=0 TEXT=0\n", cs.txBuffer.data());
}

void test_maintainConnection_addFirst()
{
    Satellite cs("1234", "Test", 4, 2);
    cs.maintainConnection(0);
    TEST_ASSERT_EQUAL_STRING("Disconnected", cs.getState().data());

    std::string input = "ADD-DEVICE CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.0\n";
    cs.maintainConnection(0, input.data());
    TEST_ASSERT_EQUAL_STRING("Disconnected", cs.getState().data());
    TEST_ASSERT_EQUAL_STRING("REMOVE-DEVICE DEVICEID=1234", cs.txBuffer.data());
    cs.txBuffer.clear();

    input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.4\n";
    cs.maintainConnection(0, input.data());
    cs.maintainConnection(0);

    TEST_ASSERT_EQUAL_STRING("Pending", cs.getState().data());

    TEST_ASSERT_FALSE_MESSAGE(cs.isActive(), "is not active");
    TEST_ASSERT_TRUE_MESSAGE(cs.isConnected(), "is connected");
}

void test_maintainConnection_beginWrongVersion()
{
    Satellite cs("1234", "Test", 4, 2);
    std::string input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=2.2.4\n";
    cs.maintainConnection(0, input.data());
    TEST_ASSERT_EQUAL_STRING_MESSAGE("Disconnected", cs.getState().data(), "should be wrong version");

    input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.4\n";
    cs.maintainConnection(0, input.data());
    TEST_ASSERT_EQUAL_STRING_MESSAGE("Pending", cs.getState().data(), "should be correct version");

    input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.1.4\n";
    cs.maintainConnection(0, input.data());
    TEST_ASSERT_EQUAL_STRING_MESSAGE("Disconnected", cs.getState().data(), "should be wrong version");

    input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.3.4\n";
    cs.maintainConnection(0, input.data());
    TEST_ASSERT_EQUAL_STRING_MESSAGE("Pending", cs.getState().data(), "should be correct version");

}

void test_maintainConnection_beginTimeout()
{
  Satellite cs("1234", "Test", 4, 2);
  std::string input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.4\n";
  int ret = cs.maintainConnection(0, input.data());
  TEST_ASSERT_EQUAL_STRING("Pending", cs.getState().data());
  
  cs.maintainConnection(500);
  TEST_ASSERT_EQUAL_STRING_MESSAGE("Pending", cs.getState().data(), "should NOT be timeout");
  cs.maintainConnection(1600);
  TEST_ASSERT_EQUAL_STRING_MESSAGE("Disconnected", cs.getState().data(), "should be timeout");

}


int main(int argc, char **argv)
{
    UNITY_BEGIN();

    RUN_TEST(test_encode_b64);
    RUN_TEST(test_decode_b64);

    RUN_TEST(test_maintainConnection_beginOK);
    RUN_TEST(test_maintainConnection_addFirst);
    RUN_TEST(test_maintainConnection_beginWrongVersion);
    RUN_TEST(test_maintainConnection_beginTimeout);

    UNITY_END();
}
