#include <lib.h>
#include <armor_types.h>
#include <damage_types.h>
inherit LIB_ARMOR;
static void create(){
    armor::create();
    SetKeyName("motorcycle helmet");
    SetId(({"helmet","cover","motorcycle"}));
    SetAdjectives(({"motorcycle","battle","hard"}));
    SetShort("a black motorcycle helmet");
    SetLong("This is a helmet made of tough composite materials.");
    SetMass(120);
    SetBaseCost("silver",200);
    SetDamagePoints(100);
    SetArmorType(A_HELMET);
    SetProtection(BLUNT, 20);
    SetProtection(BLADE, 20);
    SetProtection(KNIFE, 20);
}
