#include "TSubBoard.h"

void TSubBoard::Init(HardwareSerial *_port) {
  port = _port;
  port->begin(250000);
  port->setTimeout(30);
}

uint32_t TSubBoard::GetVcc() {
  port->write('v');
  uint32_t currentVcc = 0;
  port->readBytes((byte *)&currentVcc, sizeof(currentVcc));
  return currentVcc;
}

void TSubBoard::UpdatePosition() {
  port->write('r');
  port->readBytes((byte *)&position, sizeof(struct Angles));
  if (position.horizontal >= -30){
    position.horizontal -= 150;
  }
  else{
    position.horizontal += 210;
  }
}

void TSubBoard::TurnLegs(byte values[7]) {
  port->write('w');
  port->write(values, 7);
}
