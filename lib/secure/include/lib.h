#ifndef s_lib_h
#define s_lib_h

//#include <lpuni.h>
#include <dirs.h>

#include <comp.h>
#include <events.h>
#include <lvs.h>
#include <props.h>
#include <std.h>
#include <user.h>


#define LIB_AUTH           DIR_SECURE_LIB "/auth"
#define LIB_BATTERY        DIR_LIB        "/battery"
#define LIB_BLANK          DIR_LIB        "/blank"
#define LIB_BLANK_PILE     DIR_LIB        "/blank_pile"
#define LIB_BOARD          DIR_LIB        "/bboard"
#define LIB_BODY           DIR_LIB        "/body"
#define LIB_BONUS          DIR_LIB        "/bonus"
#define LIB_BOT            DIR_LIB        "/bot"
#define LIB_BURN           DIR_LIB        "/burn"
#define LIB_CAPTURE        DIR_LIB        "/capture"
#define LIB_CHAPEL         DIR_LIB        "/chapel"
#define LIB_CHAT           DIR_LIB        "/chat"
#define LIB_CLASSES        DIR_LIB        "/classes"
#define LIB_CLAY           DIR_LIB        "/clay"
#define LIB_CLERK          DIR_LIB        "/clerk"
#define LIB_CLIENT         DIR_SECURE_LIB "/net/client"
#define LIB_CLIP           DIR_LIB        "/clip"
#define LIB_COMBAT         DIR_LIB        "/combat"
#define LIB_COMBATMSG      DIR_LIB        "/combatmsg"
#define LIB_COMMAND        DIR_LIB        "/command"
#define LIB_CONNECT        DIR_SECURE_LIB "/connect"
#define LIB_CREATOR        DIR_LIB        "/creator"
#define LIB_CURRENCY       DIR_LIB        "/currency"
#define LIB_DETECT         DIR_LIB        "/detect"
#define LIB_DIGGING        DIR_LIB        "/digging"
#define LIB_DISASTER       DIR_LIB        "/disaster"
#define LIB_DOMESTICATE	   DIR_LIB        "/domesticate"
#define LIB_DONATE	   DIR_LIB        "/donate"
#define LIB_DOOR           DIR_LIB        "/door"
#define LIB_EDITOR         DIR_LIB        "/editor"
#define LIB_ENTER          DIR_LIB        "/enter"
#define LIB_EXITS          DIR_LIB        "/exits"
#define LIB_FILE           DIR_SECURE_LIB "/file"
#define LIB_FILES          DIR_LIB        "/files"
#define LIB_FISH           DIR_LIB        "/fish"
#define LIB_FISHING        DIR_LIB        "/fishing"
#define LIB_FLASHLIGHT     DIR_LIB        "/flashlight"
#define LIB_FOLLOW         DIR_LIB        "/follow"
#define LIB_FTP_DATA_CONN  DIR_SECURE_LIB "/net/ftp_data_connection"
#define LIB_FUEL           DIR_LIB        "/fuel"
#define LIB_GENETICS       DIR_LIB        "/genetics"
#define LIB_GUN            DIR_LIB        "/gun"
#define LIB_CLAN           DIR_LIB        "/clan"
#define LIB_HELP           DIR_LIB        "/help"
#define LIB_HISTORY        DIR_LIB        "/history"
#define LIB_INTERACTIVE    DIR_LIB        "/interactive"
#define LIB_INTERFACE      DIR_LIB        "/interface"
#define LIB_LAMP           DIR_LIB        "/lamp"
#define LIB_LANGUAGE       DIR_LIB        "/language"
#define LIB_LEAD           DIR_LIB        "/lead"
#define LIB_LEADER         DIR_LIB        "/leader"
#define LIB_LIGHT          DIR_LIB        "/light"
#define LIB_LIVING         DIR_LIB        "/living"
#define LIB_LOGIN          DIR_SECURE_LIB "/login"
#define LIB_MAGIC          DIR_LIB        "/magic"
#define LIB_MANYCOINS      DIR_LIB        "/manycoins"
#define LIB_MATCH          DIR_LIB        "/match"
#define LIB_MAYOR          DIR_LIB        "/mayor"
#define LIB_MEAL           DIR_LIB        "/meal"
#define LIB_MEAL_POISON    DIR_LIB        "/poison"
#define LIB_MESSAGES       DIR_LIB        "/messages"
#define LIB_MONEY          DIR_LIB        "/money"
#define LIB_MOUNT          DIR_LIB        "/mount"
#define LIB_NMSH           DIR_LIB        "/nmsh"
#define LIB_NPC            DIR_LIB        "/npc"
#define LIB_PAGER          DIR_LIB        "/pager"
#define LIB_PERSIST        DIR_LIB        "/persist"
#define LIB_PILE           DIR_LIB        "/pile"
#define LIB_PISTOL         DIR_LIB        "/pistol"
#define LIB_PLAYER         DIR_LIB        "/player"
#define LIB_PLAYERS        DIR_LIB        "/player"
#define LIB_POLE           DIR_LIB        "/pole"
#define LIB_POST_OFFICE    DIR_LIB        "/post_office"
#define LIB_POTION         DIR_LIB        "/potion"
#define LIB_QUEST          DIR_LIB        "/quest"
#define LIB_RACE           DIR_LIB        "/race"
#define LIB_REMOTE         DIR_LIB        "/remote"
#define LIB_RIFLE          DIR_LIB        "/rifle"
#define LIB_ROUND          DIR_LIB        "/round"
#define LIB_SCROLL         DIR_LIB        "/scroll"
#define LIB_SECURE_BOARD   DIR_SECURE_LIB "/bboard"
#define LIB_SENTIENT       DIR_LIB        "/sentient"
#define LIB_SERVER         DIR_SECURE_LIB "/net/server"
#define LIB_SHADOW         DIR_LIB        "/shadow"
#define LIB_SHELL          DIR_LIB        "/shell"
#define LIB_SHOP           DIR_LIB        "/shop"
#define LIB_SOCKET         DIR_SECURE_LIB "/net/socket"
#define LIB_SPELL          DIR_LIB        "/spell"
#define LIB_STARGATE       DIR_LIB        "/stargate"
#define LIB_STEAL          DIR_LIB        "/steal"
#define LIB_TALK           DIR_LIB        "/talk"
#define LIB_TEACHER        DIR_LIB        "/teacher"
#define LIB_TELLER         DIR_LIB        "/teller"
#define LIB_TORCH          DIR_LIB        "/torch"
#define LIB_TRAINER        DIR_LIB        "/trainer"
#define LIB_UNDEAD         DIR_LIB        "/undead"
#define LIB_USED_MEAL      DIR_LIB        "/used_meal"
#define LIB_VEHICLE        DIR_LIB        "/vehicle"
#define LIB_VERB           DIR_LIB        "/verb"
#define LIB_VIRTUAL        DIR_LIB        "/virtual"

#endif /* s_lib_h */
