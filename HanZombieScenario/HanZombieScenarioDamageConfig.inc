void LoadScenarioDamageConfig()
{
    if (g_DamageConfig != null)
    {
        delete g_DamageConfig;
    }
    g_DamageConfig = new StringMap();

    KeyValues kv = new KeyValues("ScenarioDmgConfig");

    char path[PLATFORM_MAX_PATH];
    BuildPath(Path_SM, path, sizeof(path), "configs/HanZombieScenario/HanZombieScenarioDamageConfig.cfg");

    if (!FileExists(path))
    {
        WriteDefaultScenarioDmgConfig(path);
    }

    if (!FileToKeyValues(kv, path))
    {
        PrintToServer("[DamageConfig] 配置读取失败！");
        delete kv;
        return;
    }

    if (!KvGotoFirstSubKey(kv, false))
    {
        PrintToServer("[DamageConfig] 配置为空");
        delete kv;
        return;
    }

    do
    {
        char weapon[64], value[32];
        KvGetSectionName(kv, weapon, sizeof(weapon));
        KvGetString(kv, NULL_STRING, value, sizeof(value), "");

        if (weapon[0] != '\0' && value[0] != '\0')
        {
            g_DamageConfig.SetString(weapon, value);
            PrintToServer("[DamageConfig] 武器伤害获取: %s -> %s", weapon, value);
        }
    }
    while (KvGotoNextKey(kv, false));

    delete kv;
}

void WriteDefaultScenarioDmgConfig(const char[] path)
{
    Handle file = OpenFile(path, "w");
    if (file == INVALID_HANDLE)
        return;

    WriteFileLine(file, "// =====================================================");
    WriteFileLine(file, "// HanZombieScenario 伤害调整配置文件");
    WriteFileLine(file, "// 格式: \"武器classname\"    \"伤害值\"");
    WriteFileLine(file, "// 支持运算符:");
    WriteFileLine(file, "//   +N   -> 在原伤害上增加 N");
    WriteFileLine(file, "//   -N   -> 在原伤害上减少 N");
    WriteFileLine(file, "//   *N   -> 在原伤害上乘以 N");
    WriteFileLine(file, "//   /N   -> 在原伤害上除以 N");
    WriteFileLine(file, "// 例如: \"weapon_ak47\" \"+40.0\"  表示 AK47 额外增加40伤害");
    WriteFileLine(file, "// =====================================================");
    WriteFileLine(file, "");
    WriteFileLine(file, "\"ScenarioDmgConfig\"");
    WriteFileLine(file, "{");
    WriteFileLine(file, "    \"weapon_ak47\"    \"+40.0\"");
    WriteFileLine(file, "    \"weapon_m4a1\"    \"-10.0\"");
    WriteFileLine(file, "}");
    
    CloseHandle(file);
}


float ApplyScenarioDamageModifier(const char[] classname, float damage)
{
    if (g_DamageConfig == null)
        return damage;

    char rule[32];
    if (!g_DamageConfig.GetString(classname, rule, sizeof(rule)))
        return damage;

    char op = rule[0];
    float val = StringToFloat(rule[1]); // 从第二个字符开始解析数值

    switch (op)
    {
        case '+': damage += val;
        case '-': damage -= val;
        case '*': damage *= val;
        case '/':
        {
            if (val != 0.0) damage /= val;
        }
    }

    //PrintToServer("[Zombie] %s 应用伤害规则 %s => %.1f", classname, rule, damage);
    return damage;
}
