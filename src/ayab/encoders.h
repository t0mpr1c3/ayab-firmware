/*!
 * \file encoders.h
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

// TODO(TP): incorporate into machine instead of knitter

#ifndef ENCODERS_H_
#define ENCODERS_H_

#include <Arduino.h>

// Enumerated constants

enum Direction { NoDirection, Left, Right, NUM_DIRECTIONS };
using Direction_t = enum Direction;

enum Carriage { NoCarriage, K, L, G, NUM_CARRIAGES };
using Carriage_t = enum Carriage;

enum Beltshift {
  Unknown,
  Regular,
  Shifted,
  Lace_Regular,
  Lace_Shifted,
  NUM_BELTSHIFTS
};
using Beltshift_t = enum Beltshift;

enum MachineType { Kh910, Kh930, Kh270, NUM_MACHINES };
using Machine_t = enum MachineType;

// Machine constants

constexpr uint8_t NUM_NEEDLES[NUM_MACHINES] = {200, 200, 114};
constexpr uint8_t LINE_BUFFER_LEN[NUM_MACHINES] = {25, 25, 15};
constexpr uint8_t END_OF_LINE_OFFSET_L[NUM_MACHINES] = {12, 12, 6};
constexpr uint8_t END_OF_LINE_OFFSET_R[NUM_MACHINES] = {12, 12, 6};

constexpr uint8_t END_LEFT[NUM_MACHINES]      = {   0U,   0U,   0U};
constexpr uint8_t END_RIGHT[NUM_MACHINES]     = { 255U, 255U, 140U};
constexpr uint8_t END_OFFSET[NUM_MACHINES]    = {  28U,  28U,  14U};

constexpr uint8_t START_OFFSET[NUM_MACHINES][NUM_DIRECTIONS][NUM_CARRIAGES] = {
// KH910
    {// NC,  K,  L,  G
        {0,  0,  0,  0}, // NoDirection
        {0, 40, 40,  8}, // Left
        {0, 16, 16, 32}  // Right
                          },
// KH930
    {// NC,  K,  L,  G
        {0,  0,  0,  0}, // NoDirection
        {0, 40, 40,  8}, // Left
        {0, 16, 16, 32}  // Right
                          },
// KH270
    {// NC,  K
        {0,  0,  0,  0}, // NoDirection
        {0, 14,  0,  0}, // Left
        {0,  2,  0,  0}  // Right
                          }
};

// Should be calibrated to each device
// These values are for the K carriage
//                                               KH910 KH930 KH270
constexpr uint16_t FILTER_L_MIN[NUM_MACHINES] = { 200U, 200U, 200U}; // below: L Carriage
constexpr uint16_t FILTER_L_MAX[NUM_MACHINES] = { 600U, 600U, 600U}; // above: K Carriage
constexpr uint16_t FILTER_R_MIN[NUM_MACHINES] = { 200U,   0U,   0U};
constexpr uint16_t FILTER_R_MAX[NUM_MACHINES] = {1023U, 600U, 600U};
constexpr uint16_t SOLENOIDS_BITMASK[NUM_MACHINES] = {0xFFFFU, 0xFFFFU, 0x7FF8};

/*!
 * \brief Untemplated interface for Encoders.
 *
 * Encoders for Beltshift, Direction, Active Hall sensor and Carriage Type.
 * Most methods are untemplated and can be inherited unchanged by the
 * templated derived classes.
 */
class EncodersBase {
public:
  //EncodersBase();
  virtual ~EncodersBase() {}

  virtual void encA_interrupt() {}
  static uint16_t getHallValue(Direction_t pSensor);

  uint8_t getPosition() const {return m_encoderPos;}
  Beltshift_t getBeltshift() const {return m_beltShift;}
  Direction_t getDirection() const {return m_direction;}
  Direction_t getHallActive() const {return m_hallActive;}
  Carriage_t getCarriage() const {return m_carriage;}
  Machine_t getMachineType() const {return m_machineType;}

protected:
  Direction_t m_direction;
  Direction_t m_hallActive;
  Beltshift_t m_beltShift;
  Carriage_t m_carriage;
  Machine_t m_machineType;

  uint8_t m_encoderPos = 0x00;
  bool m_oldState = false;

  virtual void encA_rising() {}
  void encA_falling();
};

/*!
 * \brief Template for derived classes for Encoders.
 */
template <Machine_t M> class Encoders : public EncodersBase {
public:
  Encoders<M>() : m_machineType(M) {}

  void encA_interrupt() override;

private:
  Machine_t m_machineType;

  void encA_rising() override;
};

#endif // ENCODERS_H_
