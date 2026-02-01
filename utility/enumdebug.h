#pragma once
#include <iostream>
#include "../gameobject/player.h"

const char* ToString(player::PlayerActState s);
std::ostream& operator<<(std::ostream& os, player::PlayerActState s);

const char* ToString(player::PlayerState s);
std::ostream& operator<<(std::ostream& os, player::PlayerState s);

const char* ToString(player::PlayerReact s);
std::ostream& operator<<(std::ostream& os, player::PlayerReact s);
