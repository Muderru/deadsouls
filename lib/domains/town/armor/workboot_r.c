#include <lib.h>
#include <armor_types.h>
#include <damage_types.h>
inherit LIB_ARMOR;

static void create(){
    armor::create();
    SetKeyName("рабочий ботинок");
    SetAdjectives( ({"strong", "brown", "кожаный", "правый", "рабочий", "тяжелый"}) );
    SetId( ({"ботинок"}) );
    SetShort("рабочий ботинок");
    SetLong("Вы видите тяжелый кожаный ботинок с армированным носком. Его можно одеть на правую ногу.");
    SetMass(40);
    SetBaseCost("silver",25);
    SetProtection(BLUNT,5);
    SetProtection(BLADE,5);
    SetProtection(KNIFE,5);
    SetArmorType(A_BOOT);
    SetRestrictLimbs(({"right foot"}));
}

void init(){
    ::init();
}
