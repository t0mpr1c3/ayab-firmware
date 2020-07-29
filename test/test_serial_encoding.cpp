/*!`
 * \file test_serial_encoding.cpp
 *
 * This file is part of AYAB.
 *
 *    AYAB is free software: you can redistribute it and/or modify
 *    it under the terms of the GNU General Public License as published by
 *    the Free Software Foundation, either version 3 of the License, or
 *    (at your option) any later version.
 *
 *    AYAB is distributed in the hope that it will be useful,
 *    but WITHOUT ANY WARRANTY; without even the implied warranty of
 *    MERCHANTABILITY or FITNESS FOR A PARTICULAR PURPOSE.  See the
 *    GNU General Public License for more details.
 *
 *    You should have received a copy of the GNU General Public License
 *    along with AYAB.  If not, see <http://www.gnu.org/licenses/>.
 *
 *    Original Work Copyright 2013 Christian Obersteiner, Andreas Müller
 *    Modified Work Copyright 2020 Sturla Lange, Tom Price
 *    http://ayab-knitting.com
 */

#include <gtest/gtest.h>

#include <serial_encoding.h>
#include <machine_mock.h>
#include <knitter_mock.h>

using ::testing::_;
using ::testing::Return;

class SerialEncodingTest : public ::testing::Test {
protected:
  void SetUp() override {
    /*machineMock = machineMockInstance();*/
    knitterMock = knitterMockInstance();
    serialMock = serialMockInstance();
    EXPECT_CALL(*serialMock, begin);
    s = new SerialEncoding();
  }

  void TearDown() override {
    /*releaseMachineMock();*/
    releaseKnitterMock();
    releaseSerialMock();
  }

  /*MachineMock *machineMock;*/
  KnitterMock *knitterMock;
  SerialMock *serialMock;
  SerialEncoding *s;
};

TEST_F(SerialEncodingTest, test_testmsg) {
  uint8_t buffer[] = {reqTest_msgid};
  s->onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(SerialEncodingTest, test_startmsg) {
  uint8_t buffer[] = {reqStart_msgid, 0, 0, 10, 1, 0x74};
  s->onPacketReceived(buffer, sizeof(buffer));
  // checksum wrong
  buffer[5] = 0x73;
  s->onPacketReceived(buffer, sizeof(buffer));
  // kh270
  buffer[1] = 2;
  s->onPacketReceived(buffer, sizeof(buffer));
  // Not enough bytes
  s->onPacketReceived(buffer, sizeof(buffer) - 1);
}

TEST_F(SerialEncodingTest, test_infomsg) {
  uint8_t buffer[] = {reqInfo_msgid};
  s->onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(SerialEncodingTest, test_cnfmsg) {
  // test machine with 200 needles
  knitterMock->getMachine().setMachineType(Kh910);
  uint8_t buffer[30] = {cnfLine_msgid /* 0x42 */, 0, 0, 1,
                        0xde, 0xad, 0xbe, 0xef, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0x00, 0x00, 0x00, 0x00, 0x00,
                        0xa7};  // CRC8
  // CRC8 calculated with
  // http://tomeko.net/online_tools/crc8.php?lang=en

  // Line not accepted
  EXPECT_CALL(*knitterMock, setNextLine);
  s->onPacketReceived(buffer, sizeof(buffer));

  // Line accepted, last line
  /*EXPECT_CALL(*machineMock, lenLineBuffer);*/
  EXPECT_CALL(*knitterMock, setLastLine);
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(true));
  s->onPacketReceived(buffer, sizeof(buffer));

  // Not last line
  buffer[3] = 0x00;
  buffer[29] = 0xc0;
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(true));
  s->onPacketReceived(buffer, sizeof(buffer));

  // crc wrong
  EXPECT_CALL(*knitterMock, setNextLine).Times(0);
  buffer[29]--;
  s->onPacketReceived(buffer, sizeof(buffer));

  // Not enough bytes in buffer
  EXPECT_CALL(*knitterMock, setNextLine).Times(0);
  s->onPacketReceived(buffer, sizeof(buffer) - 1);

  // message for KH270
  knitterMock->getMachine().setMachineType(Kh270);
  uint8_t bufferKh270[20] = {cnfLine_msgid, 0, 0, 1,
                             0xde, 0xad, 0xbe, 0xef, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00,
                             0x00, 0x00, 0x00, 0x00, 0x00,
                             0xab};  // CRC8

  // Line accepted, last line
  /*EXPECT_CALL(*machineMock, lenLineBuffer);*/
  EXPECT_CALL(*knitterMock, setLastLine);
  EXPECT_CALL(*knitterMock, setNextLine).WillOnce(Return(true));
  s->onPacketReceived(bufferKh270, sizeof(bufferKh270));
}

TEST_F(SerialEncodingTest, test_debug) {
  uint8_t buffer[] = {debug_msgid};
  s->onPacketReceived(buffer, sizeof(buffer));
}

TEST_F(SerialEncodingTest, test_constructor) {
}

TEST_F(SerialEncodingTest, test_update) {
  EXPECT_CALL(*serialMock, available);
  s->update();
}

TEST_F(SerialEncodingTest, test_send) {
  EXPECT_CALL(*serialMock, write(_, _));
  EXPECT_CALL(*serialMock, write(SLIP::END));

  uint8_t p[] = {1, 2, 3};
  s->send(p, 3);
}
