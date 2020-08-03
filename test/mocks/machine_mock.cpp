/*!`
 * \file machine_mock.cpp
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

#include <machine_mock.h>
#include <machine.h>

static MachineMock *gMachineMock = NULL;
MachineMock *machineMockInstance() {
  if (!gMachineMock) {
    gMachineMock = new MachineMock();
  }
  return gMachineMock;
}

void releaseMachineMock() {
  if (gMachineMock) {
    delete gMachineMock;
    gMachineMock = NULL;
  }
}

Machine::Machine() {
  m_machineType = NO_MACHINE;
}

void Machine::setMachineType(Machine_t machineType) {
  assert(gMachineMock != NULL);
  return gMachineMock->setMachineType(machineType);
}

uint8_t Machine::numNeedles() {
  assert(gMachineMock != NULL);
  return gMachineMock->numNeedles();
}

uint8_t Machine::lenLineBuffer() {
  assert(gMachineMock != NULL);
  return gMachineMock->lenLineBuffer();
}

uint8_t Machine::endOfLineOffsetL() {
  assert(gMachineMock != NULL);
  return gMachineMock->endOfLineOffsetL();
}

uint8_t Machine::endOfLineOffsetR() {
  assert(gMachineMock != NULL);
  return gMachineMock->endOfLineOffsetR();
}

uint8_t Machine::startOffsetLUT(Direction_t direction, Carriage_t carriage) {
  assert(gMachineMock != NULL);
  return gMachineMock->startOffsetLUT(Direction_t direction, Carriage_t carriage);
}
