#include "TSubBoard.h"

void TSubBoard::Init(HardwareSerial *_port) {
  port = _port;
  port->begin(250000);
  port->setTimeout(50);
}

void TSubBoard::UpdatePositionAndVcc() {
  port->write('r');
  port->readBytes((byte *)&position, sizeof(struct Angles));
  port->readBytes((byte *)&Vcc, sizeof(Vcc));
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
