#include <lib.h>
#include <damage_types.h>

inherit LIB_SURFACE;
object to;

void create() {
    ::create();
    SetKeyName("rack");
    SetId(({"rack"}));
    SetAdjectives(({"knife","metal","fancy","steel","stainless","stainless-steel"}));
    SetShort("a weapons rack");
    SetLong("This is a bladed weapons rack here for your combat "+
      "testing convenience.");
    SetNoCondition(1);
    SetMass(274);
    SetBaseCost("silver",50);
    SetMaxCarry(5000);
    SetInventory(([
        "/domains/default/weap/sharpsword" : ({ 10, 5 }),
      ]));
    SetCanClose(0);
}

mixed CanGet(object ob) { return "The rack does not budge.";}
int CanReceive(object ob) {
    if(ob->GetDamageType() != BLADE) {
        write("This is a rack for sword-like weapons only.");
        return 0;
    }
    else return 1;
}

void init(){
    ::init();
}
