#include <network.h>
#include <save.h>
#include <daemons.h>

string my_name = "*yatmim";
string my_password = "CROATOAN";
string *ok_ips = ({});
int irn_enabled = 1;
int irn_maxtry = 32;
int convert_channel = 0;

mapping routers = ([
  "*i4" : ([ "ip" : "204.209.44.3", "port" : 8080, "password" : "STENDEC" ]),
]);

static mapping chan_conv = ([
  //"*i4" : ([ "dead_test" : "test" ])
]);

mapping irn_connections = ([]);
mapping irn_sockets = ([]);

void eventSetup();
static void write_data(int fd, mixed data);
static varargs void SendList(mixed data, int fd, string type);
varargs void SendWholeList(int fd,string type);
static void begin_socket_handoff(int i);

static int GoodPeer(int fd, mixed data){
    string ip = explode(socket_address(fd)," ")[0];
    trr("IRN: hit GoodPeer","yellow");
    if(member_array(ip,ok_ips) == -1){
	//trr("irn: bad ip");
	return 0;
    }

    if(member_array(data[2], keys(routers)) == -1){
	//trr("IRN: unknown peer");
	return 0;
    }
    if(data[4] != my_name){
	//trr("IRN: they don't know my name. I am "+my_name+". They think I am: "+identify(data[4]));
	return 0;
    }

    if(data[6]["client_password"] != routers[data[2]]["password"]){
	//trr("IRN: wrong client password");
	return 0;
    }
    if(data[6]["server_password"] != my_password){
	//trr("IRN: wrong server password");
	return 0;
    }
    irn_connections[data[2]]["fd"] = fd;
    trr("IRN: returning 1!","yellow");
    return 1;
}

static int ValidatePeer(int fd, mixed data){
    string ip = explode(socket_address(fd)," ")[0];
    //trr("IRN: hit ValidatePeer","green");
    if(member_array(ip,ok_ips) == -1){
	//trr("IRN: bad ip");
	return 0;
    }
    if(!irn_connections[data[2]]){
	//trr("IRN: no such connection.");
	return 0;
    }
    if(member_array(data[2], keys(routers)) == -1){
	//trr("IRN: unknown peer");
	return 0;
    }

    foreach(mixed key, mixed val in routers){
	if(routers[key]["ip"] == ip && irn_connections[key]["fd"] != -1 &&
	  irn_connections[key]["fd"] != fd ){
	    //trr("IRN: wrong fd I got "+fd+" but expected "+ irn_connections[key]["fd"]);
	    return 0;
	}
    }
    //trr("returning 1!","green");
    return 1;
}

static string id_mud(int fd){
    string *ret = ({});
    foreach(mixed element in keys(irn_connections)){
	if(irn_connections[element]["fd"] == fd) ret += ({ element });
    }
    return implode(ret,", ");
}

void irn_clear(){
    foreach(mixed key, mixed val in irn_connections){
	this_object()->close_connection(irn_connections[key]["fd"]);
    }
    foreach(mixed key, mixed val in irn_sockets){
	this_object()->close_connection(key);
    }

    irn_connections = ([]);
    irn_sockets = ([]);

    foreach(string key, mixed val in routers){
	if(key == my_name){
	    map_delete(routers, key);
	    continue;         }
	ok_ips += ({ routers[key]["ip"] });
	if(!irn_connections[key]){
	    irn_connections[key] = (["blocking" : 1, "type": MUD, "fd": -1, "data" : 0 ]);
	}
    }
    save_object(SAVE_ROUTER);
}

varargs void irn_setup(int clear){
    if(!irn_enabled) return;
    if(clear){
	foreach(mixed key, mixed val in irn_connections){
	    this_object()->close_connection(irn_connections[key]["fd"]);
	}
	foreach(mixed key, mixed val in irn_sockets){
	    this_object()->close_connection(key);
	}
	irn_connections = ([]);
	irn_sockets = ([]);
    }
    foreach(string key, mixed val in routers){
	if(key == my_name){
	    map_delete(routers, key);
	    continue;
	}
	ok_ips += ({ routers[key]["ip"] });
	if(!irn_connections[key]){
	    irn_connections[key] = (["blocking" : 1, "type": MUD, "fd": -1, "data" :
	      0 ]);
	}
	else {
	    trr("sockstat on "+key+": "+identify(socket_status(irn_connections[key]["fd"])),"white");
	    if(socket_status(irn_connections[key]["fd"]) &&
	      socket_status(irn_connections[key]["fd"])[1] == "CLOSED")
		irn_connections[key]["fd"] = -1;
	    trr("sock staus on "+key+": "+irn_connections[key]["fd"],"white");
	}
    }

    foreach(string name in keys(routers)){
	int tmp_fd, sockstat;

	if(irn_connections[name]["fd"] != -1) continue;
	tmp_fd = socket_create(MUD, "irn_read_callback","irn_close_callback"); 
	if(tmp_fd < 0){
	    trr("irn: Couldn't create socket. errorcode: "+socket_error(tmp_fd));
	    return;
	}

	sockstat = socket_bind(tmp_fd, 0);
	//trr("socket_bind: "+sockstat);

	if(sockstat < 0){
	    trr("irn: Couldn't bind socket. errorcode: "+socket_error(sockstat));
	    return;
	}

	irn_connections[name]["fd"] = tmp_fd;
	irn_sockets[tmp_fd] = (["name" : name]);

	sockstat = socket_connect(irn_connections[name]["fd"], routers[name]["ip"]+" "+
	  routers[name]["port"], "irn_read_callback", "irn_write_callback");
	//trr("socket_connect: "+sockstat);

	if(sockstat < 0){
	    trr("irn: Couldn't connect to "+name+", errorcode: "+socket_error(sockstat));
	    this_object()->close_connection(irn_connections[name]["fd"]);
	    irn_connections[name]["fd"] = -1;
	}
	this_object()->Report();
	begin_socket_handoff(tmp_fd);
	call_out( "eventSendStartup", 5, tmp_fd);
    }
}

varargs void eventSendStartup(int fd){
    mixed *targets = ({});
    //trr("irn: hit eventSendStartup","yellow");
    if(!irn_enabled) return;
    //trr("irn: hit eventSendStartup","green");
    if(!fd) targets = keys(irn_connections);
    else if(irn_sockets[fd]["name"]) targets = ({ irn_sockets[fd]["name"] });
    else targets = keys(irn_connections);
    if(!sizeof(targets)){
	call_out( "irn_setup", 5, 1 );
	return;
    }
    this_object()->Report();
    foreach(mixed element in targets){
	trr("irn: sending statup to "+element+" on fd "+irn_connections[element]["fd"]);
	write_data(irn_connections[element]["fd"],
	  ({
	    "irn-startup-req",
	    5,
	    my_name,
	    "foo",
	    element,
	    "bar",
	    ([ "client_password" : my_password, "server_password" : routers[element]["password"] ])
	})
      );
}
}

static void irn_write_callback(int fd){
    //trr("hit irn_write_callback for fd"+fd);
}

static void irn_close_callback(int fd){
    if(!irn_enabled) return;
    irn_connections[id_mud(fd)]["fd"] = -1;
    map_delete(irn_sockets, fd);
    trr("I'm wanting to close "+id_mud(fd)+" on fd"+fd+" now.");
    call_out( "eventSendStartup", 5);
}

static void irn_read_callback(int fd, mixed data){
    int i;
    string tmp="";
    mapping MudList = ([]);
    if(!irn_enabled) return;
    if(this_object()->query_mudinfo())
	MudList = this_object()->query_mudinfo();
    else MudList = ([ "wt" : "f" ]);
    //trr("irn: hit irn_read_callback for fd"+fd+", "+id_mud(fd));
    if(!data || !sizeof(data)){
	//trr("irn: no data");
    }
    else {
	if(bufferp(data)){
	    //trr("data buffer: "+identify(read_buffer(data)));
	    for(i=0;i<sizeof(data);i++){
		tmp += sprintf("%c",data[i]);
	    }
	}
	else {
	    //trr("data: "+identify(data));
	}
    }
    //trr("tmp: "+tmp);
    //trr("mudinfo keys: "+identify(keys(MudList)));
    switch(data[0]){
    case "irn-startup-req" :
	trr("IRN got a startup request on fd"+fd);
	if(!GoodPeer(fd, data)) return;
	trr("IRN startup from "+data[2]+" accepted.");
	irn_connections[data[2]]["fd"] = fd;
	//trr("irn_connections[\""+data[2]+"\"][\"fd\"]: "+irn_connections[data[2]]["fd"]);
	irn_sockets[fd] = ([ "name" : data[2] ]);
    //SendList(MudList, fd);
    SendWholeList(fd);
    trr(data[2]+" has joined IRN on socket "+fd);
    return;
case "irn-mudlist-altered" :
    trr("irn: received irn-mudlist-altered");
    if(!ValidatePeer(fd, data)) {
	trr("irn: peer failed validation.");
	//this_object()->close_connection(fd);
	//if(irn_sockets[fd]) map_delete(irn_sockets, fd);
	//if(irn_connections[data[2]]) map_delete(irn_connections, data[2]);
	return;
    }
    //trr("got new delta. sending to router.");
    this_object()->ReceiveList(data[7],"mudlist");
    break;
case "irn-mudlist-delta" :
    trr("irn: got mudlist delta");
    if(!ValidatePeer(fd, data)) {
	trr("irn: peer failed validation.");
	//this_object()->close_connection(fd);
	//if(irn_sockets[fd]) map_delete(irn_sockets, fd);
	//if(irn_connections[data[2]]) map_delete(irn_connections, data[2]);
	return;
    }
    //trr("irn: got new delta. sending to router.");
    this_object()->ReceiveList(data[7],"mudlist");
    break;
case "irn-chanlist-altered" :
    trr("irn: received irn-chanlist-altered");
    if(!ValidatePeer(fd, data)) {
	trr("irn: peer failed validation.");
	//this_object()->close_connection(fd);
	//if(irn_sockets[fd]) map_delete(irn_sockets, fd);
	//if(irn_connections[data[2]]) map_delete(irn_connections, data[2]);
	return;
    }
    //trr("got new delta. sending to router.");
    this_object()->ReceiveList(data[7],"chanlist");
    break;
case "irn-chanlist-delta" :
    trr("irn: got chanlist delta");
    if(!ValidatePeer(fd, data)) {
	trr("irn: peer failed validation.");
	//this_object()->close_connection(fd);
	//if(irn_sockets[fd]) map_delete(irn_sockets, fd);
	//if(irn_connections[data[2]]) map_delete(irn_connections, data[2]);
	return;
    }
    //trr("irn: got new delta. sending to router.");
    this_object()->ReceiveList(data[7],"chanlist");
    break;
case "irn-data" :
    if(!ValidatePeer(fd, data)) {
	trr("irn-data: peer on fd"+fd+" failed validation.");
	//this_object()->close_connection(fd);
	//if(irn_sockets[fd]) map_delete(irn_sockets, fd);
	//if(irn_connections[data[2]]) map_delete(irn_connections, data[2]);
	return;
    }
    //trr("irn: got new data, type: "+data[0]+". sending to router.");
    if(convert_channel && !strsrch(data[6][0],"chan") && strsrch(data[6][0],"chan-user")){
	if(chan_conv[irn_sockets[fd]["name"]]){
	    foreach(string key, string val in chan_conv[irn_sockets[fd]["name"]]){
		//trr("data[6][6]: "+data[6][6]);
		data[6][6] = replace_string(data[6][6],key,val);
	    }
	}
    } 
    this_object()->read_callback(data[6][2],data[6]);
    break;
default : 
    if(!ValidatePeer(fd, data)) {
	//this_object()->close_connection(fd);
	//if(irn_sockets[fd]) map_delete(irn_sockets, fd);
	//if(irn_connections[data[2]]) map_delete(irn_connections, data[2]);
	return;
    }
    //trr("irn: got i3 data, type: "+data[1][0]+". sending to router.");
    this_object()->read_callback(data[6][2],data[6]);
    return;
}
}

static varargs void SendList(mixed data, int fd, string type){
    int *targets = ({});
    mixed *outbound = ({});
    mapping tmp = ([]);
    if(!type || !sizeof(type)) type = "irn-mudlist-delta";
    if(type == "mudlist") type = "irn-mudlist-delta";
    if(type == "chanlist") type = "irn-chanlist-delta";
    //trr("list type: "+type);
    //trr("irn: trying to send list","white");
    if(!irn_enabled) return;
    if(!mapp(data)){
	//trr("irn: Tried to send a non-map. guilty stack: "+get_stack(),"cyan");
	return;
    }
    //trr("1");
    tmp = data;
    if(mapp(tmp)){
	foreach(mixed key, mixed val in tmp){
	    if(!mapp(val)){
		//trr("irn: Non mapping val. Key: "+key+". stack: "+get_stack(),"yellow");
		//trr("irn: guilty stack: "+get_stack(),"cyan");
	    }
	}
    }
    //trr("2");
    if(!fd) targets = keys(irn_sockets);
    else targets = ({ fd });
    foreach(int router in targets){
	//trr("3");
	if(type == "irn-chanlist-delta"){
	    //print_long_string(find_player("cratylus"),"I'm trying to send this: "+identify(data)+" to "+router);
	    //continue;
	}
	write_data(router, ({
	    type,
	    5,
	    my_name,
	    0,
	    irn_sockets[router]["name"],
	    0,
	    time(),
	    data
	  }) );
    }
}

varargs void SendWholeList(int fd, string type){
    mapping tmp = ([]);
    if(!type || !sizeof(type)) type = "mudlist";
    if(!irn_enabled) return;
    if(!fd) fd = 0;
    //trr("SendWholeList type: "+type);
    switch(type){
    case "mudlist" :
	tmp = this_object()->query_mudinfo();
	break;
    case "chanlist" :
	tmp = this_object()->query_chaninfo();
	break;
    default :
	trr("irn SendWholeList error: bad type "+type);
	return;
    }
    foreach(mixed key, mixed val in tmp){
	//trr("irn: Sending "+key,"white");
	if(type == "mudlist") if(mapp(val)) SendList( ([ key : val ]), fd, type );
	if(type == "chanlist"){
	    foreach(mixed key2, mixed val2 in val){
		SendList( ([ key : ([ key2 : val2 ]) ]), fd, type );
	    }
	}
    }
}

static void SendMessage(mixed data){
    //trr("irn: received SendMessage call","blue");
    if(!irn_enabled) return;
    foreach(int router in keys(irn_sockets)){
	if(convert_channel && !strsrch(data[0],"chan") && strsrch(data[0],"chan-user")){
	    if(member_array(irn_sockets[router]["name"], keys(chan_conv)) != -1){
		foreach(string key, string val in chan_conv[irn_sockets[router]["name"]]){
		    //trr("data[6]: "+data[6]);
		    data[6] = replace_string(data[6],val,key);
		    //trr("data: "+identify(data));
		}
	    }
	}
	write_data(router, ({
	    "irn-data",
	    5,
	    my_name,
	    0,
	    "*",
	    0,
	    data
	  }) );
    }
}

string Report(){
    string ret = "IRN: connections: ";
    foreach(mixed key, mixed val in irn_connections){
	if(!key) continue;
	ret += key+":"+irn_connections[key]["fd"]+" ";
    }
    ret += "\n";
    ret += "IRN: sockets: ";
    foreach(mixed key, mixed val in irn_sockets){
	int sstat = -1;
	string statmess = "DISCONNECTED";
	if(irn_connections[val["name"]]){
	    sstat = irn_connections[val["name"]]["fd"];
	    if(sstat > 0 && socket_status(key)) statmess = socket_status(key)[1];
	    ret += key +":"+val["name"]+":"+statmess+" ";
	}
	ret += "\n";
    }
    trr("irn: irn_connections: "+identify(irn_connections),"green");
    trr("irn: irn_sockets: "+identify(irn_sockets),"cyan");
    foreach(mixed element in keys(irn_sockets)){
	trr("socket_status("+element+"): "+identify(socket_status(element)),"green");
    }
    return ret;
}

static void begin_socket_handoff(int fd){
    object rsock = find_object(RSOCKET_D);
    if(!irn_enabled) return;
    if(!rsock) rsock = load_object(RSOCKET_D);
    if(!rsock){
	trr("IRN: Socket handoff failed. Couldn't load RSOCKET_D");
	return;
    }
    socket_release(fd, rsock, "complete_socket_handoff");
}