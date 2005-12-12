#include <lib.h>
#include <meal_types.h>

inherit LIB_MEAL;

static void create() {
    meal::create();
    SetKeyName("ale");
    SetId( ({ "bottle", "bourbon" ,"fire","firebreather"}) );
    SetShort("a bottle of bourbon");
    SetLong("A dark green bottle of \"Firebreather\" brand bourbon.");
    SetMass(100);
    SetBaseCost("silver",50);
    SetMealType(MEAL_DRINK | MEAL_ALCOHOL);
    SetStrength(100);
    SetMealMessages("You finish off the bourbon and gasp! WHOOT!! Mean stuff!",
      "$N finishes off a bottle of Firebreather bourbon and lets loose a rebel yell!");
    SetEmptyName("bottle");
    SetEmptyShort("an empty bottle of bourbon");
    SetEmptyLong("This is an empty green bottle that once held bourbon.");
}
