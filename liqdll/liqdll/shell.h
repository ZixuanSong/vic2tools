#pragma once
#include <string>
#include "game.h"

void ShellInit();
int ParseInput(std::string&, Game&);
void RunShell(Game*);