

void LoadZombieStages()
{
    KeyValues kv = new KeyValues("Stage");

    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "configs/HanZombieScenario/HanZombieScenarioStage.cfg");
    if (!FileExists(path))
    {
        WriteDefaultZombieStageConfig(path);
    }
    if (!FileToKeyValues(kv, path))
    {
        PrintToServer("[ZombieScenario] 关卡配置文件读取失败！");
        delete kv;
        return;
    }

    KvGotoFirstSubKey(kv);
    int index = 0;

    do 
    {
        if (index >= sizeof(g_Stages)) break;

        ZombieStage stage;
        
        KvGetSectionName(kv, stage.name, sizeof(stage.name));
        stage.killRequired = KvGetNum(kv, "count", 0);
        stage.healthBoost = KvGetNum(kv, "healthboost", 0);
        KvGetString(kv, "zombie", stage.zombies, sizeof(stage.zombies));
        stage.maxszombiecount = KvGetNum(kv, "maxszombiecount", 0);
        KvGetString(kv, "specialzombie", stage.specialzombies, sizeof(stage.specialzombies));
        KvGetString(kv, "storyline", stage.storyline, sizeof(stage.storyline));

        g_Stages[index++] = stage;
    } while (KvGotoNextKey(kv));

    g_StageCount = index;
    delete kv;

    PrintToServer("[ZombieScenario] 已加载 %d 个关卡阶段。", g_StageCount);
}


void WriteDefaultZombieStageConfig(const char[] path)
{
    Handle file = OpenFile(path, "w");
    if (file == INVALID_HANDLE)
        return;

    WriteFileLine(file, "// HanZombieScenario 关卡配置文件");
    WriteFileLine(file, "// 每个 Stage 表示一个关卡，字段说明如下：");
    WriteFileLine(file, "// count:            本关卡所需击杀丧尸数量");
    WriteFileLine(file, "// healthboost:      该关卡给予所有丧尸的额外血量加成");
    WriteFileLine(file, "// zombie:           本关卡可生成的丧尸类型名称（多个用英文逗号分隔，对应 ZombieConfig 中的配置项）");
    WriteFileLine(file, "// maxszombiecount:  本关特殊丧尸额外生成数量");
    WriteFileLine(file, "// specialzombie:    本关卡可生成的特殊丧尸丧尸类型名称（多个用英文逗号分隔，对应 ZombieConfig 中的配置项）");
    WriteFileLine(file, "// storyline:        本关卡的剧情提示，可用于显示在 HUD 或聊天中");
    WriteFileLine(file, "");
    WriteFileLine(file, "\"Stage\"");
    WriteFileLine(file, "{");
    WriteFileLine(file, "    \"Stage1\"");
    WriteFileLine(file, "    {");
    WriteFileLine(file, "        \"count\"         \"50\"");
    WriteFileLine(file, "        \"healthboost\"   \"100\"");
    WriteFileLine(file, "        \"zombie\"        \"putong_stage1\"");
    WriteFileLine(file, "        \"maxszombiecount\"        \"0\"");
    WriteFileLine(file, "        \"specialzombie\"        \"putong_stage1\"");
    WriteFileLine(file, "        \"storyline\"     \"不明病毒爆发，灭杀小队请立即出动！\"");
    WriteFileLine(file, "    }");
    WriteFileLine(file, "");
    WriteFileLine(file, "    \"Stage2\"");
    WriteFileLine(file, "    {");
    WriteFileLine(file, "        \"count\"         \"80\"");
    WriteFileLine(file, "        \"healthboost\"   \"200\"");
    WriteFileLine(file, "        \"zombie\"        \"putong_stage1,zombie_lite\"");
    WriteFileLine(file, "        \"maxszombiecount\"        \"0\"");
    WriteFileLine(file, "        \"specialzombie\"        \"putong_stage1\"");
    WriteFileLine(file, "        \"storyline\"     \"敌人变异升级，小队成员请注意！\"");
    WriteFileLine(file, "    }");
    WriteFileLine(file, "}");

    CloseHandle(file);
}

