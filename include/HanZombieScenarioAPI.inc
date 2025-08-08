
#define HAN_ZOMBIE_SCENARIO_API_INC
/**
 * 设置丧尸播放自定义动画（带持续时间控制）
 * @param zombie    丧尸实体索引
 * @param anim      动画名称
 * @param duration  动画持续时间（秒）
 */

native bool Han_SetCustomAnimation(int zombie, const char[] anim, float duration);

/**
 * 判断一个实体是否为注册的丧尸
 * @param entity    实体索引
 * @return          是否为丧尸
 */
native bool Han_IsZombie(int entity);

/**
 * 获取所有当前注册的丧尸数量
 */
native int Han_GetZombieCount();

/**
 * 获取指定索引的丧尸实体
 * @param index     从 0 到 Han_GetZombieCount()-1
 * @return          丧尸实体索引
 */
native int Han_GetZombieByIndex(int index);

/**
 * 获取丧尸名称
 * @param zombie      实体索引
 * @param buffer      名称缓冲区
 * @param maxlength   缓冲区大小
 * @return            是否成功
 */
native bool Han_GetZombieName(int zombie, char[] buffer, int maxlength);


/**
 * 丧尸创建事件（不可拦截）
 * @param zombie     丧尸实体索引
 */
forward void Han_OnZombieCreated(int zombie);
/**
 * 丧尸死亡事件
 * @param zombie     丧尸实体
 * @param killer     击杀者玩家（-1为世界/自杀）
 */
forward void Han_OnZombieDeath(int zombie, int killer);
/**
 * 丧尸被攻击事件
 * @param zombie     丧尸实体
 * @param attacker   攻击者玩家
 */
forward void Han_OnZombieHurt(int zombie, int attacker);
/**
 * 丧尸攻击事件
 * @param zombie    丧尸实体索引
 * @param victim      玩家客户端索引
 */
forward void Han_OnZombieAttack(int zombie, int victim);

/**
 * 安全伤害API（强制使用DMG_NEVERGIB）
 * @param attacker  伤害来源（玩家/实体，0=世界）
 * @param zombie    目标丧尸实体
 * @param damage    伤害值
 * @return         是否成功造成伤害
 */
native bool Han_SafeDamageZombie(int attacker, int zombie, int damage);

/**
 * 
 * @param zombie  目标丧尸实体 锁定会因为 丧尸死亡 或者锁定目标死亡 而解除锁定
 * @param target  玩家客户端索引 
 * @param locktime    锁定时长 -1.0 和 0.0 代表永久锁定 填写1.0将锁定 目标1.0秒
 * @return         是否成功
 */
native bool Han_SetZombieTarget(int zombie, int target, float locktime);
/**
 * 
 * @param zombie  目标丧尸实体 解除锁定目标状态
 * @return         是否成功
 */
native bool Han_UnlockZombie(int zombie);

/**
 * 游戏开始事件（关卡加载完毕，倒计时结束，第一次生成丧尸 游戏正式开始 ）
 */
forward void Han_OnGameStart();

/**
 * 游戏结束事件（丧尸被清除,游戏结束 Gamestart = false）
 */
forward void Han_OnGameEnd();

/**
 * 玩家胜利事件（人类通关胜利事件）
 */
forward void Han_OnHumanWin();

/**
 * 玩家失败事件（僵尸获得胜利事件）
 */
forward void Han_OnZombieWin();

