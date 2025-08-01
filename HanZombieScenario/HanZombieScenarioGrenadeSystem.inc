void LoadScenarioGrenadeConfig()
{
    KeyValues kv = new KeyValues("GrenadeSysConfig");

    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "configs/HanZombieScenario/HanZombieScenarioGrenadeSysCFG.cfg");

    if (!FileExists(path))
    {
        WriteDefaultScenarioGrenadeConfig(path);
    }

    if (!FileToKeyValues(kv, path))
    {
        PrintToServer("[GrenadeSysConfig] 配置读取失败！");
        delete kv;
        return;
    }

    g_GrenadeSystemCFG.EnableGrenadeSys = KvGetNum(kv, "Enable_GrenadeSys", 1) != 0;

    g_GrenadeSystemCFG.EnableFireGrenade = KvGetNum(kv, "Enable_Fire_Grenade", 1) != 0;
    g_GrenadeSystemCFG.EnableFireGrenadetrails = KvGetNum(kv, "Enable_Fire_Grenade_trails", 1) != 0;
    g_GrenadeSystemCFG.FireGrenadeduration = KvGetFloat(kv, "Fire_Grenade_duration", 3.0);
    g_GrenadeSystemCFG.FireGrenadeBurnDamage = KvGetNum(kv, "Fire_Grenade_Burn_Damage", 1);

    g_GrenadeSystemCFG.EnableLightGrenade = KvGetNum(kv, "Enable_Light_Grenade", 1) != 0;
    g_GrenadeSystemCFG.EnableLightGrenadetrails = KvGetNum(kv, "Enable_Light_Grenade_trails", 1) != 0;
    g_GrenadeSystemCFG.LightGrenadeduration = KvGetFloat(kv, "Light_Grenade_duration", 20.0);
    g_GrenadeSystemCFG.LightGrenadeRadius = KvGetFloat(kv, "Light_Grenade_Radius", 500.0);

    g_GrenadeSystemCFG.EnableIceGrenade = KvGetNum(kv, "Enable_Ice_Grenade", 1) != 0;
    g_GrenadeSystemCFG.EnableIceGrenadetrails = KvGetNum(kv, "Enable_Ice_Grenade_trails", 1) != 0;
    g_GrenadeSystemCFG.IceGrenadeduration = KvGetFloat(kv, "Ice_Grenade_duration", 2.0);
    g_GrenadeSystemCFG.IceGrenadeRadius = KvGetFloat(kv, "Ice_Grenade_Radius", 500.0);

    delete kv;

}

void WriteDefaultScenarioGrenadeConfig(const char[] path)
{
    Handle file = OpenFile(path, "w");
    if (file == INVALID_HANDLE)
        return;

    WriteFileLine(file, "// ScenarioGrenadeConfig 大灾变手雷系统");
    WriteFileLine(file, "//  Enable_GrenadeSys 是否开启 手雷系统");
    WriteFileLine(file, "//  Enable_Fire_Grenade 是否开启燃烧手雷 爆炸后引燃丧尸");
    WriteFileLine(file, "//  Enable_Fire_Grenade_trails 是否开启手雷投掷特效");
    WriteFileLine(file, "//  Fire_Grenade_duration 燃烧手雷持续燃烧时间,设置时间过长可能引起卡顿最好 3 - 8秒");
    WriteFileLine(file, "//  Fire_Grenade_Burn_Damage 每一秒燃烧造成多少伤害");

    WriteFileLine(file, "// Enable_Light_Grenade 是否开启照明闪光弹");
    WriteFileLine(file, "// Enable_Light_Grenade_trails 是否开启照明闪光弹投掷特效");
    WriteFileLine(file, "// Light_Grenade_duration 照明弹持续时长");
    WriteFileLine(file, "// Light_Grenade_Radius 照明范围");

    WriteFileLine(file, "// Enable_Ice_Grenade 是否开启冰冻烟雾弹");
    WriteFileLine(file, "// Enable_Ice_Grenade_trails 是否开启冰冻烟雾弹投掷特效");
    WriteFileLine(file, "// Ice_Grenade_duration 冰冻持续时长");
    WriteFileLine(file, "// Ice_Grenade_Radius 冰冻弹范围");

    WriteFileLine(file, "\"GrenadeSysConfig\"");
    WriteFileLine(file, "{");
    WriteFileLine(file, "    \"Enable_GrenadeSys\"    \"1\"");

    WriteFileLine(file, "    \"Enable_Fire_Grenade\"           \"1\"");
    WriteFileLine(file, "    \"Enable_Fire_Grenade_trails\"           \"1\"");
    WriteFileLine(file, "    \"Fire_Grenade_duration\"              \"3.0\"");
    WriteFileLine(file, "    \"Fire_Grenade_Burn_Damage\"          \"1\"");

    WriteFileLine(file, "    \"Enable_Light_Grenade\"           \"1\"");
    WriteFileLine(file, "    \"Enable_Light_Grenade_trails\"           \"1\"");
    WriteFileLine(file, "    \"Light_Grenade_duration\"              \"20.0\"");
    WriteFileLine(file, "    \"Light_Grenade_Radius\"          \"500.0\"");

    WriteFileLine(file, "    \"Enable_Ice_Grenade\"           \"1\"");
    WriteFileLine(file, "    \"Enable_Ice_Grenade_trails\"           \"1\"");
    WriteFileLine(file, "    \"Ice_Grenade_duration\"              \"2.0\"");
    WriteFileLine(file, "    \"Ice_Grenade_Radius\"          \"500.0\"");


    WriteFileLine(file, "}");

    CloseHandle(file);
}

// ============================================================================
// >> He Grenade helper
// ============================================================================
public void Han_IgniteZombie(int zombie, int attacker, float duration, int dps)
{
    if (!IsValidEntity(zombie) || !Han_ZombieIsAlive(zombie))
        return;

    if (g_ZombieBurning[zombie])
        return; // 正在燃烧中，避免重复 Ignite

    g_ZombieBurning[zombie] = true;

    IgniteEntity(zombie, duration); // 火焰特效
    

    int tickCount = RoundToFloor(duration / 1.0);
    int ref = EntIndexToEntRef(zombie);

    // 使用递增延时创建多次 DOT
    for (int i = 0; i < tickCount; i++)
    {
        CreateTimer(1.0 * i, Timer_ZombieBurnTick, CreateDataPackWithZombieFire(ref, attacker, dps));
    }

    // 结束燃烧标记
    CreateTimer(duration + 0.1, Timer_ZombieBurnEnd, ref);
}


Handle CreateDataPackWithZombieFire(int ref, int attacker, int dps)
{
    Handle pack = CreateDataPack();
    WritePackCell(pack, ref);
    WritePackCell(pack, attacker);
    WritePackCell(pack, dps);
    return pack;
}

// 每秒 DOT 处理
public Action Timer_ZombieBurnTick(Handle timer, any data)
{
    ResetPack(data);
    int ref = ReadPackCell(data);
    int attacker = ReadPackCell(data);
    int dps = ReadPackCell(data);
    CloseHandle(data);

    int zombie = EntRefToEntIndex(ref);
    if (zombie <= 0 || !IsValidEntity(zombie) || !Han_ZombieIsAlive(zombie))
        return Plugin_Stop;

    float pos[3];
    GetEntPropVector(zombie, Prop_Data, "m_vecOrigin", pos);
    Han_DealZombieDamage(attacker, zombie, dps, DMG_NEVERGIB, "汽油弹烧伤");
    //FireEff(zombie);

    return Plugin_Continue;
}

// 结束标志处理
public Action Timer_ZombieBurnEnd(Handle timer, any ref)
{
    int zombie = EntRefToEntIndex(ref);
    if (zombie > 0 && IsValidEntity(zombie))
    {
        g_ZombieBurning[zombie] = false;
    }
    return Plugin_Stop;
}

stock FireEff(int zombie)
{
    SetVariantString("HelicopterMegaBomb");  
    AcceptEntityInput(zombie, "DispatchEffect"); 
}

// ============================================================================
// >> Ice Grenade helper
// ============================================================================

void PrecacheGrenadeSound()
{
    PrecacheSound("physics/glass/glass_impact_bullet4.wav", true);
	PrecacheSound("ui/freeze_cam.wav", true);
    PrecacheSound("items/nvg_on.wav", true);
    
}
public Action CreateEvent_SmokeDetonate(Handle timer, any entity)
{
	if (!IsValidEdict(entity))
	{
		return Plugin_Stop;
	}
	
	char g_szClassname[64];
	GetEdictClassname(entity, g_szClassname, sizeof(g_szClassname));
	if (!strcmp(g_szClassname, "smokegrenade_projectile", false))
	{
		float origin[3];
		GetEntPropVector(entity, Prop_Send, "m_vecOrigin", origin);
		new userid = GetClientUserId(GetEntPropEnt(entity, Prop_Send, "m_hThrower"));
	
		new Handle:event = CreateEvent("smokegrenade_detonate");
		
		SetEventInt(event, "userid", userid);
		SetEventFloat(event, "x", origin[0]);
		SetEventFloat(event, "y", origin[1]);
		SetEventFloat(event, "z", origin[2]);
		FireEvent(event);
	}
	
	return Plugin_Stop;
}

public OnSmokeDetonate(Handle event, const String:name[], bool dontBroadcast) 
{
	if (!g_GrenadeSystemCFG.EnableGrenadeSys || !g_GrenadeSystemCFG.EnableIceGrenade)
	{
		return;
	}
	
	//int client = GetClientOfUserId(GetEventInt(event, "userid"));
	
	float origin[3];
	origin[0] = GetEventFloat(event, "x"); origin[1] = GetEventFloat(event, "y"); origin[2] = GetEventFloat(event, "z");
	
	int index = MaxClients+1; float xyz[3];
	while ((index = FindEntityByClassname(index, "smokegrenade_projectile")) != -1)
	{
		GetEntPropVector(index, Prop_Send, "m_vecOrigin", xyz);
		if (xyz[0] == origin[0] && xyz[1] == origin[1] && xyz[2] == origin[2])
		{
			AcceptEntityInput(index, "kill");
		}
	}
	
	origin[2] += 10.0;
	
	float targetOrigin[3];

    int Zombie = -1;
	while ((Zombie = FindEntityByClassname(Zombie, "npc_*")) != -1) 
	{
		if(IsValidEntity(Zombie))
		{
			GetEntPropVector(Zombie, Prop_Send, "m_vecOrigin", targetOrigin);
            targetOrigin[2] += 2.0;
            if (GetVectorDistance(origin, targetOrigin) <= g_GrenadeSystemCFG.IceGrenadeRadius)
            {
                new Handle:trace = TR_TraceRayFilterEx(origin, targetOrigin, MASK_SOLID, RayType_EndPoint, FilterTarget, Zombie);
            
                if ((TR_DidHit(trace) && TR_GetEntityIndex(trace) == Zombie) || (GetVectorDistance(origin, targetOrigin) <= 100.0))
                {
                    Freeze(Zombie, g_GrenadeSystemCFG.IceGrenadeduration);
                    CloseHandle(trace);
                }
                    
                else
                {
                    CloseHandle(trace);
                    
                    GetEntPropVector(Zombie, Prop_Send, "m_vecOrigin", targetOrigin);
                    targetOrigin[2] += 55.0;
                    targetOrigin[2] -= 2.0;
            
                    trace = TR_TraceRayFilterEx(origin, targetOrigin, MASK_SOLID, RayType_EndPoint, FilterTarget, Zombie);
                
                    if ((TR_DidHit(trace) && TR_GetEntityIndex(trace) == Zombie) || (GetVectorDistance(origin, targetOrigin) <= 100.0))
                    {
                        Freeze(Zombie, g_GrenadeSystemCFG.IceGrenadeduration);
                    }
                    
                    CloseHandle(trace);
                }
            }

		}
	}


	TE_SetupBeamRingPoint(origin, 10.0, g_GrenadeSystemCFG.IceGrenadeRadius, PrecacheModel("materials/sprites/lgtning.vmt"), PrecacheModel("materials/sprites/halo01.vmt"), 1, 1, 0.2, 100.0, 1.0, {75,75,255,255}, 0, 0);
	TE_SendToAll();
	LightCreate(SMOKE, origin);
}

bool Freeze(zombie, float time)
{
	
	if (freeze_timer[zombie] != INVALID_HANDLE)
	{
		KillTimer(freeze_timer[zombie]);
		freeze_timer[zombie] = INVALID_HANDLE;
	}

    char configName[64];
    strcopy(configName, sizeof(configName), g_ZombieName[zombie]);

    ZombieType z;
    z = GetZombieType(configName)
    if (strlen(z.name) == 0)
    {
        PrintToServer("[Zombie] 配置项 '%s' 不存在", configName);
        return false;
    }

    float now = GetEngineTime();
	

    g_ZombieState[zombie].IsBeFreeze = true;

    char anim[64];
    float times;
    GetRandomTimedAnim(z.idleAnims, anim, sizeof(anim), times);

    Han_SetAnimation(zombie, anim, times);
    g_ZombieState[zombie].NextAnimTime = now + times;

    SetEntityMoveType(zombie, MOVETYPE_NONE);
	TeleportEntity(zombie, NULL_VECTOR, NULL_VECTOR, view_as<float>({0.0, 0.0, 0.0}));
	
	float vec[3];
	GetEntPropVector(zombie, Prop_Send, "m_vecOrigin", vec);
    vec[2] += 55.0;
	vec[2] -= 50.0;
	EmitAmbientSound("physics/glass/glass_impact_bullet4.wav", vec, zombie, SNDLEVEL_RAIDSIREN, SND_NOFLAGS, 0.5);

	TE_SetupGlowSprite(vec, PrecacheModel("sprites/blueglow2.vmt"), time, 2.0, 50);
	TE_SendToAll();
	
	freeze_timer[zombie] = CreateTimer(time, Unfreeze, EntIndexToEntRef(zombie), TIMER_FLAG_NO_MAPCHANGE);
	
	return true;
}

public Action Unfreeze(Handle timer, any entityRef)
{
    int zombie = EntRefToEntIndex(entityRef);
    if (!IsValidEntity(zombie))
        return Plugin_Stop;

	if (freeze_timer[zombie] != INVALID_HANDLE)
	{
        g_ZombieState[zombie].IsBeFreeze = false;
        SetEntityMoveType(zombie, MOVETYPE_STEP);
		freeze_timer[zombie] = INVALID_HANDLE;
	}
    return Plugin_Stop;
}


public bool:FilterTarget(entity, contentsMask, any:data)
{
	return (data == entity);
}
// ============================================================================
// >> Flishlight Grenade helper
// ============================================================================
public Action DoFlashLight(Handle timer, any entity)
{
	if (!IsValidEdict(entity))
	{
		return Plugin_Stop;
	}
		
	char g_szClassname[64];
	GetEdictClassname(entity, g_szClassname, sizeof(g_szClassname));
	if (!strcmp(g_szClassname, "flashbang_projectile", false))
	{
		float origin[3];
		GetEntPropVector(entity, Prop_Send, "m_vecOrigin", origin);
		origin[2] += 50.0;
		LightCreate(FLASH, origin);
		AcceptEntityInput(entity, "kill");
	}
	
	return Plugin_Stop;
}
// ============================================================================
// >> Grenade Beam helper
// ============================================================================
BeamFollowCreate(entity, color[4])
{
    TE_SetupBeamFollow(entity, PrecacheModel("materials/sprites/laserbeam.vmt"),	0, 1.0, 10.0, 10.0, 5, color);
    TE_SendToAll();	
}

LightCreate(grenade, float pos[3])   
{  
	int iEntity = CreateEntityByName("light_dynamic");
	DispatchKeyValue(iEntity, "inner_cone", "0");
	DispatchKeyValue(iEntity, "cone", "80");
	DispatchKeyValue(iEntity, "brightness", "1");
	DispatchKeyValueFloat(iEntity, "spotlight_radius", 150.0);
	DispatchKeyValue(iEntity, "pitch", "90");
	DispatchKeyValue(iEntity, "style", "1");
	switch(grenade)
	{
		case FLASH : 
		{
			DispatchKeyValue(iEntity, "_light", "255 255 255 255");
			DispatchKeyValueFloat(iEntity, "distance", g_GrenadeSystemCFG.LightGrenadeRadius);
			EmitSoundToAll("items/nvg_on.wav", iEntity, SNDCHAN_WEAPON);
			CreateTimer(g_GrenadeSystemCFG.LightGrenadeduration, Delete, iEntity, TIMER_FLAG_NO_MAPCHANGE);
		}
		case SMOKE : 
		{
			DispatchKeyValue(iEntity, "_light", "75 75 255 255");
			DispatchKeyValueFloat(iEntity, "distance", g_GrenadeSystemCFG.IceGrenadeRadius);
			EmitSoundToAll("ui/freeze_cam.wav", iEntity, SNDCHAN_WEAPON);
			CreateTimer(0.2, Delete, iEntity, TIMER_FLAG_NO_MAPCHANGE);
		}
	}
	DispatchSpawn(iEntity);
	TeleportEntity(iEntity, pos, NULL_VECTOR, NULL_VECTOR);
	AcceptEntityInput(iEntity, "TurnOn");
}

public Action Delete(Handle timer, any entity)
{
	if (IsValidEdict(entity))
	{
		AcceptEntityInput(entity, "kill");
	}
    return Plugin_Stop;
}