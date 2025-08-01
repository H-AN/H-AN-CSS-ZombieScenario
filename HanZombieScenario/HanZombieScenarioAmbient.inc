
void ChangeMapLightAndSkyBox()
{
    SetLightStyle(0, g_ScenarioConfig.lightStyle);
    SetConVarString(FindConVar("sv_skyname"), g_ScenarioConfig.skybox, true);
}

public Action AmbienceLoop(Handle timer)
{
	StopAmbience();
    CreateTimer(0.2, playsound, _, TIMER_FLAG_NO_MAPCHANGE);  

    return Plugin_Continue; 
}

public Action playsound(Handle timer)
{
    if (g_ScenarioConfig.backgroundMusic[0] == '\0')
        return Plugin_Stop;

    char fullStr[PLATFORM_MAX_PATH];
    strcopy(fullStr, sizeof(fullStr), g_ScenarioConfig.backgroundMusic);

    char soundPath[PLATFORM_MAX_PATH];
    float duration = 60.0;

    int sep = FindCharInString(fullStr, ':');
    if (sep != -1)
    {
        // 分离路径与时间
        strcopy(soundPath, sizeof(soundPath), fullStr); 
        soundPath[sep] = '\0'; 

        char timePart[16];
        strcopy(timePart, sizeof(timePart), fullStr[sep + 1]);
        duration = StringToFloat(timePart);
    }
    else
    {
        strcopy(soundPath, sizeof(soundPath), fullStr); 

    }
    // 播放音效
    EmitSoundToAll(soundPath, SOUND_FROM_WORLD, SNDCHAN_AUTO, SNDLEVEL_NORMAL, SND_NOFLAGS, 1.0, SNDPITCH_NORMAL, -1, NULL_VECTOR, NULL_VECTOR, true, 0.0);

    // 设置下一次播放
    tAmbience = CreateTimer(duration, AmbienceLoop, _, TIMER_FLAG_NO_MAPCHANGE);
    return Plugin_Continue;
}



StopAmbience()
{

    if (g_ScenarioConfig.backgroundMusic[0] == '\0')
        return Plugin_Stop;

    char fullStr[PLATFORM_MAX_PATH];
    strcopy(fullStr, sizeof(fullStr), g_ScenarioConfig.backgroundMusic);

    char soundPath[PLATFORM_MAX_PATH];
    int sep = FindCharInString(fullStr, ':');
    if (sep != -1)
    {
        // 分离路径与时间
        strcopy(soundPath, sizeof(soundPath), fullStr); 
        soundPath[sep] = '\0'; 
    }
    else
    {
        strcopy(soundPath, sizeof(soundPath), fullStr); 
    }
        
    for (int x = 1; x <= MaxClients; x++)
    {
        if (!IsClientInGame(x) || IsFakeClient(x))
        {
            continue;
        }

        StopSound(x, SNDCHAN_AUTO, soundPath);
    }
}
