// ============================================================================
// >> 删除地图目标
// ============================================================================
RemoveObjectives()
{
    char classname[64];
    int maxentities = GetMaxEntities();
    for (int x = 0; x <= maxentities; x++)
    {
        if(!IsValidEdict(x))
        {
            continue;
        }
        
        GetEdictClassname(x, classname, sizeof(classname));
        if( StrEqual(classname, "func_bomb_target") ||
            StrEqual(classname, "func_hostage_rescue") ||
            StrEqual(classname, "c4") ||
            StrEqual(classname, "hostage_entity"))
            {
                RemoveEdict(x);
            }
    }
}
// ============================================================================
// >> 清除复活Timer
// ============================================================================
void ClearPlayerReSpawnTimer()
{
    for (int x = 1; x <= MaxClients; x++)
    {
        if (!IsClientInGame(x))
        {
            continue;
        }
        
        if (ReSpawn[x] != INVALID_HANDLE)
        {
            CloseHandle(ReSpawn[x]);
            ReSpawn[x] = INVALID_HANDLE;
        }
    }

}

// ============================================================================
// >> 服务器指令
// ============================================================================

void UpdateCavr()
{
    ServerCommand("mp_timelimit 0");
    ServerCommand("mp_ignore_round_win_conditions 1");
    ServerCommand("bot_join_after_player 0");
    ServerCommand("mp_autokick 0");
    ServerCommand("mp_hostagepenalty 0");
    
}

// ============================================================================
// >> 外部伤害API
// ============================================================================
stock bool Han_DealZombieDamage(int attacker, int zombie, int damage, int damagetype, const char[] weaponname = "generic")
{
    if (!IsValidEntity(zombie) || !Han_ZombieIsAlive(zombie))
        return false;

    // 防止自伤或无效目标
    if (attacker == zombie)
        return false;

    // 使用 point_hurt 进行实际伤害
    Entity_Hurt(zombie, damage, attacker, damagetype, weaponname); //DMG_NEVERGIB

    // 同步：动画、死亡等
    char configName[64];
    strcopy(configName, sizeof(configName), g_ZombieName[zombie]);

    ZombieType z;
    z = GetZombieType(configName);
    if (strlen(z.name) == 0)
    {
        PrintToServer("[Zombie] 配置 '%s' 无效", configName);
        return false;
    }

    char anim[64];
    float animTime;
    GetRandomTimedAnim(z.getattackAnims, anim, sizeof(anim), animTime);

    // 判断是否死亡
    if (Han_HurtZombie(zombie, attacker, damage))
    {
        SDKUnhook(zombie, SDKHook_OnTakeDamage, ZombieDamageHook);

        Call_StartForward(g_ForwardZombieDeath);
        Call_PushCell(zombie);
        Call_PushCell(attacker);
        Call_Finish();
         
        SetEntDataEnt2(zombie, gLeaderOffset, zombie);
        SetEntityMoveType(zombie, MOVETYPE_VPHYSICS);
        GiveKillMoneyAndScore(attacker);
        CreateFakeKil(attacker);

        ClearZombieZombieState(zombie);
        UnregisterZombie(zombie);

        Han_ZombieDeath(zombie);
        return true;
    }

    if (!g_ZombieState[zombie].IsCustomAnim)
    {
        // 播放被击动画
        g_ZombieState[zombie].IsBeingAttacked = true;
        Han_SetAnimation(zombie, anim, animTime);

        TeleportEntity(zombie, NULL_VECTOR, NULL_VECTOR, view_as<float>({0.0, 0.0, 0.0}));
        SetEntityMoveType(zombie, MOVETYPE_NONE);
        
        CreateTimer(animTime, Timer_ResetBeingAttacked, EntIndexToEntRef(zombie));
    }
    
    return true;
}


stock bool Entity_Hurt(int entity, int damage, int attacker=0, int damageType=DMG_GENERIC, const char[] fakeClassName="")
{
	static int point_hurt = INVALID_ENT_REFERENCE;

	if (point_hurt == INVALID_ENT_REFERENCE || !IsValidEntity(point_hurt)) {
		point_hurt = EntIndexToEntRef(Entity_Create("point_hurt"));

		if (point_hurt == INVALID_ENT_REFERENCE) {
			return false;
		}

		DispatchSpawn(point_hurt);
	}

	AcceptEntityInput(point_hurt, "TurnOn");
	SetEntProp(point_hurt, Prop_Data, "m_nDamage", damage);
	SetEntProp(point_hurt, Prop_Data, "m_bitsDamageType", damageType);
	
	char orignalTargetName[128];
	Entity_GetName(entity, orignalTargetName, sizeof(orignalTargetName));
	Entity_PointHurtAtTarget(point_hurt, entity);

	if (fakeClassName[0] != '\0') {
		Entity_SetClassName(point_hurt, fakeClassName);
	}

	AcceptEntityInput(point_hurt, "Hurt", attacker);
	AcceptEntityInput(point_hurt, "TurnOff");

	if (fakeClassName[0] != '\0') {
		Entity_SetClassName(point_hurt, "point_hurt");
	}
	
	DispatchKeyValue(entity, "targetname", orignalTargetName);
	return true;
}

stock int Entity_Create(const char[] className, int ForceEdictIndex=-1)
{
	if (ForceEdictIndex != -1 && Entity_IsValid(ForceEdictIndex)) {
		return INVALID_ENT_REFERENCE;
	}

	return CreateEntityByName(className, ForceEdictIndex);
}

stock int Entity_GetName(int entity, char[] buffer, int size)
{
	return GetEntPropString(entity, Prop_Data, "m_iName", buffer, size);
}

stock void Entity_PointHurtAtTarget(int entity, int target, const char[] name="")
{
	char targetName[128];
	Entity_GetTargetName(entity, targetName, sizeof(targetName));

	if (name[0] == '\0') {

		if (targetName[0] == '\0') {
			// Let's generate our own name
			Format(
				targetName,
				sizeof(targetName),
				"_smlib_Entity_PointHurtAtTarget:%d",
				target
			);
		}
	}
	else {
		strcopy(targetName, sizeof(targetName), name);
	}

	DispatchKeyValue(entity, "DamageTarget", targetName);
	Entity_SetName(target, targetName);
}

stock bool Entity_SetClassName(int entity, const char[] className)
{
	return DispatchKeyValue(entity, "classname", className);
}

stock bool Entity_IsValid(int entity)
{
	return IsValidEntity(entity);
}

stock int Entity_GetTargetName(int entity, char[] buffer, int size)
{
	return GetEntPropString(entity, Prop_Data, "m_target", buffer, size);
}

stock bool Entity_SetTargetName(int entity, const char[] name, any ...)
{
	char format[128];
	VFormat(format, sizeof(format), name, 3);

	return DispatchKeyValue(entity, "target", format);
}

stock bool Entity_SetName(int entity, const char[] name, any ...)
{
	char format[128];
	VFormat(format, sizeof(format), name, 3);

	return DispatchKeyValue(entity, "targetname", format);
}

// ============================================================================
// >> 获取动画实体
// ============================================================================
stock int GetZombieAnimationEntity(int zombie)
{
    if (!IsValidEntity(zombie) || !IsValidEdict(zombie))
        return -1;

    char className[32];
    GetEdictClassname(zombie, className, sizeof(className));
    if (StrContains(className, "npc_") == -1)
        return -1;

    char tmp[32];
    GetEntPropString(zombie, Prop_Data, "m_iName", tmp, sizeof(tmp));

    int modelEnt = EntRefToEntIndex(StringToInt(tmp));
    if (!IsValidEntity(modelEnt))
        return -1;

    return modelEnt;
}

// ============================================================================
// >> 死亡碰撞组
// ============================================================================
void ZombieDeathSolid(int zombie)
{
    if (!IsValidEntity(zombie) || !IsValidEdict(zombie))
        return;

    if(Han_ZombieIsAlive(zombie))
        return;
    
    //SetEntProp(zombie, Prop_Data, "m_takedamage", 0, 1);

    if(GetEntPropFloat(zombie, Prop_Send, "m_flModelScale") != 1.0)
        return;

    SetEntProp(zombie, Prop_Send, "m_usSolidFlags", 1);
    SetEntProp(zombie, Prop_Data, "m_nSolidType", 0);
}


// ============================================================================
// >> 创建fake击杀事件
// ============================================================================

stock CreateFakeKil(int attacker)
{
    if(!IsValidClient(attacker))
        return;

    if(!GetConVarBool(g_ScenarioConfig.EnableKillIcon))
        return;

    Handle event = CreateEvent("player_death");
    if (event != INVALID_HANDLE)
    {
        char ClassName[30];
        int WeaponIndex = GetEntPropEnt(attacker, Prop_Send, "m_hActiveWeapon");
        GetEntityClassname(WeaponIndex, ClassName, sizeof(ClassName)); 
        ReplaceString(ClassName, sizeof(ClassName), "weapon_", "", false); 
        SetEventInt(event, "userid", 0); // victim
        SetEventInt(event, "attacker", GetClientUserId(attacker)); // attacker
        SetEventString(event, "weapon", ClassName); // weapon name
        FireEvent(event, false);
    } 
}

// ============================================================================
// >> 清除状态
// ============================================================================
void ClearZombieZombieState(int zombie)
{
    g_ZombieBurning[zombie] = false;

    g_ZombieState[zombie].IsAttacking = false;
    g_ZombieState[zombie].IsBeingAttacked = false;
    g_ZombieState[zombie].NextAnimTime = 0.0;

    g_ZombieState[zombie].IsBeFreeze = false;

    g_ZombieState[zombie].IsCustomAnim = false;
    g_ZombieState[zombie].CustomAnimEndTime = 0.0;

    g_fTargetLockExpire[zombie] = 0.0; // 解锁
    g_iForcedTarget[zombie] = 0;
}

// ============================================================================
// >> 注册/删除 丧尸
// ============================================================================

bool IsZombieEntity(int entity)
{
    if (!IsValidEntity(entity))
        return false;

    char classname[64];
    GetEntityClassname(entity, classname, sizeof(classname));

    if (StrContains(classname, "npc_") == 0)
        return true;

    return false;
}


void RegisterZombie(int entity, const char[] name)
{
    if (!IsZombieEntity(entity))
        return;

    char key[12];
    IntToString(entity, key, sizeof(key));
    g_ZombieNameMap.SetString(key, name);

    if (g_SpawnZombiedCount < sizeof(g_SpawnZombied))
    {
        g_SpawnZombied[g_SpawnZombiedCount++] = entity;
    }
}

void UnregisterZombie(int entity)
{
    for (int i = 0; i < g_SpawnZombiedCount; i++)
    {
        if (g_SpawnZombied[i] == entity)
        {
            RemoveZombieName(entity);
            g_SpawnZombied[i] = g_SpawnZombied[--g_SpawnZombiedCount];
            break;
        }
    }
}

void CleanupInvalidZombies()
{
    // 清理所有残留的丧尸名字
    for (int i = 0; i < g_SpawnZombiedCount; i++)
    {
        RemoveZombieName(g_SpawnZombied[i]);
    }
    // 直接清空数组计数
    g_SpawnZombiedCount = 0;
}


void RemoveZombieName(int entity)
{
    char key[12];
    IntToString(entity, key, sizeof(key));
    g_ZombieNameMap.Remove(key);
}

stock AssignTeamClient(client, bool spawn)
{
    if (!IsClientInGame(client))
    {
        return;
    }
  
    int clients[1];
    clients[0] = client;
    
    AssignTeam(clients, 1, spawn);
}


AssignTeam(clients[], numClients, bool spawn)
{
    for (int x = 0; x < numClients; x++)
    {
        if (!IsClientInGame(clients[x]))
        {
            continue;
        }
        
        int team = GetClientTeam(clients[x]);

        if(team == CS_TEAM_CT || team == CS_TEAM_T )
        {
            if(spawn)
            {
                if(!IsFakeClient(clients[x]))
                {
                    CS_RespawnPlayer(clients[x]);
                }
            }

        }
    }
}

void CreateAdmianCommand()
{
    RegAdminCmd("hzs_setday", Command_SetDay, ADMFLAG_GENERIC, "管理员设置关卡");
    RegAdminCmd("hzs_restgame", Command_RestGame, ADMFLAG_GENERIC, "管理员重置游戏");
}

public Action Command_RestGame(client, argc)
{
    if (!GetConVarBool(g_ScenarioConfig.enableplugins))
    {
        return Plugin_Handled;
    }

    CPrintToChatAll("{red}管理员重置了关卡");
    
    if (tHUD == INVALID_HANDLE)
    {
        return Plugin_Handled;
    }

    KillAllZombie();
    
    TerminateRound(3.0, CSRoundEnd_Draw);
    
    return Plugin_Handled;
}

public Action Command_SetDay(client, argc)
{
    if (argc < 1 || !GetConVarBool(g_ScenarioConfig.enableplugins))
    {
        return Plugin_Handled;
    }
    
    char arg1[32];
    GetCmdArg(1, arg1, sizeof(arg1));
    
    int Stage = StringToInt(arg1) - 1; 
    Stage = (Stage >= 0) ? Stage : 0;
    
    g_CurrentStage = Stage;
    g_CurrentStage = (g_CurrentStage + 1 > g_StageCount) ? g_StageCount - 1 : g_CurrentStage;
    
    CPrintToChatAll("{red}管理员跳转到 %i 关", g_CurrentStage + 1);
    
    if (tHUD == INVALID_HANDLE)
    {
        return Plugin_Handled;
    }

    KillAllZombie();
    
    TerminateRound(3.0, CSRoundEnd_Draw);
    
    return Plugin_Handled;
}

// ============================================================================
// >> 金钱相关
// ============================================================================
int GetClientMoney(int client)
{
    return GetEntProp(client, Prop_Send, "m_iAccount");
}
void SetClientMoney(int client, int amount)
{
    SetEntProp(client, Prop_Send, "m_iAccount", amount);
}

//同步金钱
void SyncClientMoney(int client)
{
    if (!IsClientInGame(client) || !IsPlayerAlive(client) || IsFakeClient(client)) 
        return;

    iAccount[client] = GetClientMoney(client);
}
// ============================================================================
// >> 回合事件API
// ============================================================================
void IsGameStart()
{
    if (g_ForwardGameStart != INVALID_HANDLE)
    {
        Call_StartForward(g_ForwardGameStart);
        Call_Finish();
    }
}

void IsGameEnd()
{
    if (g_ForwardGameEnd != INVALID_HANDLE)
    {
        Call_StartForward(g_ForwardGameEnd);
        Call_Finish();
    }
}

void IsHumanWin()
{
    if (g_ForwardHumanWin != INVALID_HANDLE)
    {
        Call_StartForward(g_ForwardHumanWin);
        Call_Finish();
    }
}

void IsZombieWin()
{
    if (g_ForwardZombieWin != INVALID_HANDLE)
    {
        Call_StartForward(g_ForwardZombieWin);
        Call_Finish();
    }
}



