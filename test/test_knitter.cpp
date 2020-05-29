#include "gtest/gtest.h"

#include "../knitter.h"
#include "knitter/beeper_mock.h"
#include "knitter/encoders_mock.h"
#include "knitter/solenoids_mock.h"

using ::testing::_;
using ::testing::Return;

void onPacketReceived(const uint8_t *buffer, size_t size) {
  (void)buffer;
  (void)size;
}

class KnitterTest : public ::testing::Test {
protected:
  void SetUp() override {
    arduinoMock = arduinoMockInstance();
    beeperMock = beeperMockInstance();
    solenoidsMock = solenoidsMockInstance();
    encodersMock = encodersMockInstance();
    serialMock = serialMockInstance();
    // test_packetSerial->begin(SERIAL_BAUDRATE);
    test_packetSerial.setPacketHandler(&onPacketReceived);
    EXPECT_CALL(*solenoidsMock, init).Times(1);
    k = Knitter(&test_packetSerial);
    ASSERT_EQ(k.getState(), s_init);
  }

  void expect_isr(uint8_t pos) {
    EXPECT_CALL(*encodersMock, encA_interrupt).Times(1);
    EXPECT_CALL(*encodersMock, getPosition).Times(1).WillOnce(Return(pos));
    EXPECT_CALL(*encodersMock, getDirection).Times(1).WillOnce(Return(Right));
    EXPECT_CALL(*encodersMock, getHallActive).Times(1).WillOnce(Return(Left));
    EXPECT_CALL(*encodersMock, getBeltshift).Times(1).WillOnce(Return(Regular));
    EXPECT_CALL(*encodersMock, getCarriage).Times(1).WillOnce(Return(G));
  }

  void expect_indState() {
    EXPECT_CALL(*encodersMock, getHallValue(Left)).Times(1);
    EXPECT_CALL(*encodersMock, getHallValue(Right)).Times(1);
    EXPECT_CALL(*encodersMock, getDirection).Times(1);
  }

  void TearDown() override {
    releaseArduinoMock();
    releaseBeeperMock();
    releaseSolenoidsMock();
    releaseEncodersMock();
    releaseSerialMock();
  }

  ArduinoMock *arduinoMock;
  BeeperMock *beeperMock;
  SolenoidsMock *solenoidsMock;
  EncodersMock *encodersMock;
  SerialMock *serialMock;
  Knitter k;
  SLIPPacketSerial test_packetSerial;
};

TEST(knitter_test, test_constructor) {
  Knitter k = Knitter();
  (void)k;
}

TEST_F(KnitterTest, test_constructor) {
}

TEST_F(KnitterTest, test_isr) {
  expect_isr(1);
  k.isr();
}

TEST_F(KnitterTest, test_startOperation) {
  byte line[] = {1};
  k.startOperation(0, NUM_NEEDLES - 1, false, line);
}

TEST_F(KnitterTest, test_startTest) {
  k.startTest();
}

TEST_F(KnitterTest, test_setNextLine) {
  k.setNextLine(1);
}

TEST_F(KnitterTest, test_setLastLine) {
  k.setLastLine();
}

TEST_F(KnitterTest, test_fsm) {
  // Machine is initialized when left hall sensor is passed in Right direction
  // Inside active needles
  expect_isr(40 + END_OF_LINE_OFFSET_L + 1);
  k.isr();
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF)).Times(1);
  expect_indState();

  // init
  k.fsm();

  // ready
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 0)).Times(1);
  k.fsm();
  ASSERT_EQ(k.getState(), s_ready);

  // operate
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1)).Times(1);
  EXPECT_CALL(*beeperMock, ready).Times(1);
  EXPECT_CALL(*arduinoMock, delay).Times(1);
  EXPECT_CALL(*beeperMock, finishedLine).Times(1);
  byte line[] = {1};
  k.startOperation(0, NUM_NEEDLES - 1, true, line);
  expect_indState();
  EXPECT_CALL(*solenoidsMock, setSolenoid);
  k.fsm();
  ASSERT_EQ(k.getState(), s_operate);

  // Outside of the active needles
  expect_isr(40 + NUM_NEEDLES - 1 + END_OF_LINE_OFFSET_R + 1);
  k.isr();
  EXPECT_CALL(*arduinoMock, digitalWrite(LED_PIN_A, 1)).Times(1);
  expect_indState();
  EXPECT_CALL(*solenoidsMock, setSolenoid).Times(1);
  EXPECT_CALL(*solenoidsMock, setSolenoids(0xFFFF)).Times(1);
  EXPECT_CALL(*beeperMock, finishedLine).Times(1);
  k.setLastLine();
  k.fsm();
}

TEST_F(KnitterTest, test_fsm_test) {
  expect_isr(1);
  k.isr();
  // test
  ASSERT_EQ(k.startTest(), true);
  ASSERT_EQ(k.getState(), s_test);
  EXPECT_CALL(*encodersMock, getHallValue(Left)).Times(1);
  EXPECT_CALL(*encodersMock, getHallValue(Right)).Times(1);
  EXPECT_CALL(*encodersMock, getDirection).Times(1);
  k.fsm();
}