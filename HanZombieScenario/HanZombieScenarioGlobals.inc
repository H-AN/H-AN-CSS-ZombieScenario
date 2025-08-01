#define VERSION "2.0"
#define MAXSPAWNS 256
bool g_bZombieConfigReady = false;
int offsCollision; 
int gLeaderOffset;

enum struct ZombieType
{
    char name[64];
    char model[PLATFORM_MAX_PATH];
    float friction;
    float attackRange;
    int health;
    int damage;
    float hitboxsize;
    float animesize;

    ArrayList idleAnims;
    ArrayList moveAnims;
    ArrayList attackAnims;
    ArrayList getattackAnims;
    ArrayList deathAnims;

    ArrayList hitsound;
    ArrayList hurtsound;
    ArrayList deathsound;

    int r;
    int g;
    int b;
    int a;
}

// 全局容器
ArrayList g_ZombieList;
StringMap g_ZombieIndexMap;


enum struct ZombieState {
    bool IsAttacking;
    bool IsBeingAttacked;
    float NextAnimTime;
    bool IsBeFreeze;
    int Target;

    // 外部动画控制
    bool IsCustomAnim;
    float CustomAnimEndTime;    // 外部动画结束时间
}
ZombieState g_ZombieState[2048];

enum struct ZombieStage
{
    char name[64];
    int killRequired;
    int healthBoost;
    char zombies[128];   // 可选僵尸名称，逗号分隔
    int maxszombiecount;   // 此关最大特殊丧尸生成数量
    char specialzombies[128];   // 可选特殊僵尸名称，逗号分隔
    char storyline[256]; // 剧情描述
}

ZombieStage g_Stages[32]; // 最多支持32个阶段
int g_StageCount = 0;
int g_CurrentStage = 0;

int g_ZombiesKilled;

enum struct ScenarioConfig
{
    bool enableplugins;
    int maxZombieSpawn;
    bool allowBuy;
    char skybox[64];
    char backgroundMusic[PLATFORM_MAX_PATH];
    char lightStyle[4];
    float playerrebornsec; 
    float playerspawnprotect; 
    float ZombieSpawnTime;
    int killmoney;
    int Maxmoney;
    bool ShowPrintDamage;
}

ScenarioConfig g_ScenarioConfig;

bool Isenable = true;

enum struct VoxSystemConfig
{
    bool EnableVox;
    char TwentysVox[PLATFORM_MAX_PATH];
    char RoundStartVox[PLATFORM_MAX_PATH];
    char CountDownVox[PLATFORM_MAX_PATH];
    char ZombieComeingVox[PLATFORM_MAX_PATH];
    char HumanWinsVox[PLATFORM_MAX_PATH];
    char ZombieWinsVox[PLATFORM_MAX_PATH];
    char WinsBgmVox[PLATFORM_MAX_PATH];
    char lostBgmVox[PLATFORM_MAX_PATH];
}

VoxSystemConfig g_VoxSystemConfig;

enum struct GrenadeSystemCFG
{
    bool EnableGrenadeSys;

    bool EnableFireGrenade;
    bool EnableFireGrenadetrails;
    float FireGrenadeduration;
    int FireGrenadeBurnDamage;

    bool EnableLightGrenade;
    bool EnableLightGrenadetrails;
    float LightGrenadeduration;
    float LightGrenadeRadius;
    
    bool EnableIceGrenade;
    bool EnableIceGrenadetrails;
    float IceGrenadeduration;
    float IceGrenadeRadius;

}

GrenadeSystemCFG g_GrenadeSystemCFG;

#define FLASH 0
#define SMOKE 1

bool g_ZombieBurning[2048];
Handle freeze_timer[2048];


StringMap g_ZombieNameMap;




Handle tHUD = INVALID_HANDLE;
Handle tAmbience = INVALID_HANDLE;
Handle FreezeEndCreateZ = INVALID_HANDLE;
Handle FreezeEndTwentysVox = INVALID_HANDLE;
Handle ReSpawn[MAXPLAYERS+1];
int RespawnTime[MAXPLAYERS+1];
bool IsCountDown = false;

char g_ZombieName[2048][64];

int g_SpawnZombied[512];
int g_SpawnZombiedCount = 0;

bool GameStart;

int iAccount[MAXPLAYERS+1];

bool IsHurtZombie[MAXPLAYERS+1];
float g_fLastAttackTime[MAXPLAYERS+1];
int g_iTargetZombie[MAXPLAYERS+1];

GlobalForward g_ForwardZombieCreated;  // 丧尸创建
GlobalForward g_ForwardZombieDeath;    // 丧尸死亡
GlobalForward g_ForwardZombieHurt;     // 丧尸被攻击
GlobalForward g_ForwardZombieAttack;   // 丧尸攻击玩家

MapChangeCleanup() 
{
    IsCountDown = false;
    g_StageCount = 0;
    g_CurrentStage = 0;
    g_ZombiesKilled = 0;
    tHUD = INVALID_HANDLE; 
    tAmbience = INVALID_HANDLE; 
    FreezeEndCreateZ = INVALID_HANDLE;
    FreezeEndTwentysVox = INVALID_HANDLE;
}