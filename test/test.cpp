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
        "ApiVersion",
        "BITMAP",
        "COLOR",
        "CompanionVersion",
        "DEVICEID",
        "DIRECTION",
        "ERROR",
        "KEY",
        "OK",
        "PRESSED",
        "TEXT",
        "TYPE"};

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
        "ApiVersion ",
        "BITMAP#",
        "COLOR=",
        "CompanionVersion\n",
        "DEVICEIDh",
        "DIRECTION5",
        "ERROR+",
        "KEY?",
        "OK'",
        "PRESSED-",
        "TEXT.",
        "TYPE,"};

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
        "ApiVgersion ",
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

// void test_Satellite_addDevice()
// {
//   {
//     CompanionSatellite cs("1234", "Test", 4, 2);
//     cs.addDevice();
//     std::string exp = "ADD-DEVICE DEVICEID=1234 PRODUCT_NAME=\"Test\" KEYS_TOTAL=4 KEYS_PER_ROW=2 BITMAPS=0 COLORS=0 TEXT=0\n";
//     std::string act = cs.transmitBuffer;
//     TEST_ASSERT_EQUAL_STRING(exp.data(), act.data());
//   }
//   {
//     CompanionSatellite cs("5678", "yyyy", 9, 3);
//     cs.addDevice();
//     std::string exp = "ADD-DEVICE DEVICEID=5678 PRODUCT_NAME=\"yyyy\" KEYS_TOTAL=9 KEYS_PER_ROW=3 BITMAPS=0 COLORS=0 TEXT=0\n";
//     std::string act = cs.transmitBuffer;
//     TEST_ASSERT_EQUAL_STRING(exp.data(), act.data());
//   }
// }

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
  // RUN_TEST(test_Satellite_addDevice);
  UNITY_END();
}
