#include "weapons.h"

namespace Game {

const WeaponDef WEAPONS[WEAPON_COUNT] = {
    // name,       clip, dmg, pellets, spread,  cooldown, reload, auto,  haptic_ms
    {  "Pistol",    12,  12,   1,      0.0f,    0.20f,    1.0f,   false,  50  },
    {  "SMG",       30,   6,   1,      0.0f,    0.07f,    1.8f,   true,   20  },
    {  "Shotgun",    6,   8,   5,      0.30f,   0.50f,    1.5f,   false, 120  },
};

} // namespace Game
