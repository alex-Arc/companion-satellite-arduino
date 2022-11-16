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

void test_Satellite_parseCmdTypeCmd_list()
{
  CompanionSatellite cs("1234", "Test", 4, 2);

  const std::vector<std::string> cmd_list = {
      "ADD-DEVICE ",
      "BEGIN ",
      "BRIGHTNESS ",
      "KEY-PRESS ",
      "KEY-STATE ",
      "KEYS-CLEAR ",
      "PING ",
      "PONG ",
      "REMOVE-DEVICE "};

  for (uint8_t i = 0; i < cmd_list.size(); i++)
  {
    TEST_ASSERT_EQUAL_INT32_MESSAGE(i, cs.parseCmdType(cmd_list.at(i).data()), cmd_list.at(i).data());
  }
}

void test_Satellite_parseCmdTypePass_list()
{
  CompanionSatellite cs("1234", "Test", 4, 2);

  const std::vector<std::string> cmd_list = {
      "ADD-DEVICE \n",
      "BEGIN    ",
      "BRIGHTNESS asdgfas",
      "KEY-PRESS ",
      "KEY-STATE ",
      "KEYS-CLEAR ",
      "PING ",
      "PONG ",
      "REMOVE-DEVICE "};

  for (uint8_t i = 0; i < cmd_list.size(); i++)
  {
    TEST_ASSERT_EQUAL_INT32_MESSAGE(i, cs.parseCmdType(cmd_list.at(i).data()), cmd_list.at(i).data());
  }
}

void test_Satellite_parseCmdTypeWrong_list()
{
  CompanionSatellite cs("1234", "Test", 4, 2);

  const std::vector<std::string> cmd_list = {
      "ADD ",
      "bEGIN ",
      "SSSSS ",
      "KEY-PRESS\n",
      "KEY-STATE564 ",
      "3654KEYS-CLEAR",
      "tttttttttttttttPING",
      "PONGgggggggggggggggg",
      "REMOVEasdhfja-DEVICE"};

  for (uint8_t i = 0; i < cmd_list.size(); i++)
  {
    TEST_ASSERT_EQUAL_INT32_MESSAGE(-1, cs.parseCmdType(cmd_list.at(i).data()), cmd_list.at(i).data());
  }
}

void test_Satellite_parseCmdTypeLong()
{
  CompanionSatellite cs("1234", "Test", 4, 2);

  std::string input = "BEGIN CompanionVersion=3.0.0+5248-develop-6976308b ApiVersion=1.3.0\nADD-DEVICE OK DEVICEID=\" 1234 \"\nBRIGHTNESS DEVICEID=1234 VALUE=100\nKEY-STATE DEVICEID=1234 KEY=0 TYPE=BUTTON  PRESSED=false\nKEY-STATE DEVICEID=1234 KEY=1 TYPE=BUTTON  PRESSED=false\nKEY-STATE DEVICEID=1234 KEY=2 TYPE=BUTTON  PRESSED=false\nKEY-STATE DEVICEID=1234 KEY=3 TYPE=BUTTON  PRESSED=false\n";
  TEST_ASSERT_EQUAL_INT32_MESSAGE(1, cs.parseCmdType(input.data()), "BEGIN");
}

void test_Satellite_parseParameters_Arg_list()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  const std::vector<std::string> list = {
      "ApiVersion ",
      "BITMAP ",
      "COLOR ",
      "CompanionVersion ",
      "DEVICEID ",
      "DIRECTION ",
      "ERROR ",
      "KEY ",
      "OK ",
      "PRESSED ",
      "TEXT ",
      "TYPE "};

  for (uint8_t i = 0; i < list.size(); i++)
  {
    CompanionSatellite::Parm_t a = cs.parseParameters(list.at(i).data());
    TEST_ASSERT_EQUAL_INT32_MESSAGE(i, a.arg, list.at(i).data());
  }
}

void test_Satellite_parseParameters_Pass_list()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  const std::vector<std::string> list = {
      "ApiVersion=1234 ",
      "BITMAP\n",
      "COLOR=1 ",
      "CompanionVersion=1\n",
      "DEVICEID\n",
      "DIRECTION    ",
      "ERROR \n",
      "KEY=wg?\n",
      "OK\n",
      "PRESSED\n",
      "TEXT\n",
      "TYPE\n"};

  for (uint8_t i = 0; i < list.size(); i++)
  {
    CompanionSatellite::Parm_t a = cs.parseParameters(list.at(i).data());
    TEST_ASSERT_EQUAL_INT32_MESSAGE(i, a.arg, list.at(i).data());
  }
}

void test_Satellite_parseParameters_Wrong_list()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  const std::vector<std::string> list = {
      "ApiVersion=1",
      " BITMAP#",
      "\nCOLOR=",
      "Compani3onVersion\n",
      "DEVasdfgICEIDh",
      "DIRECTION5",
      "ERROR+",
      "KEY?",
      "OKOK'",
      "851320PRESSED-",
      "TwEXT.",
      "TYyPE,"};

  for (uint8_t i = 0; i < list.size(); i++)
  {
    CompanionSatellite::Parm_t a = cs.parseParameters(list.at(i).data());
    TEST_ASSERT_EQUAL_INT32_MESSAGE(-1, a.arg, list.at(i).data());
  }
}

void test_Satellite_parseParameters_Value_list()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  std::string input = "ApiVersion=1234 13465";
  CompanionSatellite::Parm_t a = cs.parseParameters(input.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(0, a.arg, input.data());
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE("1234", a.val.data(), 4, input.data());

  input = "ApiVersion=\"1234 13465\"";
  a = cs.parseParameters(input.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(0, a.arg, input.data());
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE("1234 13465", a.val.data(), 10, input.data());

  input = "ApiVersion=\"1234\n13465\"";
  a = cs.parseParameters(input.data());
  TEST_ASSERT_EQUAL_INT32_MESSAGE(0, a.arg, input.data());
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE("1234\n13465", a.val.data(), 10, input.data());
}

void test_Satellite_parseData_Good()
{
  CompanionSatellite cs("1234", "Test", 4, 2);
  std::string input = "BEGIN CompanionVersion=2.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.0\nBEGIN CompanionVersion=2.3.1+4641-v2-3.1-dc01ac7c ApiVersion=1.2.0\n";
  int a = cs.parseData(input);

  TEST_ASSERT_EQUAL_INT32_MESSAGE(2, cs._cmd_buffer.at(0).parm.size(), input.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(1, cs._cmd_buffer.at(0).cmd, input.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(3, cs._cmd_buffer.at(0).parm.at(0).arg, input.data());
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE("2.3.1+4641-v2-3.1-dc01ac7c", cs._cmd_buffer.at(0).parm.at(0).val.data(), 17, input.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(0, cs._cmd_buffer.at(0).parm.at(1).arg, input.data());
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE("1.2.0", cs._cmd_buffer.at(0).parm.at(1).val.data(), 6, input.data());


  TEST_ASSERT_EQUAL_INT32_MESSAGE(2, cs._cmd_buffer.at(1).parm.size(), input.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(1, cs._cmd_buffer.at(1).cmd, input.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(3, cs._cmd_buffer.at(1).parm.at(0).arg, input.data());
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE("2.3.1+4641-v2-3.1-dc01ac7c", cs._cmd_buffer.at(0).parm.at(0).val.data(), 17, input.data());

  TEST_ASSERT_EQUAL_INT32_MESSAGE(0, cs._cmd_buffer.at(1).parm.at(1).arg, input.data());
  TEST_ASSERT_EQUAL_CHAR_ARRAY_MESSAGE("1.2.0", cs._cmd_buffer.at(0).parm.at(1).val.data(), 6, input.data());

}

int main(int argc, char **argv)
{
  UNITY_BEGIN();
  RUN_TEST(test_encode_b64);
  RUN_TEST(test_decode_b64);

  RUN_TEST(test_Satellite_parseCmdTypeCmd_list);
  RUN_TEST(test_Satellite_parseCmdTypePass_list);
  RUN_TEST(test_Satellite_parseCmdTypeWrong_list);
  RUN_TEST(test_Satellite_parseCmdTypeLong);

  RUN_TEST(test_Satellite_parseParameters_Arg_list);
  RUN_TEST(test_Satellite_parseParameters_Pass_list);
  RUN_TEST(test_Satellite_parseParameters_Wrong_list);
  RUN_TEST(test_Satellite_parseParameters_Value_list);

  RUN_TEST(test_Satellite_parseData_Good);
  // RUN_TEST(test_Satellite_addDevice);
  UNITY_END();
}
