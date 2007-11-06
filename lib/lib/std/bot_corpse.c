#include <lib.h>
#include <message_class.h>

inherit LIB_CORPSE;

void create() {
    corpse::create();
    SetId(({"chassis","body","synthetics"}));
    SetAdjectives(({"melting","corroding"}));
}

int eventDecay() {
    if( !environment() ) {
        Destruct();
        return 0;
    }
    Fresh = 0;
    switch(Count) {
    case 10:
        environment()->eventPrint(possessive_noun(Owner) + " chassis " +
          "begins to corrode.", MSG_ROOMDESC);
        SetId(GetId()..., "chassis", "body");
        SetAdjectives(GetAdjectives()..., "melting", "corroding");
        SetShort("the corroding chassis of an artificial body");
        SetSmell("The chassis emits an acrid chemical odor.");
        break;
    case 20:
        environment()->eventPrint("An acrid chemical odor fills the entire "
          "area.", MSG_ROOMDESC);
        SetId(GetId()..., "synthetics", "pile", "pile of synthetics");
        SetShort("a pile of corroding synthetics");
        SetSmell("Its smell is nearly unbearable.");
        break;
    case 30:
        Destruct();
        return 0;
    }
    Count++;
    //CallOut = call_out((: eventDecay :), DecayLife/3);
    return Count;
}

void SetCorpse(object who) {
    string tmpshort = (string)who->GetShort();
    if(sizeof(who->GetRiders())) tmpshort = (string)who->GetPlainShort();
    if( DecayLife < 100 ) {
        DecayLife = 500;
    }
    SetKeyName(who->GetKeyName());
    //SetId(who->GetId()...);
    SetId(({ "remains","wreckage",who->GetId()... }) );
    Owner = who->GetCapName();
    Race = who->GetRace();
    Count = 1;
    Fresh = 1;
    SetShort("the wreckage of "+tmpshort);
    SetLong("As you look closely at " + who->GetCapName() +
      ", you notice that " +  nominative(who) +
      " does not appear to be moving.");
    //CallOut = call_out((: eventDecay :), DecayLife/3);
}
