void LoadScenarioConfig()
{
    KeyValues kv = new KeyValues("ScenarioConfig");

    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "configs/HanZombieScenario/HanZombieScenarioConfig.cfg");

    if (!FileExists(path))
    {
        WriteDefaultScenarioConfig(path);
    }

    if (!FileToKeyValues(kv, path))
    {
        PrintToServer("[ZombieScenario] 配置读取失败！");
        delete kv;
        return;
    }
    g_ScenarioConfig.enableplugins = KvGetNum(kv, "enable_plugins", 1) != 0;
    g_ScenarioConfig.maxZombieSpawn = KvGetNum(kv, "max_zombie_spawn", 30);
    g_ScenarioConfig.allowBuy = KvGetNum(kv, "allow_buy", 1) != 0;
    KvGetString(kv, "skybox", g_ScenarioConfig.skybox, sizeof(g_ScenarioConfig.skybox));
    KvGetString(kv, "background_music", g_ScenarioConfig.backgroundMusic, sizeof(g_ScenarioConfig.backgroundMusic));

    g_ScenarioConfig.playerrebornsec = KvGetFloat(kv, "player_reborn_sec", 10.0);
    g_ScenarioConfig.playerspawnprotect = KvGetFloat(kv, "player_spawn_protect", 10.0);
    g_ScenarioConfig.ZombieSpawnTime = KvGetFloat(kv, "Zombie_SpawnTime", 20.0);
    g_ScenarioConfig.killmoney = KvGetNum(kv, "Kill_Money", 300);
    g_ScenarioConfig.Maxmoney = KvGetNum(kv, "Max_Money", 16000);
    g_ScenarioConfig.ShowPrintDamage = KvGetNum(kv, "Show_Print_Damage", 1) != 0;
    
    char light[4];
    KvGetString(kv, "lightstyle", light, sizeof(light));

    if (light[0] != '\0')
    {
        strcopy(g_ScenarioConfig.lightStyle, sizeof(g_ScenarioConfig.lightStyle), light);
    }
    else
    {
        strcopy(g_ScenarioConfig.lightStyle, sizeof(g_ScenarioConfig.lightStyle), "n");
    }

    PrecacheBackgroundMusic();

    delete kv;
    /*
        PrintToServer("[ZombieScenario] 全局配置加载完毕。是否开启插件 %d, 最大丧尸: %d, 允许购买: %d, 天空盒: %s, 背景音乐: %s, 亮度: %c, 复活时间%1f, 保护时间 %1.f,丧尸生成延迟 : %1.f,击杀给钱 : %d, 最大金钱上限:%d",
            g_ScenarioConfig.enableplugins,
            g_ScenarioConfig.maxZombieSpawn,
            g_ScenarioConfig.allowBuy,
            g_ScenarioConfig.skybox,
            g_ScenarioConfig.backgroundMusic,
            g_ScenarioConfig.lightStyle,
            g_ScenarioConfig.playerrebornsec,
            g_ScenarioConfig.playerspawnprotect,
            g_ScenarioConfig.ZombieSpawnTime,
            g_ScenarioConfig.killmoney,
            g_ScenarioConfig.Maxmoney
        );
    */
}

void PrecacheBackgroundMusic()
{
    if (g_ScenarioConfig.backgroundMusic[0] == '\0')
        return;

    char fullStr[PLATFORM_MAX_PATH];
    strcopy(fullStr, sizeof(fullStr), g_ScenarioConfig.backgroundMusic);

    char musicPath[PLATFORM_MAX_PATH];
    int sep = FindCharInString(fullStr, ':');

    if (sep != -1)
    {
        strcopy(musicPath, sizeof(musicPath), fullStr);
        musicPath[sep] = '\0';
    }
    else
    {
        strcopy(musicPath, sizeof(musicPath), fullStr);
    }

    PrecacheSound(musicPath, true);
    //PrintToServer("[预缓存] 背景音乐: %s", musicPath); // Debug 用
}



void WriteDefaultScenarioConfig(const char[] path)
{
    Handle file = OpenFile(path, "w");
    if (file == INVALID_HANDLE)
        return;

    WriteFileLine(file, "// HanZombieScenario 全局配置文件");
    WriteFileLine(file, "// enable_plugins 是否开启此插件");
    WriteFileLine(file, "// max_zombie_spawn 最大生成丧尸实体限制 实体若超过2048 会崩溃");
    WriteFileLine(file, "// allow_buy_anywhere 允许任何地图任何地点可以打开购买菜单");
    WriteFileLine(file, "// skybox 更改天空盒");
    WriteFileLine(file, "// lightstyle 更改地图亮度 a为最黑  b普通  n最亮");
    WriteFileLine(file, "// background_music 地图播放自定义背景音乐 默认时长60秒 路径后使用:可以自定义循环bgm时长");
    WriteFileLine(file, "// player_reborn_sec 玩家死亡复活时间 /秒");
    WriteFileLine(file, "// player_spawn_protect 复活后的保护时间 /秒");
    WriteFileLine(file, "// Zombie_SpawnTime 回合开始后多长时间开始生成丧尸 /秒 0.0 为 回合开始立即生成");
    WriteFileLine(file, "// Kill_Money 击杀给多少金钱");
    WriteFileLine(file, "// Max_Money 最大能获取的金钱,一次击杀最大金钱");
    WriteFileLine(file, "// Show_Print_Damage 开启显示自己造成的伤害");
    WriteFileLine(file, "\"ScenarioConfig\"");
    WriteFileLine(file, "{");
    WriteFileLine(file, "    \"enable_plugins\"    \"1\"");
    WriteFileLine(file, "    \"max_zombie_spawn\"    \"20\"");
    WriteFileLine(file, "    \"allow_buy_anywhere\"           \"1\"");
    WriteFileLine(file, "    \"skybox\"              \"sky_borealis01\"");
    WriteFileLine(file, "    \"lightstyle\"          \"n\"");
    WriteFileLine(file, "    \"background_music\"    \"ambient/zr/zr_ambience.wav:60.0\"");
    WriteFileLine(file, "    \"player_reborn_sec\"    \"10.0\"");
    WriteFileLine(file, "    \"player_spawn_protect\"    \"10.0\"");
    WriteFileLine(file, "    \"Zombie_SpawnTime\"    \"20.0\"");
    WriteFileLine(file, "    \"Kill_Money\"    \"300\"");
    WriteFileLine(file, "    \"Max_Money\"    \"16000\"");
    WriteFileLine(file, "    \"Show_Print_Damage\"    \"1\"");
    WriteFileLine(file, "}");

    CloseHandle(file);
}

