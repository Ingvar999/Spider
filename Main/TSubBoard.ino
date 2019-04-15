#include "TSubBoard.h"

void TSubBoard::Init(HardwareSerial *_port) {
  port = _port;
  port->begin(250000);
  port->setTimeout(30);
}

int TSubBoard::UpdatePositionAndVcc() {
  port->write('r');
  port->readBytes((byte *)&position, sizeof(struct Angles));
  port->readBytes((byte *)&Vcc, sizeof(Vcc));
  if (position.horizontal >= -30){
    position.horizontal -= 150;
  }
  else{
    position.horizontal += 210;
  }
  if (position.horizontal > 180 || position.horizontal < -180 || position.vertical > 90 || position.vertical < 0 || Vcc > 8000){
    return 1;
  }
  return 0;
}

void TSubBoard::TurnLegs(byte values[7]) {
  port->write('w');
  port->write(values, 7);
}
