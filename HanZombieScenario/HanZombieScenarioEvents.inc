HookSdkEvents(int client)
{
    SDKHook(client, SDKHook_OnTakeDamage, TakeDamageHook);
    SDKHook(client, SDKHook_PostThinkPost, Hook_PostThinkPost);
}

HookUserMgs()
{
    HookUserMessage(GetUserMessageId("HintText"), MsgHook_HintText, true); //屏蔽 hud显示
}

public Hook_PostThinkPost(client)
{
    if(!IsValidClient(client))
        return;

    if (!GetConVarBool(g_ScenarioConfig.enableplugins))
        return;

    bool CanBuy = GetConVarBool(g_ScenarioConfig.allowBuy);
    if(!CanBuy)
        return;

    SetEntProp(client, Prop_Send, "m_bInBuyZone", 1);
}

public Action TakeDamageHook(int victim, int &attacker, int &inflictor, float &damage, int &damagetype) //设置CT与T无法互相伤害
{
    if (!GetConVarBool(g_ScenarioConfig.enableplugins))
        return Plugin_Continue;
    // 基础验证
    if (attacker <= 0 || attacker > MaxClients || !IsClientInGame(attacker) || !IsPlayerAlive(attacker))
        return Plugin_Continue;

    if (victim <= 0 || victim > MaxClients || !IsClientInGame(victim) || !IsPlayerAlive(victim))
        return Plugin_Continue;

    if (attacker != victim) 
    {
        int attackerTeam = GetClientTeam(attacker);
        int victimTeam = GetClientTeam(victim);

        if ((attackerTeam == CS_TEAM_T || attackerTeam == CS_TEAM_CT) && (victimTeam == CS_TEAM_T || victimTeam == CS_TEAM_CT) && attackerTeam != victimTeam)
        {
            return Plugin_Handled;
        }
    }

    return Plugin_Continue;
}


HookEvents()
{
    HookEvent("round_start", RoundStart);
    HookEvent("round_end", RoundEnd);
    HookEvent("round_freeze_end", RoundFreezeEnd);
    HookEvent("player_spawn", PlayerSpawn);
    HookEvent("player_death", PlayerDeath);

    HookEvent("player_team", PlayerTeam_Pre, EventHookMode_Pre);
    HookEvent("player_team", PlayerTeam_Post, EventHookMode_Post);


	HookEvent("hostage_hurt", OnHostageHurt, EventHookMode_Pre);
	HookEvent("hostage_follows", OnHostageFollows, EventHookMode_Pre);
	HookEvent("hostage_stops_following", OnHostageStopsFollowing, EventHookMode_Pre);
	HookEvent("hostage_killed", OnHostageKilled, EventHookMode_Pre);
	AddNormalSoundHook(NormalSHook:HookHostageSound);

    HookEvent("smokegrenade_detonate", OnSmokeDetonate);

    AddCommandListener(Cmd_BuyHook, "buy");

}

UnHookEvents()
{
    UnhookEvent("round_start", RoundStart);
    UnhookEvent("round_end", RoundEnd);
    UnhookEvent("round_freeze_end", RoundFreezeEnd);
    UnhookEvent("player_spawn", PlayerSpawn);
    UnhookEvent("player_death", PlayerDeath);

    UnhookEvent("player_team", PlayerTeam_Pre, EventHookMode_Pre);
    UnhookEvent("player_team", PlayerTeam_Post, EventHookMode_Post);


	UnhookEvent("hostage_hurt", OnHostageHurt, EventHookMode_Pre);
	UnhookEvent("hostage_follows", OnHostageFollows, EventHookMode_Pre);
	UnhookEvent("hostage_stops_following", OnHostageStopsFollowing, EventHookMode_Pre);
	UnhookEvent("hostage_killed", OnHostageKilled, EventHookMode_Pre);

    UnhookEvent("smokegrenade_detonate", OnSmokeDetonate);

    RemoveCommandListener(Cmd_BuyHook, "buy");

}

void ClearDisconnect(int client)
{
    ReSpawn[client] = INVALID_HANDLE;
    RespawnTime[client] = 0;
    iAccount[client] = 0;
    IsHurtZombie[client] = false;
    g_fLastAttackTime[client] = 0.0;
    g_iTargetZombie[client] = 0;
}


bool ptFire = true;
public Action PlayerTeam_Pre(Handle event, const String:name[], bool dontBroadcast)
{
    if (ptFire)
    {
        Handle hPlayerTeam = CreateEvent("player_team", true);
        
        SetEventInt(hPlayerTeam, "userid", GetEventInt(event, "userid"));
        SetEventInt(hPlayerTeam, "team", GetEventInt(event, "team"));
        SetEventInt(hPlayerTeam, "oldteam", GetEventInt(event, "oldteam"));
        SetEventBool(hPlayerTeam, "disconnect", GetEventBool(event, "disconnect"));
        
        ptFire = false;
        FireEvent(hPlayerTeam, true);
        ptFire = true;
        
        return Plugin_Handled;
    }
    
    return Plugin_Continue;
}

public Action PlayerTeam_Post(Handle event, const String:name[], bool dontBroadcast)
{
    bool disconnect = GetEventBool(event, "disconnect");
    if (disconnect)
    {
        return Plugin_Continue;
    }
    
    int index = GetClientOfUserId(GetEventInt(event, "userid"));
    int oldteam = GetEventInt(event, "oldteam");
    int team = GetEventInt(event, "team");
    
    if (team != CS_TEAM_SPECTATOR && oldteam == CS_TEAM_NONE || oldteam == CS_TEAM_SPECTATOR)
    {
        CreateTimer(0.0, CheckTeam, index);
    }
    
    if (team == CS_TEAM_CT || team == CS_TEAM_T)
    {
        
        if (IsPlayerAlive(index) || ReSpawn[index] != INVALID_HANDLE || tHUD == INVALID_HANDLE)
        {
            return Plugin_Continue;
        }
        
        StartRespawnTimer(index);
        
    }
    return Plugin_Continue;
}

public Action CheckTeam(Handle timer, any index)
{
    AssignTeamClient(index, IsPlayerAlive(index));

    return Plugin_Continue;
}

public Action RoundFreezeEnd(Handle event, const String:name[], bool dontBroadcast)
{
    RemoveObjectives();
    CleanupInvalidZombies();
    g_ZombiesKilled = 0;
    for(int i = 1; i <= MaxClients; i++)
	{
		if(IsClientConnected(i) && IsClientInGame(i) && IsPlayerAlive(i))
		{
            SetEntProp(i, Prop_Data, "m_takedamage", 2, 1);
        }
    }

    float SpawnTime = GetConVarFloat(g_ScenarioConfig.ZombieSpawnTime);

    if (IsValidHandle(FreezeEndCreateZ))
    {
        CloseHandle(FreezeEndCreateZ);
        FreezeEndCreateZ = INVALID_HANDLE;
    }
    FreezeEndCreateZ = CreateTimer(SpawnTime, Timer_ToCreateZombie, _, TIMER_FLAG_NO_MAPCHANGE);

    if (SpawnTime >= 20.0)
    {
        float playTime = SpawnTime - 20.0;

        if (IsValidHandle(FreezeEndTwentysVox))
        {
            CloseHandle(FreezeEndTwentysVox);
            FreezeEndTwentysVox = INVALID_HANDLE;
        }
        FreezeEndTwentysVox = CreateTimer(playTime, Timer_ToPlayTwentysVox, _, TIMER_FLAG_NO_MAPCHANGE);
    }

    PlayRoundVox(g_VoxSystemConfig.RoundStartVox);
    
    if(!IsCountDown)
    {
        StartCountDownVox(RoundFloat(SpawnTime));
        IsCountDown = true;
    }


    if (IsValidHandle(tAmbience))
    {
        CloseHandle(tAmbience);
        tAmbience = INVALID_HANDLE;
    }
	CreateTimer(SpawnTime, AmbienceLoop, _, TIMER_FLAG_NO_MAPCHANGE);

    return Plugin_Continue;
}

public Action Timer_ToPlayTwentysVox(Handle timer)
{
    PlayRoundVox(g_VoxSystemConfig.TwentysVox);
    for(int i = 1; i <= MaxClients; i++)
	{
		if(IsClientConnected(i) && IsClientInGame(i) && IsPlayerAlive(i))
		{
            PrintCenterText(i,"20 秒倒计时后丧尸开始行动!!");
        }
    }
    return Plugin_Stop;
}

public Action Timer_ToCreateZombie(Handle timer)
{
    GameStart = true;
    IsGameStart();
    PlayRoundVox(g_VoxSystemConfig.ZombieComeingVox);
    int random = GetRandomInt(0, 6)
    switch(random)
    {
        case 0:
        PrintCenterTextAll("击溃它们!!");
        case 1:
        PrintCenterTextAll("存活下去!!");
        case 2:
        PrintCenterTextAll("逃出这里!!");
        case 3:
        PrintCenterTextAll("直面恐惧!!");
        case 4:
        PrintCenterTextAll("寻找有利地势!!");
        case 5:
        PrintCenterTextAll("进攻就是最好的防守!!");
        default:
        PrintCenterTextAll("击溃它们!!");
    }
    
    SpawnRoundZombie();
    return Plugin_Stop;
}

void SpawnRoundZombie()
{
    int ZombieCount = 0;
    //int maxZombies = g_ScenarioConfig.maxZombieSpawn;
    int maxZombies = GetConVarInt(g_ScenarioConfig.maxZombieSpawn);

    char zombieList[64][64];
    int zombieCount = ExplodeString(g_Stages[g_CurrentStage].zombies, ",", zombieList, sizeof(zombieList), sizeof(zombieList[]));

    if (zombieCount <= 0)
    {
        PrintToServer("[ZombieScenario] 当前关卡未配置有效的丧尸类型！");
        return;
    }

    while (ZombieCount < maxZombies) 
    {
        int randIndex = GetRandomInt(0, zombieCount - 1);
        CreateZombie(zombieList[randIndex]);
        ZombieCount++;
    }

    if(g_Stages[g_CurrentStage].maxszombiecount > 0)
    {
        int maxSZombies = g_Stages[g_CurrentStage].maxszombiecount;
        int SZombieCount = 0;
        char SzombieList[64][64];
        int szombieCount = ExplodeString(g_Stages[g_CurrentStage].specialzombies, ",", SzombieList, sizeof(SzombieList), sizeof(SzombieList[]));
        if(szombieCount <= 0)
        {
            return;
        }

        while (SZombieCount < maxSZombies) 
        {
            int srandIndex = GetRandomInt(0, szombieCount - 1);
            CreateZombie(SzombieList[srandIndex]);
            SZombieCount++;
        }
    }

    // 额外生成特定数量的自定义丧尸
    if (g_Stages[g_CurrentStage].CustomZombies[0] != '\0')
    {
        SpawnCustomZombies(g_Stages[g_CurrentStage].CustomZombies);
    }

}


void SpawnCustomZombies(const char[] configStr)
{
    char entries[32][64];
    int count = ExplodeString(configStr, ",", entries, sizeof(entries), sizeof(entries[]));

    for (int i = 0; i < count; i++)
    {
        char pair[2][64];
        if (ExplodeString(entries[i], ":", pair, sizeof(pair), sizeof(pair[])) != 2)
        {
            PrintToServer("[ZombieScenario] customzombies 格式错误：%s", entries[i]);
            continue;
        }

        char zombieName[64];
        int zombieCount;

        strcopy(zombieName, sizeof(zombieName), pair[0]);
        zombieCount = StringToInt(pair[1]);

        for (int j = 0; j < zombieCount; j++)
        {
            CreateZombie(zombieName);
        }
    }
}


void RebornZombie()
{
    if(!GameStart)
    return;

    char zombieList[8][64];
    int zombieCount = ExplodeString(g_Stages[g_CurrentStage].zombies, ",", zombieList, sizeof(zombieList), sizeof(zombieList[]));

    if (zombieCount <= 0)
    {
        PrintToServer("[ZombieScenario] 当前关卡未配置有效的丧尸类型！");
        return;
    }
    int randIndex = GetRandomInt(0, zombieCount - 1);
    CreateZombie(zombieList[randIndex]);
}



public Action RoundStart(Handle event, const String:name[], bool dontBroadcast)  
{
    IsCountDown = false;
    if (tHUD != INVALID_HANDLE)
    {
        CloseHandle(tHUD);
        tHUD = INVALID_HANDLE;
    }
    tHUD = CreateTimer(5.0, HUD, _, TIMER_REPEAT|TIMER_FLAG_NO_MAPCHANGE);
        
    UpdateHUDAll();

    return Plugin_Continue;
}


public Action PlayerSpawn(Handle event, const String:name[], bool dontBroadcast)
{
    int client = GetClientOfUserId(GetEventInt(event, "userid"));

    if(!IsValidClient(client) || IsFakeClient(client))
        return Plugin_Continue;
    
    int team = GetClientTeam(client);
    if (team != CS_TEAM_T || team != CS_TEAM_CT)
    {
        return Plugin_Continue;
    }

    UpdateHUDAll();
    CreateTimer(0.1, SpawnProtect, client);

    return Plugin_Continue;
}

public Action SpawnProtect(Handle timer, any client)
{
    if(!IsValidClient(client))
        return Plugin_Stop;

    if(IsClientConnected(client) && IsClientInGame(client))
    {
        //float RemoveprotectTime = g_ScenarioConfig.playerspawnprotect;
        float RemoveprotectTime = GetConVarFloat(g_ScenarioConfig.playerspawnprotect);
        CPrintToChat(client,"{green}[华仔]你拥有 {red}%1.f {green}秒的重生保护 !!", RemoveprotectTime);
        SetEntProp(client, Prop_Data, "m_takedamage", 0, 1);
        SetEntityRenderColor(client, 0, 0, 255, 125);
        CreateTimer(RemoveprotectTime, RemoveSpawnProtect, client);
    }
    return Plugin_Stop;
}

public Action RemoveSpawnProtect(Handle timer, any client)
{
    if(!IsValidClient(client))
        return Plugin_Stop;
        
    if(IsClientConnected(client) && IsClientInGame(client))
    {
        CPrintToChat(client,"{green}[华仔]{red}你的重生保护已经结束!!");
        SetEntProp(client, Prop_Data, "m_takedamage", 2, 1);
        SetEntityRenderColor(client, 255, 255, 255, 255);
    }
    return Plugin_Stop;
}
public Action RoundEnd(Handle event, const String:name[], bool dontBroadcast)
{
    KillAllZombie();
    GameStart = false;
    if (tHUD != INVALID_HANDLE)
    {
        CloseHandle(tHUD);
        tHUD = INVALID_HANDLE;
    }
    for(int i = 1; i <= MaxClients; i++)
	{
		if(IsClientConnected(i) && IsClientInGame(i) && IsPlayerAlive(i))
		{
            SetEntProp(i, Prop_Data, "m_takedamage", 0, 1);
        }
    }
    return Plugin_Continue;
}


void KillAllZombie()
{
    for (int i = 0; i < g_SpawnZombiedCount; i++)
    {
        int zombie = g_SpawnZombied[i];

        if (!IsValidEntity(zombie)) continue;

        char classname[64];
        GetEntityClassname(zombie, classname, sizeof(classname));

        if (StrContains(classname, "npc_") != 0) continue;  

        ClearZombieZombieState(zombie);

        if (GetEntProp(zombie, Prop_Data, "m_takedamage") != 0)
        {
            SetEntProp(zombie, Prop_Data, "m_takedamage", 0, 1);
        }

        AcceptEntityInput(zombie, "Kill");
    }
    CleanupInvalidZombies();
}



public Action PlayerDeath(Handle event, const String:name[], bool dontBroadcast)
{
    int client = GetClientOfUserId(GetEventInt(event, "userid"));

    if(!IsValidClient(client) || client <= 0)
        return Plugin_Continue;

    CreateTimer(1.0, Timer_CheckZombieWin, _, TIMER_FLAG_NO_MAPCHANGE);
    
    if (tHUD == INVALID_HANDLE)
    {
        return Plugin_Continue;
    }

    UpdateHUDAll(); 

    
    if(!IsFakeClient(client))
    {
        StartRespawnTimer(client);
    }
    /*
    else
    {
        //BOT死亡后不能复活, 一旦NPC丧尸已经召唤,BOT生成会导致服务器崩溃,原因未知,也许是CSS底层代码原因
        CreateTimer(10.0, Timer_ReBornBot, client, TIMER_FLAG_NO_MAPCHANGE);
    }
    */

    return Plugin_Continue; 
}

/*//BOT死亡后不能复活, 一旦NPC丧尸已经召唤,BOT生成会导致服务器崩溃,原因未知,也许是CSS底层代码原因
public Action Timer_ReBornBot(Handle timer, any client)
{
    if (!IsClientInGame(client) || IsPlayerAlive(client))
    {
        return;
    }
    
    CS_RespawnPlayer(client);

}
*/

StartRespawnTimer(client)
{
    if(!IsValidClient(client))
        return;

    if(IsPlayerAlive(client))
        return;
    
    if(IsFakeClient(client))
        return;

    float respawn = GetConVarFloat(g_ScenarioConfig.playerrebornsec);
    if (!respawn)
    {
        return;
    }
    
    if (ReSpawn[client] != INVALID_HANDLE)
    {
        CloseHandle(ReSpawn[client]);
    }
    
    PrintCenterText(client, "你已死亡 %i 秒后重生", RoundToZero(respawn));
   
    RespawnTime[client] = RoundToZero(respawn);
    ReSpawn[client] = CreateTimer(1.0, HumanRespawn, client, TIMER_REPEAT);
}

public Action HumanRespawn(Handle timer, any client)
{
    if(IsFakeClient(client))
        return Plugin_Continue;

    if (!IsValidClient(client) || IsPlayerAlive(client))
    {
        ReSpawn[client] = INVALID_HANDLE;
        return Plugin_Continue;
    }
    int team = GetClientTeam(client);
    if (team == CS_TEAM_T || team == CS_TEAM_CT)
    {
        RespawnTime[client]--;
    }
    
    int timeremaining = (RespawnTime[client] < 0) ? 0 : RespawnTime[client];
    
    PrintCenterText(client, "你已死亡 %i 秒后重生", timeremaining);
    
    if (RespawnTime[client] > 0)
    {
        return Plugin_Continue;
    }
    
    ReSpawn[client] = INVALID_HANDLE;
    
    if (team == CS_TEAM_T || team == CS_TEAM_CT)
    {
        CS_RespawnPlayer(client);
    }
    
    return Plugin_Continue;
}

CheckHumansWin()
{
    if(!GameStart)
        return;
    
    g_ZombiesKilled++;
    if(g_ZombiesKilled >= g_Stages[g_CurrentStage].killRequired)
    {
        g_ZombiesKilled = 0;
        g_CurrentStage++;
        KillAllZombie();
        IsHumanWin();
        PlayRoundVox(g_VoxSystemConfig.HumanWinsVox);
        PlayRoundVox(g_VoxSystemConfig.WinsBgmVox);
        if(GetConVarBool(g_ScenarioConfig.WinEndRound))
        {
            TerminateRound(5.0, CSRoundEnd_CTWin);
        }
        else
        {
            DontEndRound(3.0);
        }
        
    } 
    if (g_CurrentStage + 1 > g_StageCount)
    {
        g_StageCount = 0;
        GotoNextMap();
        CreateTimer(1.0, Timer_ForceMapEnd, _, TIMER_FLAG_NO_MAPCHANGE);
    }
}

void DontEndRound(float Time)
{
    if(!GetConVarBool(g_ScenarioConfig.WinEndRound))
    {
        PrintToChatAll("DontEndRound");
        GameStart = false;
        CreateTimer(Time, CreateFakeNewRound, TIMER_FLAG_NO_MAPCHANGE);
        IsGameEnd();
    }
}

public Action CreateFakeNewRound(Handle timer)
{
    if(GetConVarBool(g_ScenarioConfig.WinEndRound))
        return Plugin_Stop;


    IsCountDown = false;

    CleanupInvalidZombies();
    g_ZombiesKilled = 0;
    float SpawnTime = GetConVarFloat(g_ScenarioConfig.ZombieSpawnTime);
    if (IsValidHandle(FreezeEndCreateZ))
    {
        CloseHandle(FreezeEndCreateZ);
        FreezeEndCreateZ = INVALID_HANDLE;
    }
    FreezeEndCreateZ = CreateTimer(SpawnTime, Timer_ToCreateZombie, _, TIMER_FLAG_NO_MAPCHANGE);
    if (SpawnTime >= 20.0)
    {
        float playTime = SpawnTime - 20.0;

        if (IsValidHandle(FreezeEndTwentysVox))
        {
            CloseHandle(FreezeEndTwentysVox);
            FreezeEndTwentysVox = INVALID_HANDLE;
        }
        FreezeEndTwentysVox = CreateTimer(playTime, Timer_ToPlayTwentysVox, _, TIMER_FLAG_NO_MAPCHANGE);
    }

    PlayRoundVox(g_VoxSystemConfig.RoundStartVox);
    
    if(!IsCountDown)
    {
        StartCountDownVox(RoundFloat(SpawnTime));
        IsCountDown = true;
    }

    return Plugin_Continue;

}




public Action Timer_ForceMapEnd(Handle timer)
{
    CS_TerminateRound(3.0, CSRoundEnd_GameStart, false);
    return Plugin_Stop;
}



public Action Timer_CheckZombieWin(Handle timer)
{
    if(!GameStart)
        return Plugin_Stop;

    if (GetLiveHumanCount() <= 0)
    {
        CPrintToChatAll("{green}[华仔] {red}所有人类死亡{green}僵尸胜利！");
        PlayRoundVox(g_VoxSystemConfig.ZombieWinsVox);
        PlayRoundVox(g_VoxSystemConfig.lostBgmVox);
        KillAllZombie();
        IsZombieWin();
        TerminateRound(5.0, CSRoundEnd_TerroristWin);
        GameStart = false;
        IsGameEnd();

        if(IsValidHandle(FreezeEndCreateZ))
        {
            CloseHandle(FreezeEndCreateZ)
            FreezeEndCreateZ = INVALID_HANDLE;
        }
    }
    return Plugin_Stop;
}

stock GotoNextMap()
{
    Handle timelimit = FindConVar("mp_timelimit");
    
    if (timelimit == INVALID_HANDLE)
    {
        return;
    }
    
    int flags = GetConVarFlags(timelimit) & FCVAR_NOTIFY;
    SetConVarFlags(timelimit, flags);
    
    SetConVarInt(timelimit, 1);
}



TerminateRound(float delay, CSRoundEndReason:reason)
{
    if (tHUD == INVALID_HANDLE)
        return;

    if(!GameStart)
        return;
    
    CS_TerminateRound(delay, reason, false);
}


public Action HookHostageSound(clients[64], &numClients, String:sample[PLATFORM_MAX_PATH], &entity, &channel, &Float:volume, &level, &pitch, &flags)
{
	char edictname[32];
	GetEdictClassname(entity, edictname, 32);
	if (StrContains(edictname, "npc_") != -1)
		return Plugin_Stop;
	
	return Plugin_Continue;
}

public Action OnHostageHurt(Handle:event, const String:name[], bool dontBroadcast)
{
	return Plugin_Handled;
}

public Action OnHostageFollows(Handle:event, const String:name[], bool dontBroadcast)
{
	return Plugin_Handled;
}

public Action:OnHostageStopsFollowing(Handle:event, const String:name[], bool dontBroadcast)
{
	return Plugin_Handled;
}

public Action OnHostageKilled(Handle event, const String:name[], bool dontBroadcast)
{
	return Plugin_Handled;
}
// ============================================================================
// >> 同步购买金钱
// ============================================================================
public Action Cmd_BuyHook(int client, const char[] command, int argc)
{
    if (IsClientInGame(client) && IsPlayerAlive(client) && !IsFakeClient(client))
    {
        CreateTimer(0.1, Timer_SyncMoney, client, TIMER_FLAG_NO_MAPCHANGE);
    }

    return Plugin_Continue;
}

public Action Timer_SyncMoney(Handle timer, any client)
{
    if (IsClientInGame(client))
    {
        SyncClientMoney(client);
        //PrintToChat(client, "购买装备金钱同步：%d", iAccount[client]);
    }
    return Plugin_Stop;
}

/* hud msg 关键字
hud msg
{
    Geiger                             
    Train                             
    HudText                         
    SayText                           
    SayText2                            
    TextMsg                             
    HudMsg                          
    ResetHUD                        
    GameTitle                          
    ItemPickup                         
    ShowMenu                         
    Shake                            
    Fade                               
    VGUIMenu                          
    CloseCaption                       
    SendAudio                       
    RawAudio                         
    VoiceMask                       
    RequestState                      
    BarTime                          
    Damage                               
    RadioText                           
    HintText                           
    ReloadEffect                       
    PlayerAnimEvent                     
    AmmoDenied                        
    UpdateRadar                         
    KillCam
}
*/
public Action MsgHook_HintText(UserMsg msg_id, Handle pb, const int[] players, int playersNum, bool reliable, bool init)
{
    /* 检查原始数据*/
    char strRest[256]="";
    char strRestc[256]="";
    int curbyte;

    while (BfGetNumBytesLeft(pb))
    {
        curbyte = BfReadByte(pb);
        Format(strRest, sizeof(strRest), "%s %d", strRest, curbyte);
        Format(strRestc, sizeof(strRestc), "%s%c", strRestc, curbyte);
    }

    //PrintToServer("[HintText] Bytes: %s", strRest);
    //PrintToServer("[HintText] Text : %s", strRestc);

    
    if (StrContains(strRestc, "hostage", false) != -1 || StrContains(strRestc, "money", false) != -1 )
    {
        return Plugin_Handled;
    }

    return Plugin_Continue;
}