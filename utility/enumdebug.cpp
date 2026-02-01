#include "../utility/enumdebug.h"

const char* ToString(player::PlayerActState s)
{
    switch (s)
    {
    case player::PlayerActState::Idle:   return "Idle";
    case player::PlayerActState::Walk:   return "Walk";
    case player::PlayerActState::Jump:   return "Jump";
    case player::PlayerActState::Attack: return "Attack";
    }
    return "Unknown";
}

const char* ToString(player::PlayerState s)
{
	switch (s)
	{
	case player::PlayerState::Alive:   return "Alive";
	case player::PlayerState::Dead:   return "Dead";
	}
	return "Unknown";
}

const char* ToString(player::PlayerReact s)
{
	switch (s)
	{
	case player::PlayerReact::None:   return "None";
	case player::PlayerReact::Damage:   return "Damage";
	}
	return "Unknown";
}

std::ostream& operator<<(std::ostream& os, player::PlayerActState s)
{
    return os << ToString(s);
}
