
void InitScenarioCvars()
{
    g_ScenarioConfig.enableplugins = CreateConVar("hzs_enableplugins", "1", "是否启用插件");
    g_ScenarioConfig.maxZombieSpawn = CreateConVar("hzs_maxzombiespawn", "20", "最大生成僵尸数量");
    g_ScenarioConfig.allowBuy = CreateConVar("hzs_allowbuy", "1", "是否允许购买");

    g_ScenarioConfig.skybox = CreateConVar("hzs_skybox", "sky_borealis01", "地图天空盒名称");
    g_ScenarioConfig.backgroundMusic = CreateConVar("hzs_music", "ambient/zr/zr_ambience.wav:60.0", "地图背景音乐路径");
    g_ScenarioConfig.lightStyle = CreateConVar("hzs_lightstyle", "n", "地图亮度设置");

    g_ScenarioConfig.playerrebornsec = CreateConVar("hzs_playerreborn", "10.0", "玩家复活时间（秒）");
    g_ScenarioConfig.playerspawnprotect = CreateConVar("hzs_spawnprotect", "10.0", "玩家出生保护时间");
    g_ScenarioConfig.ZombieSpawnTime = CreateConVar("hzs_zombiespawntime", "20.0", "回合开始丧尸出现延迟时间");

    g_ScenarioConfig.killmoney = CreateConVar("hzs_killmoney", "300", "击杀僵尸金钱");
    g_ScenarioConfig.Maxmoney = CreateConVar("hzs_maxmoney", "16000", "最大金钱上限");

    g_ScenarioConfig.ShowPrintDamage = CreateConVar("hzs_showdamage", "1", "是否显示伤害输出");
    g_ScenarioConfig.WinEndRound = CreateConVar("hzs_winendround", "1", "胜利是否结束回合");

    g_ScenarioConfig.EnableKillIcon = CreateConVar("hzs_enablekillicon", "1", "是否开启右上角击杀图标");

    HookConVarChange(g_ScenarioConfig.enableplugins, EnableHook);
}


public EnableHook(Handle convar, const char[] oldValue, const char[] newValue)
{
    bool enable = StringToInt(newValue) != 0;

    if (enable)
    {
        PrintToServer("[HanZombieScenario] 插件启用，开始初始化...");

        g_CurrentStage = 0;
        HookEvents();
        HookUserMgs();
        offsCollision = FindSendPropInfo("CBaseEntity", "m_CollisionGroup");
        gLeaderOffset = FindSendPropInfo("CHostage", "m_leader");

        AddCommandListener(Cmd_BlockAddBot, "bot_add");
        AddCommandListener(Cmd_BlockAddBot, "bot_add_ct");
        AddCommandListener(Cmd_BlockAddBot, "bot_add_t");

        CreateAdmianCommand();

        TerminateRound(3.0, CSRoundEnd_GameStart);

    }
    else
    {
        PrintToServer("[HanZombieScenario] 插件禁用，正在清理...");
        ScenarioEnd();  
    }

}


ScenarioEnd()
{
    TerminateRound(3.0, CSRoundEnd_GameStart);
    UnHookEvents(); 
    ClearPlayerReSpawnTimer();
    RemoveCommandListener(Cmd_BlockAddBot, "bot_add");
    RemoveCommandListener(Cmd_BlockAddBot, "bot_add_ct");
    RemoveCommandListener(Cmd_BlockAddBot, "bot_add_t");
    ServerCommand("mp_ignore_round_win_conditions 0");
}


