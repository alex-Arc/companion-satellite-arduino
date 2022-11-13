#include <unity.h>

#include <B64.h>
#include <CompanionSatellite.h>


CompanionSatellite cs("1234", "Test", 4, 2);


void setUp(void) {
  // set stuff up here
}

void tearDown(void) {
  // clean stuff up here
}

void test_encode_b64()
{
    std::string exp = "VEVTVCAxMjM0CmFuZCBuZXcgbGluZQ==";
    std::string act = B64::encode("TEST 1234\nand new line");
    TEST_ASSERT_EQUAL_STRING(exp.data(),act.data());
}

void test_decode_b64()
{
    std::string exp = "TEST 1234\nand new line";
    std::string act = B64::decode("VEVTVCAxMjM0CmFuZCBuZXcgbGluZQ==");
    TEST_ASSERT_EQUAL_STRING(exp.data(),act.data());
}

void test_SatelliteSetup() 
{
    // cs.addDevice();
    cs.keyDown(0);
    TEST_FAIL_MESSAGE(cs.transmitBuffer.data());
}


int main(int argc, char **argv)
{
    UNITY_BEGIN();
    RUN_TEST(test_encode_b64);
    RUN_TEST(test_decode_b64);
    RUN_TEST(test_SatelliteSetup);
    UNITY_END();
}
