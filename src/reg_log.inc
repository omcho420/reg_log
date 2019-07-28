
#if defined _reg_log_included
	#endinput
#endif
#define _reg_log_included

#include <a_samp>

#if !defined mysql_included
	#include <a_mysql>

	#define MYSQL_HOST 		"localhost"
	#define MYSQL_USER 		"root"
	#define MYSQL_PASS 		""
	#define MYSQL_DATABASE 	"register_db"
#endif
new MySQL:dbHandle;

#if !defined USE_DIALOG_ENUM
	#define USE_DIALOG_ENUM			true
#endif

#if !defined USE_PLAYER_VARS
	#define USE_PLAYER_VARS			true
#endif

#if USE_PLAYER_VARS
	new
		p_ID 			[MAX_PLAYERS],
		p_Password		[MAX_PLAYERS][65],
		p_Salt			[MAX_PLAYERS][11],
		p_racecheck		[MAX_PLAYERS],
		bool: p_Logged 	[MAX_PLAYERS char]
		;
#endif

#if USE_DIALOG_ENUM
	enum { 
		D_REGISTER,
		D_LOGIN
	}
#endif

// -- // OnGameMode_*
#if defined _inc_y_hooks || defined _INC_y_hooks
	hook OnGameModeInit()
#else
	public OnGameModeInit()
#endif	
{
	if(mysql_errno((dbHandle = mysql_connect(MYSQL_HOST, MYSQL_USER, MYSQL_PASS, MYSQL_DATABASE)))) {
		print("MYSQL >> Couldn't connect to MySQL database.");
	} else {
		print("MYSQL >> Connection to database was successful.");
	}
	#if !defined _inc_y_hooks && !defined _INC_y_hooks\
		&& defined mysql_OnGameModeInit
		return mysql_OnGameModeInit();
	#else
		return true;
	#endif
}

#if !defined _inc_y_hooks && !defined _INC_y_hooks
	#if defined _ALS_OnGameModeInit
		#undef OnGameModeInit
	#else
		#define _ALS_OnGameModeInit
	#endif

	#define OnGameModeInit mysql_OnGameModeInit
	#if defined mysql_OnGameModeInit
		forward mysql_OnGameModeInit();
	#endif
#endif	

#if defined _inc_y_hooks || defined _INC_y_hooks
	hook OnGameModeExit()
#else
	public OnGameModeExit()
#endif
{
    mysql_close(dbHandle);
	#if !defined _inc_y_hooks && !defined _INC_y_hooks\
		&& defined mysql_OnGameModeExit
        return mysql_OnGameModeExit();
    #else
        return true;
    #endif
}

#if !defined _inc_y_hooks && !defined _INC_y_hooks
	#if defined _ALS_OnGameModeExit
	    #undef OnGameModeExit
	#else
	    #define _ALS_OnGameModeExit
	#endif

	#define OnGameModeExit mysql_OnGameModeExit
	#if defined mysql_OnGameModeExit
	    forward mysql_OnGameModeExit();
	#endif
#endif	    

// -- // OnPlayer_*

#if defined _inc_y_hooks || defined _INC_y_hooks
	hook OnPlayerConnect(playerid)
#else
	public OnPlayerConnect(playerid)
#endif	
{
	p_racecheck[playerid] ++;

	new 
		query[128],
		name[MAX_PLAYER_NAME + 1];
	GetPlayerName(playerid, name, sizeof name);

	mysql_format(dbHandle, query, sizeof query,
		"SELECT * FROM `players` WHERE `NAME` = '%s' LIMIT 1",
		name);
	mysql_pquery(dbHandle, query, "f_CheckPlayerData", "ii", playerid, p_racecheck[playerid]);

	#if !defined _inc_y_hooks && !defined _INC_y_hooks\
		&& defined mysql_OnPlayerConnect
		return mysql_OnPlayerConnect(playerid);
	#else
		return true;
	#endif
}

#if !defined _inc_y_hooks && !defined _INC_y_hooks
	#if defined _ALS_OnPlayerConnect
		#undef OnPlayerConnect
	#else
		#define _ALS_OnPlayerConnect
	#endif

	#define OnPlayerConnect mysql_OnPlayerConnect
	#if defined mysql_OnPlayerConnect
		forward mysql_OnPlayerConnect(playerid);
	#endif
#endif		


#if defined _inc_y_hooks || defined _INC_y_hooks
	hook OnPlayerDisconnect(playerid, reason)
#else
	public OnPlayerDisconnect(playerid, reason)
{
	p_Logged{playerid} = false;

	#if !defined _inc_y_hooks && !defined _INC_y_hooks\
		&& defined mysql_OnPlayerDisconnect
		return mysql_OnPlayerDisconnect(playerid, reason);
	#else
		return true;
	#endif
}

#if !defined _inc_y_hooks && !defined _INC_y_hooks
	#if defined _ALS_OnPlayerDisconnect
		#undef OnPlayerDisconnect
	#else
		#define _ALS_OnPlayerDisconnect
	#endif

	#define OnPlayerDisconnect mysql_OnPlayerDisconnect
	#if defined mysql_OnPlayerDisconnect
		forward mysql_OnPlayerDisconnect(playerid, reason);
	#endif
#endif		

// -- // OnDialogResponse

#if defined _inc_y_hooks || defined _INC_y_hooks
	hook OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
#else
	public OnDialogResponse(playerid, dialogid, response, listitem, inputtext[])
{
	switch(dialogid) {
		case D_REGISTER: {
			if(!response) return Kick(playerid);
			if(response) {
				if(p_Logged{playerid} == true) return Kick(playerid);
				new name[MAX_PLAYER_NAME + 1], str[34];
				GetPlayerName(playerid, name, sizeof name);
				if(strlen(inputtext) > 24 || strlen(inputtext) < 3) {

   					format(str, sizeof str, "Welcome %s", name);
   					#if USE_DIALOG_ENUM
    					ShowPlayerDialog(playerid, D_REGISTER, DIALOG_STYLE_PASSWORD, "Register", str, "Register", "Cancel");
    				#endif
				}
				else {
					new
						pIP[16]
					;

					for(new i = 0; i < 10; i++) {
					    p_Salt[playerid][i] = random(100) + 50;
					}

					GetPlayerName(playerid, name, sizeof name);

					p_Salt[playerid][10] = 0;
					SHA256_PassHash(inputtext, p_Salt[playerid], p_Password[playerid], 65);

					GetPlayerIp(playerid, pIP, sizeof pIP);

					new query[156];
					mysql_format(dbHandle, query, sizeof query,
						"INSERT INTO `players` (`NAME`,`PASSWORD`,`SALT`) \
							VALUES('%e','%e','%e')",
						name, p_Password[playerid], p_Salt[playerid]);
					mysql_tquery(dbHandle, query, "f_GiveAccountId", "i", playerid);
					SetSpawnInfo(playerid, 0, 1, 2495.0720, -1687.5278, 13.5150, 359.6696, 0, 0, 0, 0, 0, 0);
					SpawnPlayer(playerid);
				}
			}
		}
		case D_LOGIN: {
			if(!response) return Kick(playerid);
			if(response) {
		        new 
		        	buf[65];
		        SHA256_PassHash(inputtext, p_Salt[playerid], buf, 65);
		       
		        if(strcmp(buf, p_Password[playerid]) == 0) {
		        	new 
		        		query[124], name[MAX_PLAYER_NAME + 1];
					GetPlayerName(playerid, name, sizeof name);

					mysql_format(dbHandle, query, sizeof query, "SELECT * FROM `players` WHERE `NAME` = '%e' LIMIT 0,1", name);
					mysql_tquery(dbHandle, query, "f_OnPlayerLogin", "i", playerid);
				}
				else {
					new
						dialog[40], name[MAX_PLAYER_NAME + 1];
					GetPlayerName(playerid, name, sizeof name);
			   		format(dialog, sizeof dialog, "Welcome back %s", name);
			   		#if USE_DIALOG_ENUM
			    		ShowPlayerDialog(playerid, D_LOGIN, DIALOG_STYLE_PASSWORD, "Login", dialog, "Login", "Cancel");
			    	#endif
				}
			}
		}
	}

	#if !defined _inc_y_hooks && !defined _INC_y_hooks\
		&& defined mysql_OnDialogResponse
        return mysql_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]);
    #else
        return true;
    #endif
}

#if !defined _inc_y_hooks && !defined _INC_y_hooks
	#if defined _ALS_OnDialogResponse
	    #undef OnDialogResponse
	#else
	    #define _ALS_OnDialogResponse
	#endif

	#define OnDialogResponse mysql_OnDialogResponse
	#if defined mysql_OnDialogResponse
	    forward mysql_OnDialogResponse(playerid, dialogid, response, listitem, inputtext[]);
	#endif
#endif

// -- // f_*

forward f_OnPlayerLogin(playerid);
public f_OnPlayerLogin(playerid) {

	new iRows;

	if(iRows) {
		cache_get_value_name(0, "id", p_ID[playerid]);
	}

	p_Logged{playerid} = true;
	SetSpawnInfo(playerid, 0, 1, 2495.0720, -1687.5278, 13.5150, 359.6696, 0, 0, 0, 0, 0, 0);
	SpawnPlayer(playerid);
	return true;
}


forward f_GiveAccountId(playerid);
public f_GiveAccountId(playerid) {
	p_ID[playerid] = cache_insert_id();
	return true;
}

forward f_CheckPlayerData(playerid, race_check);
public f_CheckPlayerData(playerid, race_check) {

	if(race_check != p_racecheck[playerid]) 
		return Kick(playerid);

	static 
		rows,
		dialog[124],
		name[MAX_PLAYER_NAME + 1]
		; 

	GetPlayerName(playerid, name, sizeof name);

	cache_get_row_count(rows);

	if(!rows) {
   		format(dialog, sizeof dialog, "Welcome %s", name);
   		#if USE_DIALOG_ENUM
    		ShowPlayerDialog(playerid, D_REGISTER, DIALOG_STYLE_PASSWORD, "Register", dialog, "Register", "Cancel");
    	#endif
	}
	else {
		cache_get_value(0, "password", p_Password[playerid], 65);
        cache_get_value(0, "salt", p_Salt[playerid], 11);

   		format(dialog, sizeof dialog, "Welcome back %s", name);
   		#if USE_DIALOG_ENUM
    		ShowPlayerDialog(playerid, D_LOGIN, DIALOG_STYLE_PASSWORD, "Login", dialog, "Login", "Cancel");
    	#endif
    }
	return true;
}

// -- // undef_*

#undef USE_DIALOG_ENUM
#undef USE_PLAYER_VARS