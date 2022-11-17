#ifndef NATIVE
#include <Arduino.h>
#endif

#include <unity.h>

#include <string>
#include <algorithm>
#include <iterator>
#include <vector>

#include <B64.h>
#include <CompanionSatellite.h>

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

/*
void test_Satellite_BEGIN()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  std::string input = "BEGIN CompanionVersion=2.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.0\nBEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=7.2.0\n";
  int a = cs.parseData(input.data());

  // 2 commands
  TEST_ASSERT_EQUAL_INT32_MESSAGE(2, cs._cmd_buffer.size(), input.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(2, cs._cmd_buffer.at(0).parm.size(), input.data());    // 2 parms
  TEST_ASSERT_EQUAL_INT32_MESSAGE(1, cs._cmd_buffer.at(0).cmd, input.data());            // BEGIN
  TEST_ASSERT_EQUAL_INT32_MESSAGE(3, cs._cmd_buffer.at(0).parm.at(0).arg, input.data()); // CompanionVersion
  TEST_ASSERT_EQUAL_STRING_MESSAGE("2.3.1+4641-v2-3.1-dc01ac7c", cs._cmd_buffer.at(0).parm.at(0).val.data(), input.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(0, cs._cmd_buffer.at(0).parm.at(1).arg, input.data()); // ApiVersion
  TEST_ASSERT_EQUAL_STRING_MESSAGE("1.2.0", cs._cmd_buffer.at(0).parm.at(1).val.data(), input.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(2, cs._cmd_buffer.at(1).parm.size(), input.data());    // 2 parms
  TEST_ASSERT_EQUAL_INT32_MESSAGE(1, cs._cmd_buffer.at(1).cmd, input.data());            // BEGIN
  TEST_ASSERT_EQUAL_INT32_MESSAGE(3, cs._cmd_buffer.at(1).parm.at(0).arg, input.data()); // CompanionVersion
  TEST_ASSERT_EQUAL_STRING_MESSAGE("8.3.1+4641-v2-3.1-dc01ac7c", cs._cmd_buffer.at(1).parm.at(0).val.data(), input.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(0, cs._cmd_buffer.at(1).parm.at(1).arg, input.data()); // ApiVersion
  TEST_ASSERT_EQUAL_STRING_MESSAGE("7.2.0", cs._cmd_buffer.at(1).parm.at(1).val.data(), input.data());
}

void test_Satellite_ADDDEVICE()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  cs.addDevice();
  TEST_ASSERT_EQUAL_STRING("ADD-DEVICE DEVICEID=1234 PRODUCT_NAME=\"Test\" KEYS_TOTAL=4 KEYS_PER_ROW=2 BITMAPS=0 COLORS=0 TEXT=0\n", cs.transmitBuffer.data());
  std::string input = "BEGIN CompanionVersion=2.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.0\n";
  int a = cs.parseData(input.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(1, cs._cmd_buffer.size(), input.data());               // 1 Command
  TEST_ASSERT_EQUAL_INT32_MESSAGE(1, cs._cmd_buffer.at(0).cmd, input.data());            // BEGIN
  TEST_ASSERT_EQUAL_INT32_MESSAGE(2, cs._cmd_buffer.at(0).parm.size(), input.data());    // 2 parms
  TEST_ASSERT_EQUAL_INT32_MESSAGE(3, cs._cmd_buffer.at(0).parm.at(0).arg, input.data()); // CompanionVersion
  TEST_ASSERT_EQUAL_STRING_MESSAGE("2.3.1+4641-v2-3.1-dc01ac7c", cs._cmd_buffer.at(0).parm.at(0).val.data(), input.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(0, cs._cmd_buffer.at(0).parm.at(1).arg, input.data()); // ApiVersion
  TEST_ASSERT_EQUAL_STRING_MESSAGE("1.2.0", cs._cmd_buffer.at(0).parm.at(1).val.data(), input.data());

  cs._cmd_buffer.clear();

  input = "ADD-DEVICE OK DEVICEID=\"1234\"\nBRIGHTNESS DEVICEID=1234 VALUE=100\nKEY-STATE DEVICEID=1234    KEY=0 TYPE=PAGEUP  PRESSED=false\nKEY-STATE DEVICEID=1234 KEY=1 TYPE=BUTTON  PRESSED=false\nKEY-STATE DEVICEID=1234 KEY=2 TYPE=PAGENUM  PRESSED=false\nKEY-STATE DEVICEID=1234 KEY=3 TYPE=BUTTON  PRESSED=false\n";

  TEST_ASSERT_EQUAL_INT32_MESSAGE(6, cs.parseData(input.data()), "ret 6");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(6, cs._cmd_buffer.size(), "6 Commands");

  TEST_ASSERT_EQUAL_INT32_MESSAGE(0, cs._cmd_buffer.at(0).cmd, "ADD-DEVICE");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(2, cs._cmd_buffer.at(0).parm.size(), "2 Parm");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(8, cs._cmd_buffer.at(0).parm.at(0).arg, "OK");
  TEST_ASSERT_EQUAL_STRING("t", cs._cmd_buffer.at(0).parm.at(0).val.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(4, cs._cmd_buffer.at(0).parm.at(1).arg, "DEVICEID");
  TEST_ASSERT_EQUAL_STRING("1234", cs._cmd_buffer.at(0).parm.at(1).val.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(2, cs._cmd_buffer.at(1).cmd, "BRIGHTNESS");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(2, cs._cmd_buffer.at(1).parm.size(), "2 Parm");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(4, cs._cmd_buffer.at(1).parm.at(0).arg, "DEVICEID");
  TEST_ASSERT_EQUAL_STRING("1234", cs._cmd_buffer.at(1).parm.at(0).val.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(12, cs._cmd_buffer.at(1).parm.at(1).arg, "VALUE");
  TEST_ASSERT_EQUAL_STRING("100", cs._cmd_buffer.at(1).parm.at(1).val.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(4, cs._cmd_buffer.at(2).cmd, "KEY-STATE");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(4, cs._cmd_buffer.at(2).parm.size(), "4 Parm");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(4, cs._cmd_buffer.at(2).parm.at(0).arg, "DEVICEID");
  TEST_ASSERT_EQUAL_STRING("1234", cs._cmd_buffer.at(2).parm.at(0).val.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(7, cs._cmd_buffer.at(2).parm.at(1).arg, "KEY");
  TEST_ASSERT_EQUAL_STRING("0", cs._cmd_buffer.at(2).parm.at(1).val.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(11, cs._cmd_buffer.at(2).parm.at(2).arg, "TYPE");
  TEST_ASSERT_EQUAL_STRING("PAGEUP", cs._cmd_buffer.at(2).parm.at(2).val.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(9, cs._cmd_buffer.at(2).parm.at(3).arg, "PRESSED");
  TEST_ASSERT_EQUAL_STRING("false", cs._cmd_buffer.at(2).parm.at(3).val.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(4, cs._cmd_buffer.at(5).cmd, "KEY-STATE");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(4, cs._cmd_buffer.at(5).parm.size(), "4 Parm");
  TEST_ASSERT_EQUAL_INT32_MESSAGE(4, cs._cmd_buffer.at(5).parm.at(0).arg, "DEVICEID");
  TEST_ASSERT_EQUAL_STRING("1234", cs._cmd_buffer.at(5).parm.at(0).val.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(7, cs._cmd_buffer.at(5).parm.at(1).arg, "KEY");
  TEST_ASSERT_EQUAL_STRING("3", cs._cmd_buffer.at(5).parm.at(1).val.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(11, cs._cmd_buffer.at(5).parm.at(2).arg, "TYPE");
  TEST_ASSERT_EQUAL_STRING("BUTTON", cs._cmd_buffer.at(5).parm.at(2).val.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(9, cs._cmd_buffer.at(5).parm.at(3).arg, "PRESSED");
  TEST_ASSERT_EQUAL_STRING("false", cs._cmd_buffer.at(5).parm.at(3).val.data());
}
*/

void test_maintainConnection_noData()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  std::string input = "PON";
  int ret = cs.maintainConnection(0);
  TEST_ASSERT_EQUAL_INT32(0, ret);
  ret = cs.maintainConnection(0, input.data());
  TEST_ASSERT_EQUAL_INT32(0, ret);
}

void test_maintainConnection_msgBeforBegin()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  std::string input = "ADD-DEVICE CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.0\n";
  int ret = cs.maintainConnection(0, input.data());
  TEST_ASSERT_FALSE(cs.isConnected());
}

void test_maintainConnection_beginWrongVersion()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  std::string input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=2.2.4\n";
  int ret = cs.maintainConnection(0, input.data());
  TEST_ASSERT_FALSE(cs.isConnected());

  input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.1.4\n";
  ret = cs.maintainConnection(0, input.data());
  TEST_ASSERT_FALSE(cs.isConnected());
}

void test_maintainConnection_beginOK()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  std::string input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.4\n";
  int ret = cs.maintainConnection(0, input.data());
  TEST_ASSERT_TRUE_MESSAGE(cs.isConnected(), "is connected");
  TEST_ASSERT_FALSE_MESSAGE(cs.isActive(), "is not active");
}

void test_maintainConnection_beginTimeout()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  std::string input = "BEGIN CompanionVersion=8.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.4\n";
  int ret = cs.maintainConnection(0, input.data());
  TEST_ASSERT_TRUE_MESSAGE(cs.isConnected(), "is connected");
  TEST_ASSERT_FALSE_MESSAGE(cs.isActive(), "is not active");
  
  cs.maintainConnection(500);
  TEST_ASSERT_TRUE_MESSAGE(cs.isConnected(), "not timeout");
  cs.maintainConnection(1600);
  TEST_ASSERT_FALSE_MESSAGE(cs.isConnected(), "is timeout");

}

int main(int argc, char **argv)
{
  UNITY_BEGIN();
  RUN_TEST(test_encode_b64);
  RUN_TEST(test_decode_b64);

  // RUN_TEST(test_Satellite_BEGIN);
  // RUN_TEST(test_Satellite_ADDDEVICE);

  RUN_TEST(test_maintainConnection_noData);
  RUN_TEST(test_maintainConnection_msgBeforBegin);
  RUN_TEST(test_maintainConnection_beginWrongVersion);
  RUN_TEST(test_maintainConnection_beginOK);
  RUN_TEST(test_maintainConnection_beginTimeout);

  UNITY_END();
}
