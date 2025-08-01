

public Action:HUD(Handle:timer)
{
    UpdateHUDAll();
}


UpdateHUD(clients[], numClients)
{
    if (tHUD == INVALID_HANDLE || g_CurrentStage >= g_StageCount)
        return;

    int liveZombies = g_SpawnZombiedCount;
    int liveHumans = GetLiveHumanCount();

    for (int x = 0; x < numClients; x++)
    {
        if (!IsClientInGame(clients[x]) || IsFakeClient(clients[x]) || GetClientTeam(clients[x]) == CS_TEAM_NONE)
            continue;

        int target = g_iTargetZombie[clients[x]];
        float attacktime = GetGameTime() - g_fLastAttackTime[clients[x]];
        char name[64];
        if(target != -1 && IsHurtZombie[clients[x]] && attacktime <= 5.0)
        {
            if(!IsValidEdict(target) || target < 0)
               continue;

            char key[12];
            IntToString(target, key, sizeof(key));
            g_ZombieNameMap.GetString(key, name, sizeof(name))

            int hp = GetEntProp(target, Prop_Data, "m_iHealth");
            PrintHintText(clients[x], "%s\n第 %d/%d 关\n需要击杀：%d/%d\n存活人类：%d\n丧尸数量：%d\n目标: %s 剩余血量: %d", g_Stages[g_CurrentStage].name, g_CurrentStage + 1, g_StageCount, g_ZombiesKilled, g_Stages[g_CurrentStage].killRequired, liveHumans, liveZombies, name, hp);
            
        }
        else
        {
            PrintHintText(clients[x], "%s\n第 %d/%d 关\n需要击杀：%d/%d\n存活人类：%d\n丧尸数量：%d\n%s", g_Stages[g_CurrentStage].name, g_CurrentStage + 1, g_StageCount, g_ZombiesKilled, g_Stages[g_CurrentStage].killRequired, liveHumans, liveZombies, g_Stages[g_CurrentStage].storyline);
        }
    }
}


stock UpdateHUDClient(client)
{
    if (!IsClientInGame(client) || GetClientTeam(client) == CS_TEAM_NONE)
    {
        return;
    }
    
    int clients[1];
	clients[0] = client;
	
    UpdateHUD(clients, 1);
}

stock UpdateHUDAll()
{
    int maxplayers = MaxClients;
    
    new clients[maxplayers];
    int count = 0;
    
    for (int x = 1; x <= maxplayers; x++)
    {
        if (!IsClientInGame(x) || GetClientTeam(x) == CS_TEAM_NONE)
        {
            continue;
        }
        
        clients[count++] = x;
    }
    
    UpdateHUD(clients, count);
}
        
GetLiveHumanCount()
{
    int humansleft = 0;
    int maxplayers = MaxClients;
    for (int x = 1; x <= maxplayers; x++)
    {
        if (!IsClientInGame(x) || !IsPlayerAlive(x))  
        {
            continue;
        }
        
        humansleft++;
    }
    
    return humansleft;
}

/*
GetLiveZombieCount()
{
   return g_Stages[g_CurrentStage].killRequired - g_ZombiesKilled;  
}
*/