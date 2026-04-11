<h1>🧟 HanZombieScenario 大灾变插件说明文档</h1> 
<p class="en">HanZombieScenario Plugin Documentation (Chinese & English)</p>

如果你喜欢这个插件,可以用以下方式支持我,感谢!

If you like this plugin, you can support me in the following ways. Thank you!

[![ko-fi](https://github.com/user-attachments/assets/3c01a28f-efe2-48af-9385-cef3a99fbb8c)](https://www.ifdian.net/a/XMHHAN)
[![paypal](https://github.com/user-attachments/assets/da293573-12c8-40bc-b956-d562cd46d4ae)](https://www.paypal.com/paypalme/XMHHAN)
[![ko-fi](https://ko-fi.com/img/githubbutton_sm.svg)](https://ko-fi.com/Z8Z31PY52N)

---


<div class="section">
  <h2>插件介绍 / Introduction</h2>
  <p>本插件基于 <strong>SourceMod 1.12</strong> 开发，为 <strong>CS 起源 64 位新版本</strong> 提供 PvE 丧尸玩法。它通过将“人质实体”作为受击点，附加动画模型，模拟出高度拟真的 NPC 丧尸与怪物行为。</p>
  <p class="en">This plugin is built on <strong>SourceMod 1.12</strong> and targets the <strong>CS:S 64-bit version</strong>, offering rich zombie/monster PvE gameplay using hostage entities with attached animation models.</p>

  <div class="note">
    ⚠️ 注意：<br>
    当地图中已存在 NPC 时，请勿执行 BOT 添加或复活操作！这将导致服务器崩溃。<br>
    BOT 操作应在 NPC 创建之前完成。人类玩家不受此限制。
  </div>
</div>

<div class="section">
  <h2>核心功能 / Core Features</h2>
  <ul>
    <li>🧟‍♂️ 自定义丧尸系统（模型/动画/伤害/受击）</li>
    <li>📦 多阶段关卡系统（击杀目标/血量加成/剧情提示）</li>
    <li>💥 多种特殊手雷（燃烧弹、照明弹、冰冻弹）</li>
    <li>🔊 自定义 VOX 音效播放系统（倒计时、胜负提示、背景音乐）</li>
    <li>🧩 内建 API，支持开发者轻松扩展玩法</li>
  </ul>
  <p class="en">Core functionalities include:<br>
  Custom zombie system (models, animations, damage), multi-stage mission system, special grenade mechanics, VOX audio system, and a complete public API for developers.</p>
</div>

<div class="section">
  <h2>配置说明 / Configuration Overview</h2>

  <h3>🔧 主配置文件：ScenarioConfig</h3>
  <ul>
    <li><code>enable_plugins</code> - 启用插件</li>
    <li><code>max_zombie_spawn</code> - 最大 NPC 数量（推荐不超过 2048 总实体数）</li>
    <li><code>skybox</code> / <code>lightstyle</code> - 更改天空盒与地图亮度</li>
    <li><code>background_music</code> - 循环 BGM（支持自定义时长）</li>
    <li><code>player_reborn_sec</code> - 玩家复活时间</li>
    <li><code>allow_buy_anywhere</code> - 任意地点打开购买菜单</li>
    <li><code>Zombie_SpawnTime</code> - 丧尸生成延迟</li>
    <li><code>Kill_Money</code> / <code>Max_Money</code> - 击杀金钱系统</li>
    <li><code>Show_Print_Damage</code> - 显示伤害输出</li>
  </ul>

  <h3>🧟 丧尸配置：ZombieSystem</h3>
  每种丧尸包括模型、摩擦力、动画名称、攻击距离、音效、血量等详细属性，支持 Boss 类型。<br>
  <span class="en">Each zombie entry defines its visual model, movement, animations, sounds, and damage/health attributes. Boss zombies are supported.</span>

  <h3>🎯 关卡配置：HanZombieScenarioStage</h3>
  每关卡设置击杀数量、血量加成、可用丧尸列表、剧情提示，可选特殊 Boss。
  <pre><code>"count": 击杀数, "zombie": 丧尸类型, "specialzombie": Boss类型, "storyline": 提示文本</code></pre>

  <h3>🔊 VOX 系统配置：ScenarioVoxConfig</h3>
  倒计时语音、开始提示、人类/丧尸胜利语音、背景胜利/失败音乐，均可自定义路径与顺序。

  <h3>💣 手雷系统配置：ScenarioGrenadeConfig</h3>
  支持三种手雷：燃烧弹、照明弹、冰冻弹。每种均支持开关、特效、持续时间、范围、伤害等参数。

  <p class="en">All configuration files are structured in KV format. Please refer to the included sample configs in the repository for actual file structures.</p>
</div>

<div class="section">
  <h2>管理员命令 / Admin Commands</h2>
  <ul>
    <li><code>hzs_setday &lt;x&gt;</code> - 设置当前关卡 (Set current stage)</li>
    <li><code>hzs_restgame</code> - 重置本局游戏 (Reset current game)</li>
  </ul>
</div>

<div class="section">
  <h2>🧠 开发者 API / Developer API</h2>
  插件提供一套完善 API，允许其他插件模块读取、操作 NPC 丧尸行为，例如：
  <pre><code>
Han_SetCustomAnimation(zombie, animName, duration);
Han_IsZombie(entity);
Han_GetZombieCount();
Han_GetZombieByIndex(index);
Han_GetZombieName(zombie, buffer, maxlen);
Han_SafeDamageZombie(attacker, zombie, damage);
  </code></pre>

  <strong>事件转发：</strong>
  <ul>
    <li><code>Han_OnZombieCreated</code> - 丧尸生成事件</li>
    <li><code>Han_OnZombieDeath</code> - 丧尸死亡事件</li>
    <li><code>Han_OnZombieHurt</code> - 丧尸被攻击事件</li>
    <li><code>Han_OnZombieAttack</code> - 丧尸攻击玩家事件</li>
  </ul>

  <p class="en">You can integrate this plugin into your own gameplay mods or gamemodes using the above API. Full documentation is included in <code>HanZombieScenarioAPI.inc</code>.</p>
</div>

<div class="section">
  <h2>📝 模型动画注意事项 / Animation Setup Tip</h2>
  使用模型查看器（如 HLMV）预览并获取模型动画名称及时长。<br>
  请根据动画名设置 <code>idle_anim</code>, <code>move_anim</code>, <code>attack_anim</code> 等字段。多个动画使用英文逗号分隔，格式为：<br>
  <code>动画名:持续时间</code>
</div>

<div class="section">
  <h2>💡 总结 / Final Notes</h2>
  <p>HanZombieScenario 插件提供、可拓展的 PvE 框架，支持完整的地图/关卡/怪物/音效/手雷玩法，适合打造自定义合作模式。</p>
  <p class="en">HanZombieScenario is a complete PvE zombie mod base for CS:S with custom configuration and API, allowing rich gameplay and expansion possibilities.</p>
</div>

</body>
</html>

<div class="section">
  <h2>🎨 丧尸属性变体 / Zombie Variants by Color</h2>

  <p>插件支持创建多个拥有相同模型与动画但不同属性（如颜色、血量、伤害等）的丧尸，用于制作更具挑战的关卡敌人。例如：</p>

  <pre><code>
"putong_stage1_red"
{
    "name"            "狂暴丧尸"
    "model"           "models/player/putonghe/tank_zombi_host.mdl"
    "friction"        "1.5"
    "attack_range"    "150.0"
    "health"          "150"
    "damage"          "5"
    "hitbox_size"     "1.0"
    "anime_size"      "1.0"
    "idle_anim"       "walk_upper_knife:1.0" 
    "move_anim"       "walk_upper_knife:1.0,run_upper_knife:1.0"
    "attack_anim"     "Idle_shoot_knife:2.0"
    "getattack_anim"  "head_flinch:0.5,gut_flinch:0.5"
    "death_anim"      "death1:1.2,death2:1.2,death3:1.2"
    "hit_sound"       "weapons/zombi/zombi_hit1.wav,weapons/zombi/zombi_stab.wav"
    "hurt_sound"      "weapons/fists/zombi_hurt_01.wav,weapons/fists/zombi_hurt_02.wav"
    "death_sound"     "weapons/fists/zombi_death.wav"
    <mark>"color"           "255 0 0 255"</mark>  // 红色：更高伤害
}
  </code></pre>

  <pre><code>
"putong_stage1_purple"
{
    "name"            "强化丧尸"
    "model"           "models/player/putonghe/tank_zombi_host.mdl"
    "friction"        "1.5"
    "attack_range"    "150.0"
    "health"          "300"
    "damage"          "1"
    "hitbox_size"     "1.0"
    "anime_size"      "1.0"
    "idle_anim"       "walk_upper_knife:1.0" 
    "move_anim"       "walk_upper_knife:1.0,run_upper_knife:1.0"
    "attack_anim"     "Idle_shoot_knife:2.0"
    "getattack_anim"  "head_flinch:0.5,gut_flinch:0.5"
    "death_anim"      "death1:1.2,death2:1.2,death3:1.2"
    "hit_sound"       "weapons/zombi/zombi_hit1.wav,weapons/zombi/zombi_stab.wav"
    "hurt_sound"      "weapons/fists/zombi_hurt_01.wav,weapons/fists/zombi_hurt_02.wav"
    "death_sound"     "weapons/fists/zombi_death.wav"
    <mark>"color"           "128 0 255 255"</mark>  // 紫色：更高血量
}
  </code></pre>

  <p>你可以在关卡配置中自由组合这些丧尸以实现难度逐步递增的挑战体验。</p>
  <p class="en">Zombies can be visually and functionally varied using the <code>color</code> field. For example, red zombies may have higher damage, while purple ones may have higher health. This enables flexible stage-based difficulty design.</p>
</div>

<div class="section">
  <h2>📘 关卡系统配置 / Stage System Configuration</h2>

  <p>关卡配置文件定义了每一阶段刷怪的类型、数量、强化属性和剧情文本，格式如下：</p>

  <pre><code>
"Stage"
{
    "关卡1"
    {
        "count"         "5"   // 生成普通丧尸数量
        "healthboost"   "20"  // 所有丧尸血量增加值（+20HP）
        "zombie"        "putong_stage1"  // 支持多个类型，用英文逗号 , 分隔
        "maxszombiecount" "1" // 同时最多存在的特殊丧尸数量
        "specialzombie" "boss_pangzi"  // 特殊丧尸的ID
        "storyline"     "不明病毒爆发，灭杀小队请立即出动！" // 本阶段剧情
    }

    "关卡2"
    {
        "count"         "10"
        "healthboost"   "40"
        "zombie"        "fast_stage1"
        "storyline"     "第二关"
    }

    "关卡3"
    {
        "count"         "15"
        "healthboost"   "60"
        "zombie"        "putong_stage1,fast_stage1"
        "storyline"     "第三关"
    }

    ...
}
  </code></pre>

  <h3>🧩 字段说明 / Field Explanation</h3>
  <ul>
    <li><b>count</b>: 每个阶段生成的普通丧尸数量。</li>
    <li><b>healthboost</b>: 所有该阶段丧尸的额外血量加成，单位为 HP。</li>
    <li><b>zombie</b>: 本阶段使用的普通丧尸类型 ID，可用多个 ID 用逗号 <code>,</code> 分隔。</li>
    <li><b>specialzombie</b>: 本阶段生成的特殊丧尸类型（如 Boss），可选。</li>
    <li><b>maxszombiecount</b>: 同时可存在的特殊丧尸最大数量，默认不限制。</li>
    <li><b>storyline</b>: 显示在该阶段开始时的剧情文本。</li>
  </ul>

  <p class="en">Each stage defines the type and number of zombies to spawn, additional health boost, storyline, and optional special zombies (bosses). Multiple stages can be easily copied and modified to build complex progression.</p>
</div>


