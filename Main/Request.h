#pragma once

struct Request{
  byte id;
  char requestType;
  String requiredValues;
  char command_property;
  int argc;
  int args[2];
};
