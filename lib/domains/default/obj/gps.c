#include <lib.h>
#include <daemons.h>
#include <vendor_types.h>
inherit LIB_ITEM;
mapping Tracked = ([]);
mixed gtmp;

string LongDesc(){
    string ret;
    Tracked = ([]);
    if(creatorp(this_player())){
        ret = "Это маленькое устройство создано чтобы "+
            "обеспечивать вас информацией о вашем местоположении в мире, "+
            "если это возможно. Команды: coord, prox";
    }
    else {
        ret = "Это устройство билдеров для определения "+
            "их местоположения в мире.";
    }
    return ret;
}

void create(){
    ::create();
    SetKeyName("GPS навигатор");
    SetAdjectives( ({"gps", "global positioning", "system", "маленькое"}) );
    SetId( ({"gps", "устройство", "навигатор"}) );
    SetShort("GPS навигатор");
    SetLong( (: LongDesc :) );
    SetNoCondition(1);
    SetMass(20);
    SetVendorType(VT_TREASURE);
}

void init(){
    ::init();
    add_action("GetCoord","coord");
    add_action("GetProx","prox");
    add_action("GetShell","shell");
    add_action("StartTrack","track");
    add_action("StopTrack","untrack");
}

int GetCoord(){
    string ret;
    if(!creatorp(this_player())) return 0;
    ret = ROOMS_D->GetCoordinates(environment(this_player()));
    if(!sizeof(ret) || ROOMS_D->GetGrid(ret)["room"] != 
            base_name(environment(this_player()))){
        ret = "Кажется сейчас вы не можете получить "+
            "информацию о вашем местоположении.";
    }
    else{
        mixed foo = ROOMS_D->GetGrid(ret);
        if(foo && creatorp(this_player())){
            ret += " "+identify((foo["room"] || ""));
        }
    }
    write(ret);
    return 1;
}

int GetProx(int i){
    string ret = "";
    int x, y, z;
    string coords;
    if(!creatorp(this_player())) return 0;
    coords = ROOMS_D->GetCoordinates(environment(this_player()));
    if(!sizeof(coords) || ROOMS_D->GetGrid(coords)["room"] !=
            base_name(environment(this_player())) || 
            sscanf(coords,"%d,%d,%d",x,y,z) != 3){
        ret = "Кажется сейчас вы не можете получить "+
            "полную информацию о вашем местоположении.";
    }
    else{
        if(ROOMS_D->GetGrid(x+","+(y+1)+","+z)["room"])
            ret += "North: "+ROOMS_D->GetGrid(x+","+(y+1)+","+z)["room"];
        if(ROOMS_D->GetGrid(x+","+(y-1)+","+z)["room"])
            ret += "\nSouth: "+ROOMS_D->GetGrid(x+","+(y-1)+","+z)["room"];
        if(ROOMS_D->GetGrid((x+1)+","+y+","+z)["room"])
            ret += "\nEast: "+ROOMS_D->GetGrid((x+1)+","+y+","+z)["room"];
        if(ROOMS_D->GetGrid((x-1)+","+y+","+z)["room"])
            ret += "\nWest: "+ROOMS_D->GetGrid((x-1)+","+y+","+z)["room"];
        if(ROOMS_D->GetGrid((x+1)+","+(y+1)+","+z)["room"])
            ret += "\nNortheast: "+ROOMS_D->GetGrid((x+1)+","+(y+1)+","+z)["room"];
        if(ROOMS_D->GetGrid((x-1)+","+(y+1)+","+z)["room"])
            ret += "\nNorthwest: "+ROOMS_D->GetGrid((x-1)+","+(y+1)+","+z)["room"];
        if(ROOMS_D->GetGrid((x+1)+","+(y-1)+","+z)["room"])
            ret += "\nSoutheast: "+ROOMS_D->GetGrid((x+1)+","+(y-1)+","+z)["room"];
        if(ROOMS_D->GetGrid((x-1)+","+(y-1)+","+z)["room"])
            ret += "\nSouthwest: "+ROOMS_D->GetGrid((x-1)+","+(y-1)+","+z)["room"];
        if(ROOMS_D->GetGrid(x+","+y+","+(z+1))["room"])
            ret += "\nUp: "+ROOMS_D->GetGrid(x+","+y+","+(z+1))["room"];
        if(ROOMS_D->GetGrid(x+","+y+","+(z-1))["room"])
            ret += "\nDown: "+ROOMS_D->GetGrid(x+","+y+","+(z-1))["room"];
    }
    write(ret);
    return 1;
}

int StartTrack(string str){
    object mark = to_object(str);
    if(!mark || !living(mark)){
        mark = find_player(str);
    }
    if(!mark){
        write("Это существо не может быть найдено.");
        return 1;
    }
    write("Начато выслеживание "+capitalize(mark->GetKeyName())+".");
    TRACKER_D->AddTrack(mark, this_object());
    Tracked[str] = mark;
    return 1;
}

int StopTrack(string str){
    object mark;
    if(!Tracked[str]){         
        write("Это устройство не отслеживало это живое существо.");
        return 1;     
    }
    write("Закончено выслеживание "+
            capitalize(Tracked[str]->GetKeyName())+".");     
    TRACKER_D->RemoveTrack(mark, this_object());
    Tracked[str] = 0;
    return 1;
}

int ReceiveTrackingData(mapping data){
    object env = environment();
    if(!env) return 0;
    tell_object(env, "GPS навигатор сообщил: Получены следующие данные слежения "+
            data["object"]->GetShort()+": "+identify(data));
    return 1;
}

int GetShell(string str){
    mapping Shell = ([]);
    mixed all = ({});
    int shell = atoi(str);
    int i, max = shell+1;
    Shell[0] = room_environment(this_player())->GetNeighbors();
    if(!shell){
        return 1;
    }
    for(i = 1; i < max; i++){ 
        Shell[i] = ({});
        foreach(mixed foo in Shell[i-1]){
            object bar;
            mixed tmparr = ({});
            catch( bar = load_object(foo) );
            if(bar){
                tmparr = (bar->GetNeighbors() - ({ foo }) );
                if(!sizeof(tmparr)){
                    bar->CompileNeighbors();
                    tmparr = (bar->GetNeighbors() - ({ foo }) );
                }
            }
            gtmp = Shell[i-1];
            tmparr = filter(tmparr, (: member_array($1, gtmp) == -1 :) );
            gtmp = all;
            Shell[i] += filter(tmparr, (: member_array($1, gtmp) == -1 :) );
            all += Shell[i];
        }
        Shell[i] = distinct_array(Shell[i]);
    }
    write("Комнаты на расстоянии "+shell+": "+identify(Shell[shell]));
    return 1;
}
