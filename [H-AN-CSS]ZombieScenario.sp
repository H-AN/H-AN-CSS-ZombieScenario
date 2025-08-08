#include <sourcemod>
#include <sdktools>
#include <sdkhooks>
#include <cstrike>
#include <morecolors>
#include <HanZombieScenarioAPI>

#include "HanZombieScenario/HanZombieScenarioGlobals"
#include "HanZombieScenario/HanZombieScenarioCvars"
#include "HanZombieScenario/HanZombieScenarioHelper"
#include "HanZombieScenario/HanZombieScenarioStageData"
#include "HanZombieScenario/HanZombieScenarioEvents"
#include "HanZombieScenario/HanZombieScenarioHud"
#include "HanZombieScenario/HanZombieScenarioConfig"
#include "HanZombieScenario/HanZombieScenarioAmbient"
#include "HanZombieScenario/HanZombieScenarioVoxSystem"
#include "HanZombieScenario/HanZombieScenarioGrenadeSystem"


public Plugin:myinfo =
{
    name = "[华仔]CS起源大灾变", 
    author = "华仔 H-AN", 
    description = "华仔 H-AN 大灾变", 
    version = VERSION, 
    url = "[华仔]大灾变, QQ群107866133, github https://github.com/H-AN"
};

public APLRes:AskPluginLoad2(Handle:myself, bool:late, String:error[], err_max)
{
    CreateNative("Han_SetCustomAnimation", Native_HanSetCustomAnimation);
    CreateNative("Han_IsZombie", Native_HanIsZombie);
    CreateNative("Han_GetZombieCount", Native_HanGetZombieCount);
    CreateNative("Han_GetZombieByIndex", Native_HanGetZombieByIndex);
    CreateNative("Han_GetZombieName", Native_HanGetZombieName);

    CreateNative("Han_SafeDamageZombie", Native_SafeDamageZombie);

    g_ForwardZombieCreated = new GlobalForward("Han_OnZombieCreated", ET_Ignore, Param_Cell);
    g_ForwardZombieDeath = new GlobalForward("Han_OnZombieDeath", ET_Ignore, Param_Cell, Param_Cell);
    g_ForwardZombieHurt = new GlobalForward("Han_OnZombieHurt", ET_Ignore, Param_Cell, Param_Cell);
    g_ForwardZombieAttack = new GlobalForward("Han_OnZombieAttack", ET_Ignore, Param_Cell, Param_Cell);

    g_ForwardGameStart = new GlobalForward("Han_OnGameStart", ET_Ignore);
    g_ForwardGameEnd = new GlobalForward("Han_OnGameEnd", ET_Ignore);
    g_ForwardHumanWin = new GlobalForward("Han_OnHumanWin", ET_Ignore);
    g_ForwardZombieWin = new GlobalForward("Han_OnZombieWin", ET_Ignore);

    CreateNative("Han_SetZombieTarget", Native_HanSetZombieTarget);
    CreateNative("Han_UnlockZombie", Native_HanUnlockZombie)

    
    
    return APLRes_Success;
}


public void OnPluginStart()
{
    HookEvents();
    HookUserMgs();
    InitScenarioCvars();
    AddCommandListener(Cmd_BlockAddBot, "bot_add");
    AddCommandListener(Cmd_BlockAddBot, "bot_add_ct");
    AddCommandListener(Cmd_BlockAddBot, "bot_add_t");
    CreateAdmianCommand();
    LoadScenarioConfig();   
    
}

public void OnMapStart()
{
    if (!GetConVarBool(g_ScenarioConfig.enableplugins))
    {
        ServerCommand("mp_ignore_round_win_conditions 0");
		return;
    }

    LoadScenarioConfig();

    UpdateCavr();

    MapChangeCleanup();

    InitZombieConfig();
    LoadZombieConfig();
    LoadZombieStages();
    LoadScenarioVoxConfig();
    ChangeMapLightAndSkyBox();
    LoadScenarioGrenadeConfig();

    g_ZombieNameMap = new StringMap();

    offsCollision = FindSendPropInfo("CBaseEntity", "m_CollisionGroup");
	gLeaderOffset = FindSendPropInfo("CHostage", "m_leader");
    PrecacheModel("models/blackout.mdl", true);  

    PrecacheGrenadeSound();

    CleanupInvalidZombies();

    

}

public OnPluginEnd()
{
    ScenarioEnd();
}

public void OnMapEnd()
{
    KillAllZombie();
}



/*重要: CS起源 引擎BUG, 当npc已经生成后,BOT的复活 会导致服务器崩溃 以此方式在游戏开始NPC召唤后禁止添加BOT,回合开始未创建NPC的时候 可以加BOT*/
public Action Cmd_BlockAddBot(int client, const char[] cmd, int argc) 
{
    if(GameStart)
    {
        CPrintToChat(client, "{red}[警告]{green}NPC已经生成,{red}禁止进行BOT添加操作,{green}以免出现炸服情况!!!");
        CPrintToChat(client, "{red}[警告]{green}请在NPC生成之前,回合开始的时候,进行BOT添加操作{red}以免出现炸服情况!!!");
        return Plugin_Handled;
    }
    return Plugin_Continue;
}



// ============================================================================
// >> 创建配置文件
// ============================================================================
void InitZombieConfig()
{
    if (g_bZombieConfigReady)
        return;

    char cfgDir[PLATFORM_MAX_PATH];
    char cfgFile[PLATFORM_MAX_PATH];

    BuildPath(Path_SM, cfgDir, sizeof(cfgDir), "configs/HanZombieScenario");
    if (!FileExists(cfgDir))
    {
        CreateDirectory(cfgDir, 511);
    }

    BuildPath(Path_SM, cfgFile, sizeof(cfgFile), "configs/HanZombieScenario/HanZombieScenarioZombieData.cfg");
    if (!FileExists(cfgFile))
    {
        WriteDefaultZombieConfig(cfgFile);
    }

    g_bZombieConfigReady = true;
}

// ============================================================================
// >> 写入示例文件
// ============================================================================


void WriteDefaultZombieConfig(const char[] path)
{
    Handle file = OpenFile(path, "w");
    if (file == INVALID_HANDLE)
        return;

    WriteFileLine(file, "// ZombieSystem 丧尸配置文件");
    WriteFileLine(file, "// 每个丧尸一个子节点，可配置以下字段：");
    WriteFileLine(file, "// name:             显示名称（用于HUD/提示/通用调用）");
    WriteFileLine(file, "// model:            动画实体使用的模型路径");
    WriteFileLine(file, "// friction:         摩擦力（决定移动速度，越小越快）");
    WriteFileLine(file, "// attack_range:     与玩家距离多少时可发动攻击");
    WriteFileLine(file, "// health:           血量");
    WriteFileLine(file, "// damage:           单次攻击伤害");
    WriteFileLine(file, "// hitboxsize:          受击框尺寸大小(用于定义能被攻击的部位大小 默认1.0)");
    WriteFileLine(file, "// animesize:           动画尺寸大小(定义动画实体的视觉大小 默认1.0)");
    WriteFileLine(file, "// idle_anim:        待机动画（多个用 , 分隔）");
    WriteFileLine(file, "// move_anim:        移动动画（多个用 , 分隔）");
    WriteFileLine(file, "// attack_anim:      攻击动画（多个用 , 分隔）");
    WriteFileLine(file, "// getattack_anim:      守击动画（多个用 , 分隔）");
    WriteFileLine(file, "// death_anim:       死亡动画（多个用 , 分隔）");
    WriteFileLine(file, "// color:            显示颜色（RGBA 格式，例：255 255 255 255）");
    WriteFileLine(file, "");
    WriteFileLine(file, "\"zombies\"");
    WriteFileLine(file, "{");
    WriteFileLine(file, "    \"putong_stage1\"");
    WriteFileLine(file, "    {");
    WriteFileLine(file, "        \"name\"            \"普通丧尸 一阶段\"");
    WriteFileLine(file, "        \"model\"           \"models/player/putonghe/tank_zombi_host.mdl\"");
    WriteFileLine(file, "        \"friction\"        \"1.2\"");
    WriteFileLine(file, "        \"attack_range\"    \"100.0\"");
    WriteFileLine(file, "        \"health\"          \"100\"");
    WriteFileLine(file, "        \"damage\"          \"10\"");
    WriteFileLine(file, "        \"hitbox_size\"          \"1.0\"");
    WriteFileLine(file, "        \"anime_size\"          \"1.0\"");
    WriteFileLine(file, "        \"idle_anim\"       \"idle1,idle2\"");
    WriteFileLine(file, "        \"move_anim\"       \"walk_upper_knife\"");
    WriteFileLine(file, "        \"attack_anim\"     \"attack1\"");
    WriteFileLine(file, "        \"getattack_anim\"     \"getattack1\"");
    WriteFileLine(file, "        \"death_anim\"      \"death1\"");
    WriteFileLine(file, "        \"hit_sound\"      \"hit/xxx.wav\"");
    WriteFileLine(file, "        \"hurt_sound\"      \"hurt/xxx.wav\"");
    WriteFileLine(file, "        \"death_sound\"      \"death/xxx.wav\"");
    WriteFileLine(file, "        \"color\"           \"255 255 255 255\"");
    WriteFileLine(file, "    }");
    WriteFileLine(file, "}");

    CloseHandle(file);
}

// ============================================================================
// >> 丧尸生成位置
// ============================================================================
stock void TeleportToRandomSpawn(int entity, const char[] spawnClassname = "info_player_terrorist")
{
    SetEntData(entity, offsCollision, 2, 1, true);

    float spawns[MAXSPAWNS][3];
    float angles[MAXSPAWNS][3];
    int count = 0;

    int spawnEnt = -1;
    while ((spawnEnt = FindEntityByClassname(spawnEnt, spawnClassname)) != -1)
    {
        GetEntPropVector(spawnEnt, Prop_Data, "m_vecOrigin", spawns[count]);
        GetEntPropVector(spawnEnt, Prop_Data, "m_angRotation", angles[count]);
        count++;
    }

    if (count == 0)
    {
        PrintToServer("[Zombie] 未找到出生点：%s", spawnClassname);
        return;
    }

    int iRand = GetRandomInt(0, count - 1);
    TeleportEntity(entity, spawns[iRand], angles[iRand], NULL_VECTOR);
}


// ============================================================================
// >> 读取配置文件
// ============================================================================
void LoadZombieConfig()
{
    // 初始化存储结构
    if (g_ZombieList != null)
        delete g_ZombieList;
    if (g_ZombieIndexMap != null)
        delete g_ZombieIndexMap;

    g_ZombieList = new ArrayList(sizeof(ZombieType));
    g_ZombieIndexMap = new StringMap();

    // 打开配置文件
    KeyValues kv = CreateKeyValues("zombies");
    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "configs/HanZombieScenario/HanZombieScenarioZombieData.cfg");

    if (!FileToKeyValues(kv, path))
    {
        PrintToServer("[ZombieSystem] 配置文件加载失败: %s", path);
        return;
    }

    char mapname[64];
    GetCurrentMap(mapname, sizeof(mapname));

    char mapConfigPath[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, mapConfigPath, sizeof(mapConfigPath), "configs/HanZombieScenario/%s/HanZombieScenarioZombieData.cfg", mapname);
    if (FileExists(mapConfigPath))
    {
        // 若存在地图专属配置，优先读取
        KeyValues kvMap = new KeyValues("zombies");

        if (FileToKeyValues(kvMap, mapConfigPath))
        {
            delete kv; // 删除默认配置
            kv = kvMap;

            PrintToServer("[ZombieScenario] 载入地图专属丧尸配置：%s", mapConfigPath);
        }
        else
        {
            delete kvMap;
            PrintToServer("[ZombieScenario] 地图专属丧尸配置读取失败，使用默认配置。");
        }
    }

    if (!KvGotoFirstSubKey(kv))
    {
        PrintToServer("[ZombieSystem] 没有找到任何丧尸节点");
        return;
    }

    int count = 0;

    do {
        char section[64];
        KvGetSectionName(kv, section, sizeof(section));

        ZombieType z;

        KvGetString(kv, "name", z.name, sizeof(z.name));
        KvGetString(kv, "model", z.model, sizeof(z.model));
        z.friction = KvGetFloat(kv, "friction", 1.2);
        z.attackRange = KvGetFloat(kv, "attack_range", 100.0);
        z.health = KvGetNum(kv, "health", 100);
        z.damage = KvGetNum(kv, "damage", 10);
        z.hitboxsize = KvGetFloat(kv, "hitbox_size", 1.0);
        z.animesize = KvGetFloat(kv, "anime_size", 1.0);

        z.idleAnims = CreateArray(64);
        z.moveAnims = CreateArray(64);
        z.attackAnims = CreateArray(64);
        z.getattackAnims = CreateArray(64);
        z.deathAnims = CreateArray(64);

        z.hitsound = CreateArray(64);
        z.hurtsound = CreateArray(64);
        z.deathsound = CreateArray(64);

        ParseAnimList(kv, "idle_anim", z.idleAnims);
        ParseAnimList(kv, "move_anim", z.moveAnims);
        ParseAnimList(kv, "attack_anim", z.attackAnims);
        ParseAnimList(kv, "getattack_anim", z.getattackAnims);
        ParseAnimList(kv, "death_anim", z.deathAnims);

        ParseAnimList(kv, "hit_sound", z.hitsound);
        ParseAnimList(kv, "hurt_sound", z.hurtsound);
        ParseAnimList(kv, "death_sound", z.deathsound);

        char colorStr[32];
        KvGetString(kv, "color", colorStr, sizeof(colorStr));
        ExplodeColor(colorStr, z.r, z.g, z.b, z.a);

        // 插入并索引
        int index = g_ZombieList.PushArray(z);
        g_ZombieIndexMap.SetValue(section, index);

        PrecacheZombieResources(z);

        count++;
    } while (KvGotoNextKey(kv));

    CloseHandle(kv);
    PrintToServer("[ZombieSystem] 成功加载 %d 种丧尸配置", count);
    
}

void PrecacheZombieResources(ZombieType z)
{
    PrecacheModelSafe(z.model);
    PrecacheSoundArray(z.hitsound);
    PrecacheSoundArray(z.hurtsound);
    PrecacheSoundArray(z.deathsound);
}


// ============================================================================
// >> 创建丧尸
// ============================================================================
stock int CreateZombie(const char[] configName)
{
    ZombieType z;
    z = GetZombieType(configName);
    if (strlen(z.name) == 0)
    {
        PrintToServer("[Zombie] 配置项 '%s' 不存在", configName);
        return -1;
    }

    

    char entIndex[16];

    int zombie = CreateEntityByName("hostage_entity"); 
    if (zombie == -1)
        return -1;

    int visual = CreateEntityByName("prop_dynamic_ornament");
    if (visual == -1)
        return -1;

    IntToString(EntIndexToEntRef(visual), entIndex, sizeof(entIndex)-1);

    DispatchKeyValue(zombie, "classname", "npc_zombie");
	DispatchKeyValue(zombie, "targetname", entIndex);
	DispatchKeyValue(zombie, "disableshadows", "1");
	DispatchKeyValueFloat(zombie, "friction", z.friction);  //移动速度
	DispatchSpawn(zombie);

    SetEntPropFloat(zombie, Prop_Send, "m_flModelScale", z.hitboxsize);
    int totalhealth = z.health + g_Stages[g_CurrentStage].healthBoost;
    SetEntProp(zombie, Prop_Data, "m_iHealth", totalhealth); //血量 
    SetEntityModel(zombie, "models/blackout.mdl"); // 隐身占位模型

    char anim[64];
    float time;
    GetRandomTimedAnim(z.idleAnims, anim, sizeof(anim), time);
    Han_SetAnimation(zombie, anim, time);

    DispatchKeyValue(visual, "model", z.model); //动画模型
	DispatchKeyValue(visual, "DefaultAnim", anim);
	DispatchKeyValue(visual, "spawnflags", "320");
	DispatchKeyValue(visual, "parent", entIndex);
	DispatchSpawn(visual); //生成动画

    SetEntPropFloat(visual, Prop_Send, "m_flModelScale", z.animesize);

    SetVariantString(entIndex);
	AcceptEntityInput(visual, "SetAttached");

    SetEntityRenderColor(visual, z.r, z.g, z.b, z.a);
    SetEntityRenderColor(zombie, z.r, z.g, z.b, z.a);

    RegisterZombie(zombie, z.name); //注册丧尸

    Call_StartForward(g_ForwardZombieCreated);
    Call_PushCell(zombie);
    Call_Finish();

    ClearZombieZombieState(zombie);

    TeleportToRandomSpawn(zombie);

    strcopy(g_ZombieName[zombie], sizeof(g_ZombieName[]), configName);

    SDKHook(zombie, SDKHook_Think, ZombieThink);
    SDKHook(zombie, SDKHook_OnTakeDamage, ZombieDamageHook);

    return zombie;
}

// ============================================================================
// >> 丧尸被攻击逻辑
// ============================================================================

public Action ZombieDamageHook(int zombie, int &attacker, int &inflictor, float &damage, int &damagetype)
{
    char configName[64];
    strcopy(configName, sizeof(configName), g_ZombieName[zombie]);

    ZombieType z;
    z = GetZombieType(configName);
    if (strlen(z.name) == 0)
    {
        PrintToServer("[Zombie] 配置项 '%s' 不存在", configName);
        return Plugin_Continue;
    }

    if (IsValidEntity(inflictor))
    {
        char classname[64];
        GetEdictClassname(inflictor, classname, sizeof(classname));
        if (StrEqual(classname, "汽油弹烧伤") || StrEqual(classname, "entityflame")
           ||StrEqual(classname, "external"))
        {
            return Plugin_Handled;
        }

        if (StrEqual(classname, "hegrenade_projectile"))
        {
            if (g_GrenadeSystemCFG.EnableGrenadeSys && g_GrenadeSystemCFG.EnableFireGrenade)
            {
                float duration = g_GrenadeSystemCFG.FireGrenadeduration;
                int dps = g_GrenadeSystemCFG.FireGrenadeBurnDamage;
                Han_IgniteZombie(zombie, attacker, duration, dps);
            }
        }
    }

    char anim[64];
    float animTime;
    GetRandomTimedAnim(z.getattackAnims, anim, sizeof(anim), animTime);

    if(Han_HurtZombie(zombie, attacker, RoundToZero(damage)))
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

        //fakeRoll(zombie);
        
        ClearZombieZombieState(zombie);

        UnregisterZombie(zombie);

        Han_ZombieDeath(zombie);
        return Plugin_Handled;
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

    //char name[64];
    //GetEdictClassname(inflictor, name, sizeof(name));

    //PrintToChatAll("inflictor %s",name);

    return Plugin_Handled;
}

/* 测试功能 死亡后 隐藏动画 显示真实实体 交换双方模型, 应用于 有布娃娃的模型
void fakeRoll(int zombie)
{

    char tmp[32];
    GetEntPropString(zombie, Prop_Data, "m_iName", tmp, sizeof(tmp));
    int zombie_ent = EntRefToEntIndex(StringToInt(tmp));

    char AnimeMdl[128];
    GetEntPropString(zombie_ent, Prop_Data, "m_ModelName", AnimeMdl, sizeof(AnimeMdl));

    SetEntityModel(zombie, AnimeMdl);

    SetEntityModel(zombie_ent, "models/blackout.mdl");
}
*/

public Action Timer_ResetBeingAttacked(Handle timer, int ref)
{
    int zombie = EntRefToEntIndex(ref);
    if (!IsValidEntity(zombie))
        return Plugin_Stop;

    g_ZombieState[zombie].IsBeingAttacked = false;
    return Plugin_Handled;
}



void GiveKillMoneyAndScore(int attacker)
{
    if (!IsValidClient(attacker))
        return;
    
    SetEntProp(attacker, Prop_Data, "m_iFrags", GetClientFrags(attacker) + 1);

    int Kmoney = GetConVarInt(g_ScenarioConfig.killmoney);
    int Mmoney = GetConVarInt(g_ScenarioConfig.Maxmoney);

    if (iAccount[attacker] < Mmoney && Kmoney > 0) //使用iAccount数组记录真实金钱
    {
        iAccount[attacker]+= Kmoney;
        if(iAccount[attacker] > Mmoney)
        {
            iAccount[attacker] = Mmoney;
        }
        SetEntProp(attacker, Prop_Send, "m_iAccount", iAccount[attacker]);
    }

    CreateTimer(0.4, Timer_FixMoney, attacker, TIMER_FLAG_NO_MAPCHANGE); //延迟同步真实金钱,以免因击杀人质被扣除
}

public Action Timer_FixMoney(Handle timer, any client)
{
    if (IsClientInGame(client))
    {
        FixMoney(client);
    }
    return Plugin_Continue;
}

void FixMoney(int client)
{
    int currentMoney = GetEntProp(client, Prop_Send, "m_iAccount");
    if (currentMoney != iAccount[client])
    {
        SetClientMoney(client, iAccount[client]);
    }
}



// ============================================================================
// >> 丧尸行为
// ============================================================================

public void ZombieThink(int zombie)
{
    ZombieDeathSolid(zombie);

    if (!Han_ZombieIsAlive(zombie))
    {
        return;
    }

    ZombieAvoidOverlap(zombie); //模拟碰撞体积
    // 获取配置
    char configName[64];
    strcopy(configName, sizeof(configName), g_ZombieName[zombie]);

    ZombieType z;
    z = GetZombieType(configName)
    if (strlen(z.name) == 0)
    {
        PrintToServer("[Zombie] 配置项 '%s' 不存在", configName);
        return;
    }

    float now = GetEngineTime();
    
    // 外部动画优先判断
    if (g_ZombieState[zombie].IsCustomAnim)
    {

        if (now < g_ZombieState[zombie].CustomAnimEndTime)
        {
            return; 
        }
        else
        {
            g_ZombieState[zombie].IsCustomAnim = false;
        }

    }
    else if (now < g_ZombieState[zombie].NextAnimTime)
    {
        return; // 内部动画未结束
    }

    // 尝试获取或更新目标
    int target = -1;
    Han_Think(zombie, target);

    // 目标无效，跳过行为
    if (target <= 0 || !IsClientInGame(target))
        return;

    float pos[3], targetPos[3];
    GetEntPropVector(zombie, Prop_Data, "m_vecOrigin", pos);
    GetClientEyePosition(target, targetPos);

    float dist = GetVectorDistance(pos, targetPos);

    // 受击状态下不做其他行为
    if (g_ZombieState[zombie].IsBeingAttacked)
    {
        return;
    }
        

    if(g_ZombieState[zombie].IsBeFreeze)
    {
        return;
    }


    // 攻击逻辑（近距离 + 可视 + 非攻击中）
    if (!g_ZombieState[zombie].IsAttacking && dist < z.attackRange && Han_ZombieCanSee(zombie, target))
    {
        g_ZombieState[zombie].IsAttacking = true;

        char anim[64];
        float time;
        GetRandomTimedAnim(z.attackAnims, anim, sizeof(anim), time);

        Han_SetAnimation(zombie, anim, time);
        g_ZombieState[zombie].NextAnimTime = now + time;

        TeleportEntity(zombie, NULL_VECTOR, NULL_VECTOR, view_as<float>({0.0, 0.0, 0.0}));
        SetEntityMoveType(zombie, MOVETYPE_NONE);
        
        CreateTimer(time, Timer_ResetAttackState, EntIndexToEntRef(zombie));
        
        float damage = float(z.damage);

        Call_StartForward(g_ForwardZombieAttack);
        Call_PushCell(zombie);
        Call_PushCell(target);
        Call_Finish();

        char hitsound[64];
        GetRandomSound(z.hitsound, hitsound, sizeof(hitsound));
        EmitSoundToAll(hitsound, 0, SNDCHAN_AUTO, SNDLEVEL_NORMAL, SND_NOFLAGS, 0.5, SNDPITCH_NORMAL, -1, pos);
        SDKHooks_TakeDamage(target, zombie, zombie, damage, DMG_SLASH, _, _, _, false); // bypassHooks false 禁止hook绕过伤害
        

        return;
    }

    // 非攻击状态下播放移动动画
    if (!g_ZombieState[zombie].IsAttacking)
    {
        TeleportEntity(zombie, NULL_VECTOR, NULL_VECTOR, view_as<float>({0.0, 0.0, 0.0}));
        SetEntityMoveType(zombie, MOVETYPE_STEP);
        
        char anim[64];
        float time;
        GetRandomTimedAnim(z.moveAnims, anim, sizeof(anim), time);

        Han_SetAnimation(zombie, anim, time);
        g_ZombieState[zombie].NextAnimTime = now + time;
    }
}


public Action Timer_ResetAttackState(Handle timer, int ref)
{
    int zombie = EntRefToEntIndex(ref);
    if (!IsValidEntity(zombie))
        return Plugin_Stop;

    g_ZombieState[zombie].IsAttacking = false;
    return Plugin_Handled;
}

// ============================================================================
// >> 模拟碰撞体积
// ============================================================================

void ZombieAvoidOverlap(int Zombie)
{
    float vZombiePosition[3];
    GetEntPropVector(Zombie, Prop_Data, "m_vecOrigin", vZombiePosition);

    for (int i = 0; i < g_SpawnZombiedCount; i++)
    {
        int otherzombie = g_SpawnZombied[i];

        if (!IsValidEntity(otherzombie)) 
            continue;

        char classname[64];
        GetEntityClassname(otherzombie, classname, sizeof(classname));

        if (StrContains(classname, "npc_") != 0) 
            continue;  

        if(otherzombie != Zombie)
        {
            float otherOrigin[3];
            GetEntPropVector(otherzombie, Prop_Data, "m_vecOrigin", otherOrigin);
            if(GetVectorDistance(vZombiePosition, otherOrigin) <= 60.0)
            {
                float fDirection[3];
                MakeVectorFromPoints(otherOrigin, vZombiePosition, fDirection);
                NormalizeVector(fDirection, fDirection);              
                float fForce = -120.0; 
                ScaleVector(fDirection, fForce);
                TeleportEntity(otherzombie, NULL_VECTOR, NULL_VECTOR, fDirection);
            }
        }
        
    }
}

// ============================================================================
// >> helper
// ============================================================================

void PrecacheModelSafe(const char[] modelPath)
{
    if (FileExists(modelPath, true))
    {
        PrecacheModel(modelPath, true);
    }
    else
    {
        PrintToServer("[ZombieSystem] 模型不存在: %s", modelPath);
    }
}

void PrecacheSoundSafe(const char[] soundPath)
{
    char fullpath[PLATFORM_MAX_PATH];
    Format(fullpath, sizeof(fullpath), "sound/%s", soundPath);

    //PrintToServer("输出: %s", soundPath);

    if (FileExists(fullpath, true))
    {
        PrecacheSound(soundPath, true);
    }
    else
    {
        PrintToServer("[ZombieSystem] 音效不存在: %s", soundPath);
    }
}

void PrecacheSoundArray(ArrayList sounds)
{
    char sound[PLATFORM_MAX_PATH];
    for (int i = 0; i < sounds.Length; i++)
    {
        sounds.GetString(i, sound, sizeof(sound));
        PrecacheSoundSafe(sound);
    }
}


void ParseAnimList(KeyValues kv, const char[] key, ArrayList list)
{
    char raw[256];
    KvGetString(kv, key, raw, sizeof(raw));

    if (strlen(raw) == 0)
        return;

    char anims[16][64];
    int count = ExplodeString(raw, ",", anims, sizeof(anims), sizeof(anims[]));

    for (int i = 0; i < count; i++)
    {
        TrimString(anims[i]);
        list.PushString(anims[i]);
    }
}

void ExplodeColor(const char[] str, int &r, int &g, int &b, int &a)
{
    int c[4] = {255, 255, 255, 255};
    char split[4][8];
    int n = ExplodeString(str, " ", split, 4, sizeof(split[]));

    for (int i = 0; i < n; i++)
        c[i] = StringToInt(split[i]);

    r = c[0];
    g = c[1];
    b = c[2];
    a = c[3];
}


void GetRandomSound(ArrayList list, char[] buffer, int maxlen)
{
    if (list == null || list.Length == 0)
    {
        strcopy(buffer, maxlen, "");
        return;
    }

    list.GetString(GetRandomInt(0, list.Length - 1), buffer, maxlen);
}


void GetRandomTimedAnim(ArrayList list, char[] animName, int nameLen, float &duration)
{
    if (list == null || list.Length == 0)
    {
        strcopy(animName, nameLen, "");
        duration = 0.0;
        return;
    }

    char raw[128];
    list.GetString(GetRandomInt(0, list.Length - 1), raw, sizeof(raw));

    int sep = FindCharInString(raw, ':');
    if (sep != -1)
    {
        strcopy(animName, nameLen, raw);
        animName[sep] = '\0'; 
        duration = StringToFloat(raw[sep + 1]);
    }
    else
    {
        strcopy(animName, nameLen, raw);
        duration = 1.0; // 默认时间
    }
}


ZombieType GetZombieType(const char[] name)
{
    ZombieType z;

    int index;
    if (!g_ZombieIndexMap.GetValue(name, index))
    {
        // 手动赋默认值
        z.name[0] = '\0';
        z.model[0] = '\0';
        z.friction = 0.0;
        z.attackRange = 0.0;
        z.health = 0;
        z.damage = 0;
        z.hitboxsize = 0.0;
        z.animesize = 0.0;

        // 创建空的 ArrayList 指针（或 null）
        z.idleAnims = null;
        z.moveAnims = null;
        z.attackAnims = null;
        z.getattackAnims = null;
        z.deathAnims = null;

        z.hitsound = null;
        z.hurtsound = null;
        z.deathsound = null;

        z.r = 0;
        z.g = 0;
        z.b = 0;
        z.a = 0;

        return z;
    }

    g_ZombieList.GetArray(index, z);
    return z;
}

stock bool IsValidClient(client)
{
    return (client > 0 && client <= MaxClients && IsClientInGame(client));
}

bool Han_ZombieIsAlive(int zombie)
{
    return GetEntProp(zombie, Prop_Data, "m_iHealth") > 0;
}

bool Han_HurtZombie(zombie, attacker, damage = 0)
{
    
	char edictname[32];
	GetEdictClassname(attacker, edictname, 32);
	if(!StrEqual(edictname, "player")) 
    return false;
    

    char configName[64];
    strcopy(configName, sizeof(configName), g_ZombieName[zombie]);

    ZombieType z;
    z = GetZombieType(configName);
    if (strlen(z.name) == 0)
    {
        PrintToServer("[Zombie] 配置项 '%s' 不存在", configName);
        return -1;
    }

    Call_StartForward(g_ForwardZombieHurt);
    Call_PushCell(zombie);
    Call_PushCell(attacker);
    Call_Finish();
     
    int health = GetEntProp(zombie, Prop_Data, "m_iHealth") - damage;
    if(health < 0)
    {
        health = 0;
    }
    SetEntProp(zombie, Prop_Data, "m_iHealth", health);
    float position[3];
    GetEntPropVector(zombie, Prop_Send, "m_vecOrigin", position);
    
    IsHurtZombie[attacker] = true;
    g_iTargetZombie[attacker] = zombie;
    g_fLastAttackTime[attacker] = GetGameTime();
    UpdateHUDClient(attacker);
    
    if(GetConVarBool(g_ScenarioConfig.ShowPrintDamage))
    {
        PrintCenterText(attacker, "造成 %i 伤害", damage);
    }

    FixMoney(attacker);

    char hurtsound[64];
    GetRandomSound(z.hurtsound, hurtsound, sizeof(hurtsound));
    EmitSoundToAll(hurtsound, 0, SNDCHAN_AUTO, SNDLEVEL_NORMAL, SND_NOFLAGS, 0.5, SNDPITCH_NORMAL, -1, position);
    CreateBloodSprite(zombie);
    

	if(!GetEntDataEnt2(zombie, gLeaderOffset)) 
    {
        SetEntDataEnt2(zombie, gLeaderOffset, attacker);
    }
	if(health <= 0) 
    {
        return true;
    }
	else 
    {
        return false;
    }
}



void CreateBloodSprite(int zombie)
{
    float vEntPosition2[3], vAngle[3];
	GetEntPropVector(zombie, Prop_Send, "m_vecOrigin", vEntPosition2);

	vEntPosition2[2] += 45.0;
	
	vAngle[0] = GetRandomFloat(-3.0, 3.0);
	vAngle[1] = GetRandomFloat(-3.0, 3.0);
	vAngle[2] = GetRandomFloat(-3.0, 3.0);

	TE_SetupBloodSprite(vEntPosition2, vAngle, {102, 0, 0, 255}, 5, PrecacheModel("sprites/bloodspray.vmt"), PrecacheModel("sprites/blood.vmt"));
	TE_SendToAll();
	TE_SetupBloodSprite(vEntPosition2, vAngle, {102, 0, 0, 255}, 6, PrecacheModel("sprites/bloodspray.vmt"), PrecacheModel("sprites/blood.vmt"));
	TE_SendToAll();
	TE_SetupBloodSprite(vEntPosition2, vAngle, {102, 0, 0, 255}, 7, PrecacheModel("sprites/bloodspray.vmt"), PrecacheModel("sprites/blood.vmt"));
	TE_SendToAll();
	TE_SetupBloodSprite(vEntPosition2, vAngle, {102, 0, 0, 255}, 8, PrecacheModel("sprites/bloodspray.vmt"), PrecacheModel("sprites/blood.vmt"));
	TE_SendToAll();
}

stock Han_ZombieDeath(zombie) 
{ 
    if(IsValidEntity(zombie) && IsValidEdict(zombie))
	{
        SetEntDataEnt2(zombie, gLeaderOffset, zombie);

        char configName[64];
        strcopy(configName, sizeof(configName), g_ZombieName[zombie]);

        ZombieType z;
        z = GetZombieType(configName);
        if (strlen(z.name) == 0)
        {
            PrintToServer("[Zombie] 配置项 '%s' 不存在", configName);
            return -1;
        }

        SetEntityMoveType(zombie, MOVETYPE_VPHYSICS);	

		float position[3];
		GetEntPropVector(zombie, Prop_Send, "m_vecOrigin", position);

        char deathsound[64];
        GetRandomSound(z.deathsound, deathsound, sizeof(deathsound));

		EmitSoundToAll(deathsound, 0, SNDCHAN_AUTO, SNDLEVEL_NORMAL, SND_NOFLAGS, 0.5, SNDPITCH_NORMAL, -1, position);
        
		char tmp[32]; float Pos[3], Angle[3];
		GetEntPropString(zombie, Prop_Data, "m_iName", tmp, sizeof(tmp));
		GetEntPropVector(zombie, Prop_Send, "m_vecOrigin", Pos);
		GetEntPropVector(zombie, Prop_Send, "m_angRotation", Angle);

        char anim[64];
        float time;
        GetRandomTimedAnim(z.deathAnims, anim, sizeof(anim), time);
        Han_SetAnimation(zombie, anim, time);

        CheckHumansWin(); 
        UpdateHUDAll(); 
	
		CreateTimer(time, kill, EntIndexToEntRef(zombie));
    }
	


}


stock void Han_Think(int zombie, int &target, float height = 55.0)
{
    if (!Han_ZombieIsAlive(zombie))
    {
        SetEntDataEnt2(zombie, gLeaderOffset, zombie);
        g_fTargetLockExpire[zombie] = 0.0;
        g_iForcedTarget[zombie] = 0;
        target = zombie;
        return;
    }

    float gameTime = GetGameTime();

    // 如果被锁定
    if (g_fTargetLockExpire[zombie] < 0.0 || g_fTargetLockExpire[zombie] > gameTime)
    {
        int locked = g_iForcedTarget[zombie];
        if (IsValidClient(locked) && IsPlayerAlive(locked))
        {
            SetEntDataEnt2(zombie, gLeaderOffset, locked);
            target = locked;
            return;
        }
        else
        {
            // 强制目标已失效，解除锁定
            g_fTargetLockExpire[zombie] = 0.0;
            g_iForcedTarget[zombie] = 0;
        }
    }

    // 否则寻找最近的玩家目标
    float entityPos[3], clientPos[3], dist = 0.0;
    GetEntPropVector(zombie, Prop_Send, "m_vecOrigin", entityPos);

    for (int i = 1; i <= MaxClients; i++)
    {
        if (IsClientConnected(i) && IsClientInGame(i) && IsPlayerAlive(i))
        {
            GetClientEyePosition(i, clientPos);
            float distance = GetVectorDistance(entityPos, clientPos, false);
            if (Han_ZombieCanSee(zombie, i, height))
            {
                if (dist == 0.0 || distance < dist)
                {
                    SetEntDataEnt2(zombie, gLeaderOffset, i);
                    target = i;
                    dist = distance;
                }
            }
        }
    }

    // 如果没有任何目标，设置为自己
    if (target == 0)
    {
        SetEntDataEnt2(zombie, gLeaderOffset, zombie);
        target = zombie;
    }
}


//能看见的目标射线
stock Han_ZombieCanSee(zombie, target, float zombieheight = 55.0) 
{
	float vzombiePosition[3], vTargetPosition[3];	
	GetEntPropVector(zombie, Prop_Send, "m_vecOrigin", vzombiePosition);
	vzombiePosition[2] += zombieheight;		
	if(IsClientInGame(target))
	{
		GetClientAbsOrigin(target, vTargetPosition);
		vTargetPosition[2] += zombieheight;

		Handle trace = TR_TraceRayFilterEx(vzombiePosition, vTargetPosition, MASK_SOLID_BRUSHONLY, RayType_EndPoint, tracerayfilternotdefault);	      
		if(TR_DidHit(trace))
		{
			CloseHandle(trace);
			return true; 
		}
		CloseHandle(trace);
		return true; 
	}
	return true;  
}

//========================================================================================================================================================
//怪物射线
//========================================================================================================================================================
public bool tracerayfilternotdefault(entity, contentsMask, any data)
{
	if(entity != data || entity == 0)	
		return false;

	return true;
}

//========================================================================================================================================================
//死亡与复活
//========================================================================================================================================================
public Action kill(Handle timer, any entityRef) 
{
    int zombie = EntRefToEntIndex(entityRef);
    int zombie_tmp;
	if(zombie != -1)
	{
		char tmp[32]; float Pos[3], Angle[3];
		GetEntPropString(zombie, Prop_Data, "m_iName", tmp, sizeof(tmp));
		GetEntPropVector(zombie, Prop_Send, "m_vecOrigin", Pos);
		GetEntPropVector(zombie, Prop_Send, "m_angRotation", Angle);
		zombie_tmp = StringToInt(tmp);
		SetEntityMoveType(zombie, MOVETYPE_VPHYSICS);
		AcceptEntityInput(zombie, "Kill");
        RebornZombie();
	}
    if(zombie_tmp != -1)
    {
        AcceptEntityInput(zombie_tmp, "Kill");
    }
    return Plugin_Continue;
}

//========================================================================================================================================================
//获取目标
//========================================================================================================================================================
stock Han_ZombieGetTarget(zombie)
{
    int target = GetEntDataEnt2(zombie, gLeaderOffset);
    
    if (target > 0 && target <= MaxClients && IsClientInGame(target) && IsPlayerAlive(target))
        return target;

    return 0;
}


//========================================================================================================================================================
//动画控制逻辑
//========================================================================================================================================================

stock void Han_SetAnimation(int zombie, const char animation[64], float animTime = 0.0, float delay = 0.0)
{
    if (!IsValidEntity(zombie) || !IsValidEdict(zombie))
        return;

    char className[32];
    GetEdictClassname(zombie, className, sizeof(className));
    if (StrContains(className, "npc_") == -1)
        return;

    // 动画立即执行
    if (delay <= 0.0)
    {
        char tmp[32];
        GetEntPropString(zombie, Prop_Data, "m_iName", tmp, sizeof(tmp));
        int zombie_ent = EntRefToEntIndex(StringToInt(tmp));

        if (IsValidEntity(zombie_ent))
        {
            SetVariantString(animation);
            AcceptEntityInput(zombie_ent, "SetAnimation");

            g_ZombieState[zombie].NextAnimTime = GetEngineTime() + animTime;
        }
    }
    else
    {
        g_ZombieState[zombie].NextAnimTime = GetEngineTime() + delay + animTime;

        Handle pack = CreateDataPack();
        WritePackCell(pack, EntIndexToEntRef(zombie));
        WritePackFloat(pack, animTime);
        WritePackString(pack, animation);

        
        CreateDataTimer(delay, Timer_DelayedAnimation, pack, TIMER_FLAG_NO_MAPCHANGE);

    }
}

public Action Timer_DelayedAnimation(Handle timer, Handle pack)
{
    ResetPack(pack);

    int ref = ReadPackCell(pack);
    float animTime = ReadPackFloat(pack);
    char anim[64];
    ReadPackString(pack, anim, sizeof(anim));

    CloseHandle(pack);  

    int zombie = EntRefToEntIndex(ref);
    if (IsValidEntity(zombie))
    {
        Han_SetAnimation(zombie, anim, animTime);
    }

    return Plugin_Handled;
}

//========================================================================================================================================================
//SdkHook
//========================================================================================================================================================

public OnClientPutInServer(client)
{
    HookSdkEvents(client);
    ReSpawn[client] = INVALID_HANDLE;
    iAccount[client] = GetConVarInt(g_ScenarioConfig.Maxmoney); //进入服务器设置金钱为最大金钱
}

public void OnClientDisconnect(int client)
{
    ClearDisconnect(client);
}


//========================================================================================================================================================
//OnEntityCreated
//========================================================================================================================================================

public OnEntityCreated(entity, const String:classname[])
{
    if (!g_GrenadeSystemCFG.EnableGrenadeSys)
	{
		return;
	}

    if (!strcmp(classname, "hegrenade_projectile"))
	{
        if(!g_GrenadeSystemCFG.EnableFireGrenade)
        {
            return;
        }

        if(g_GrenadeSystemCFG.EnableFireGrenadetrails)
        {
            BeamFollowCreate(entity, {255,75,75,255});
		    IgniteEntity(entity, 2.0);	
        }


	}
	else if (!strcmp(classname, "flashbang_projectile"))
	{
		if (g_GrenadeSystemCFG.EnableLightGrenade)
		{
			CreateTimer(1.3, DoFlashLight, entity, TIMER_FLAG_NO_MAPCHANGE);
		}
        if(g_GrenadeSystemCFG.EnableLightGrenadetrails)
        {
		    BeamFollowCreate(entity, {255,255,255,255});
        }

	}
	else if (!strcmp(classname, "smokegrenade_projectile"))
	{
		if (g_GrenadeSystemCFG.EnableIceGrenade)
		{
			CreateTimer(1.3, CreateEvent_SmokeDetonate, entity, TIMER_FLAG_NO_MAPCHANGE);
		}
		if(g_GrenadeSystemCFG.EnableIceGrenadetrails)
        {
		    BeamFollowCreate(entity, {75,75,255,255});
        }
	}
	else if (g_GrenadeSystemCFG.EnableIceGrenade && !strcmp(classname, "env_particlesmokegrenade"))
	{
		AcceptEntityInput(entity, "Kill");
	}
}

//========================================================================================================================================================
//API
//========================================================================================================================================================


public int Native_HanSetCustomAnimation(Handle plugin, int numParams) 
{
    int zombie = GetNativeCell(1); 

    float now = GetEngineTime();

    if (g_ZombieState[zombie].CustomAnimEndTime > now)
    {
        return 0;
    }
    
    char anim[64];
    GetNativeString(2, anim, sizeof(anim)); 
    
    float duration = view_as<float>(GetNativeCell(3));
    
    Han_SetAnimation(zombie, anim, duration);
    g_ZombieState[zombie].IsCustomAnim = true;
    
    g_ZombieState[zombie].CustomAnimEndTime = now + duration;
    
    return 1; 
}


public any Native_HanIsZombie(Handle plugin, int numParams)
{
    int entity = GetNativeCell(1);
    for (int i = 0; i < g_SpawnZombiedCount; i++)
    {
        if (g_SpawnZombied[i] == entity)
            return true;
    }
    return false;
}


public any Native_HanGetZombieCount(Handle plugin, int numParams)
{
    return g_SpawnZombiedCount;
}

public any Native_HanGetZombieByIndex(Handle plugin, int numParams)
{
    int index = GetNativeCell(1);
    if (index < 0 || index >= g_SpawnZombiedCount)
        return -1;
    return g_SpawnZombied[index];
}

public int Native_HanGetZombieName(Handle plugin, int numParams) 
{
    int zombie = GetNativeCell(1);
    int maxLen = GetNativeCell(3);
    
    char key[12];
    IntToString(zombie, key, sizeof(key));
    
    char name[64];
    if(g_ZombieNameMap.GetString(key, name, sizeof(name))) {
        SetNativeString(2, name, maxLen);
        return 1;
    }
    return 0;
}

public int Native_SafeDamageZombie(Handle plugin, int numParams) 
{
    int attacker = GetNativeCell(1);
    int zombie = GetNativeCell(2);
    int damage = GetNativeCell(3);
    damage = damage < 1 ? 1 : (damage > 100000 ? 100000 : damage);
    
    return Han_DealZombieDamage(attacker, zombie, damage, DMG_NEVERGIB, "external");
}


public int Native_HanSetZombieTarget(Handle plugin, int numParams) 
{
    int zombie = GetNativeCell(1);
    int target = GetNativeCell(2);
    float locktime = view_as<float>(GetNativeCell(3));

    if (!IsValidEntity(zombie) || !Han_ZombieIsAlive(zombie) || !IsValidClient(target) || !IsPlayerAlive(target))
    {
        return 0;
    }

    g_iForcedTarget[zombie] = target;

    if (locktime <= 0.0)
    {
        g_fTargetLockExpire[zombie] = -1.0; // 永久锁定
    }
    else
    {
        g_fTargetLockExpire[zombie] = GetGameTime() + locktime;
    }

    return 1;
}


// 解除目标锁定
public int Native_HanUnlockZombie(Handle plugin, int numParams) 
{
    int zombie = GetNativeCell(1);

    if (!IsValidEntity(zombie))
    {
        return 0;
    }

    g_fTargetLockExpire[zombie] = 0.0; // 解锁
    g_iForcedTarget[zombie] = 0;

    return 1;
}




