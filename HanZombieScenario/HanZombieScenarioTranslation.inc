stock HZS_PrintCenterText(client, any:...)
{
    SetGlobalTransTarget(client);
    
    char phrase[192];
    
    VFormat(phrase, sizeof(phrase), "%t", 2);
    
    PrintCenterText(client, phrase);
}

stock HZS_PrintCenterTextAll(any:...)
{
    int maxplayers = MaxClients;
    for (int x = 1; x <= maxplayers; x++)
    {
        if (IsClientInGame(x) && !IsFakeClient(x))
        {
            SetGlobalTransTarget(x);
            char phrase[192];
            VFormat(phrase, sizeof(phrase), "%t", 1);
            PrintCenterText(x, phrase);
        }
    }
}

stock HZS_HudHint(client, any:...)
{
    SetGlobalTransTarget(client);
    
    char phrase[192];
    VFormat(phrase, sizeof(phrase), "%t", 2);
    
   PrintHintText(client, phrase);

}
