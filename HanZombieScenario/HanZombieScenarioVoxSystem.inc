void LoadScenarioVoxConfig()
{
    KeyValues kv = new KeyValues("ScenarioVoxConfig");

    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "configs/HanZombieScenario/HanZombieScenarioVoxConfig.cfg");

    if (!FileExists(path))
    {
        WriteDefaultScenarioVoxConfig(path);
    }

    if (!FileToKeyValues(kv, path))
    {
        PrintToServer("[ScenarioVoxConfig] 配置读取失败！");
        delete kv;
        return;
    }

    g_VoxSystemConfig.EnableVox = KvGetNum(kv, "enable_vox", 1) != 0;
    KvGetString(kv, "TwentysVox_Path", g_VoxSystemConfig.TwentysVox, sizeof(g_VoxSystemConfig.TwentysVox));
    KvGetString(kv, "RoundStartVox_Path", g_VoxSystemConfig.RoundStartVox, sizeof(g_VoxSystemConfig.RoundStartVox));
    KvGetString(kv, "CountDownVox_Path", g_VoxSystemConfig.CountDownVox, sizeof(g_VoxSystemConfig.CountDownVox));
    KvGetString(kv, "ZombieComeingVox_Path", g_VoxSystemConfig.ZombieComeingVox, sizeof(g_VoxSystemConfig.ZombieComeingVox));
    KvGetString(kv, "HumanWinsVox_Path", g_VoxSystemConfig.HumanWinsVox, sizeof(g_VoxSystemConfig.HumanWinsVox));
    KvGetString(kv, "ZombieWinsVox_Path", g_VoxSystemConfig.ZombieWinsVox, sizeof(g_VoxSystemConfig.ZombieWinsVox));
    KvGetString(kv, "WinsBgmVox_Path", g_VoxSystemConfig.WinsBgmVox, sizeof(g_VoxSystemConfig.WinsBgmVox));
    KvGetString(kv, "lostBgmVox_Path", g_VoxSystemConfig.lostBgmVox, sizeof(g_VoxSystemConfig.lostBgmVox));

    PrecacheSoundList(g_VoxSystemConfig.TwentysVox);
    PrecacheSoundList(g_VoxSystemConfig.RoundStartVox);
    PrecacheSoundList(g_VoxSystemConfig.CountDownVox);
    PrecacheSoundList(g_VoxSystemConfig.ZombieComeingVox);
    PrecacheSoundList(g_VoxSystemConfig.HumanWinsVox);
    PrecacheSoundList(g_VoxSystemConfig.ZombieWinsVox);
    PrecacheSoundList(g_VoxSystemConfig.WinsBgmVox);
    PrecacheSoundList(g_VoxSystemConfig.lostBgmVox);

    delete kv;

}

void WriteDefaultScenarioVoxConfig(const char[] path)
{
    Handle file = OpenFile(path, "w");
    if (file == INVALID_HANDLE)
        return;

    WriteFileLine(file, "// ScenarioVoxConfig 音效播报系统配置文件 填写音效路径");
    WriteFileLine(file, "//  多个音效用 , 隔开");
    WriteFileLine(file, "//  CountDownVox_Path 必须按顺序 1- n 填写 用 ,隔开 填写任意数量 倒计时会自动从最后一个倒数至第一个");
    WriteFileLine(file, "// enable_vox 是否开启音效系统 1 为开启 0 为关闭");
    WriteFileLine(file, "// RoundStartVox_Path 填写音乐路径用于回合开始播放音乐");
    WriteFileLine(file, "// TwentysVox_Path 填写20秒提示");
    WriteFileLine(file, "// CountDownVox_Path 填写倒计时音效");
    WriteFileLine(file, "// ZombieComeingVox_Path 填写游戏开始音效");
    WriteFileLine(file, "// HumanWinsVox_Path 填写人类胜利音效");
    WriteFileLine(file, "// ZombieWinsVox_Path 填写丧尸胜利音效");
    WriteFileLine(file, "// WinsBgmVox_Path 填写人类胜利BGM");
    WriteFileLine(file, "// lostBgmVox_Path 填写人类失败BGM");
    WriteFileLine(file, "\"ScenarioVoxConfig\"");
    WriteFileLine(file, "{");
    WriteFileLine(file, "    \"enable_vox\"    \"1\"");
    WriteFileLine(file, "    \"TwentysVox_Path\"           \"music/zombie_scenario_TwentysVox.mp3\"");
    WriteFileLine(file, "    \"RoundStartVox_Path\"           \"music/zombie_scenario_start.mp3\"");
    WriteFileLine(file, "    \"CountDownVox_Path\"              \"music/zombie_scenario_sCountDown.mp3\"");
    WriteFileLine(file, "    \"ZombieComeingVox_Path\"          \"music/zombie_scenario_ZombieComeing.mp3\"");
    WriteFileLine(file, "    \"HumanWinsVox_Path\"    \"music/zombie_scenario_HumanWins.mp3\"");
    WriteFileLine(file, "    \"ZombieWinsVox_Path\"    \"music/zombie_scenario_ZombieWins.mp3\"");
    WriteFileLine(file, "    \"WinsBgmVox_Path\"    \"music/zombie_scenario_WinsBgmVox.mp3\"");
    WriteFileLine(file, "    \"lostBgmVox_Path\"    \"music/zombie_scenario_lostBgmVox.mp3\"");
    WriteFileLine(file, "}");

    CloseHandle(file);
}

// ============================================================================
// >> helper
// ============================================================================

void PrecacheSoundList(const char[] input)
{
    if (input[0] == '\0')
        return;

    char paths[128][PLATFORM_MAX_PATH];
    int count = ExplodeString(input, ",", paths, sizeof(paths), sizeof(paths[]));

    for (int i = 0; i < count; i++)
    {
        TrimString(paths[i]);
        PrecacheSoundSafe(paths[i]);
    }
}



void StartCountDownVox(int zombieDelaySeconds)
{
    if (g_VoxSystemConfig.CountDownVox[0] == '\0' || zombieDelaySeconds <= 0)
        return;

    char sounds[32][PLATFORM_MAX_PATH];
    int total = ExplodeString(g_VoxSystemConfig.CountDownVox, ",", sounds, sizeof(sounds), sizeof(sounds[]));
    if (total <= 0)
        return;

    for (int i = 0; i < total; i++)
        TrimString(sounds[i]);

    // 如果音效比秒数多，跳过前面的
    int startIndex = total > zombieDelaySeconds ? total - zombieDelaySeconds : 0;
    int playCount = total - startIndex;

    for (int i = 0; i < playCount; i++)
    {
        int countdownSecond = playCount - i; 
        int playAt = zombieDelaySeconds - countdownSecond - 1;

        if (playAt >= 0)
        {
            DataPack dp = new DataPack();
            dp.WriteString(sounds[startIndex + i]);
            CreateTimer(float(playAt), Timer_PlayCountdownVox, dp, TIMER_FLAG_NO_MAPCHANGE);
        }
    }

    if (zombieDelaySeconds <= 0)
    {
        return;
    }

    float hudAdvanceSeconds = 1.0;

    int hintStartSecond = zombieDelaySeconds;
    if (hintStartSecond > 10)
    {
        hintStartSecond = 10;
    }

    for (int secondsLeft = hintStartSecond; secondsLeft >= 1; secondsLeft--)
    {
        float delay = float(zombieDelaySeconds - secondsLeft) - hudAdvanceSeconds;
         if (delay < 0.0)
        {
            delay = 0.0;
        }
        CreateTimer(delay, Timer_ShowCountdownHint, secondsLeft, TIMER_FLAG_NO_MAPCHANGE);
    }
}

public Action Timer_PlayCountdownVox(Handle timer, DataPack dp)
{
    ResetPack(dp);
    char sound[PLATFORM_MAX_PATH];
    dp.ReadString(sound, sizeof(sound));
    delete dp;

    for (int client = 1; client <= MaxClients; client++)
    {
        if (IsClientInGame(client))
        {
            bool isenable = g_VoxSystemConfig.EnableVox;
            if(!isenable)
                return Plugin_Stop;

            EmitSoundToClient(client, sound, client, SNDCHAN_AUTO, SNDLEVEL_NORMAL, SND_NOFLAGS, 0.5, SNDPITCH_NORMAL);
        }
    }
    return Plugin_Stop;
}


void PlayRoundVox(const char[] input)
{
    bool isenable = g_VoxSystemConfig.EnableVox;
    if(!isenable)
        return;

    if (input[0] == '\0')
        return;

    // 用于存储所有分割出的子字符串
    ArrayList soundList = new ArrayList(PLATFORM_MAX_PATH);

    // 临时变量，用于存储当前正在处理的子字符串
    char sound[PLATFORM_MAX_PATH];
    int soundIndex = 0;

    // 遍历输入字符串
    int length = strlen(input);
    for (int i = 0; i <= length; i++)
    {
        if (input[i] == ',' || input[i] == '\0')
        {
            // 如果遇到逗号或字符串结尾，将当前子字符串添加到列表中
            sound[soundIndex] = '\0'; // 确保字符串以 null 结尾
            soundList.PushString(sound);

            // 重置当前子字符串的索引
            soundIndex = 0;
        }
        else
        {
            // 将当前字符添加到当前子字符串中
            sound[soundIndex++] = input[i];
        }
    }

    // 检查是否有分割出的子字符串
    int count = soundList.Length;
    if (count <= 0)
    {
        // 如果没有分割出任何子字符串，直接返回
        delete soundList;
        return;
    }

    // 随机选择一个索引
    int index = GetRandomInt(0, count - 1);

    // 获取随机选择的声音路径
    char selectedSound[PLATFORM_MAX_PATH];
    soundList.GetString(index, selectedSound, sizeof(selectedSound));

    // 播放声音给所有在线玩家
    for (int client = 1; client <= MaxClients; client++)
    {
        if (IsClientInGame(client))
            EmitSoundToClient(client, selectedSound, client, SNDCHAN_AUTO, SNDLEVEL_NORMAL, SND_NOFLAGS, 0.5, SNDPITCH_NORMAL);
    }

    // 释放 ArrayList 的内存
    delete soundList;
}

public Action Timer_ShowCountdownHint(Handle timer, int remainingSeconds)
{
    PrintCenterTextAll("%d 秒后丧尸开始行动!!", remainingSeconds);
    return Plugin_Stop;
}
