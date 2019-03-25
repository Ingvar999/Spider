#pragma once

#include "Request.h"

struct TTask {
  public:
    TTask() {}

    TTask(const Request &req):
      command(req.command_property), argc(req.argc), sender(req.id) {
      args[0] = req.args[0];
      args[1] = req.args[1];
    }

    char command;
    int argc;
    int args[2];
    byte sender;
};
