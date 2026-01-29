# ArduCopter 目录文件说明（概要）

本文件对 `ArduCopter` 目录下的主要源文件和头文件做简要说明，指出它们的作用与典型用法。用于快速了解各模块在飞控固件中的角色。每项保持简短，便于查阅。

- `APM_Config.h`：平台/板级配置模板，定义构建时的板级选项。用法：按需复制为本地 `APM_Config.h` 并修改硬件相关配置。
- `AP_ExternalControl_Copter.h` / `.cpp`：支持外部导航/控制器接口（外部控制模式）的实现和接口定义。用法：集成外部控制源时参考。
- `GCS_Mavlink.cpp` / `GCS_Mavlink.h`：MAVLink 通信实现，处理地面站消息收发。用法：调试和扩展地面站交互功能。
- `GCS_Copter.cpp` / `GCS_Copter.h`：Copter 特定的 GCS 接口定制（比如传感器标志、帧类型）。用法：了解 Copter 向 GCS 报告的信息。
- `Log.cpp`：日志接口/集成，负责将系统信息写入日志子系统。用法：查看日志写入点与错误记录调用。
- `mode.cpp` / `mode.h`：飞行模式管理与调度入口（选择、切换、模式接口）。用法：实现或调试具体模式逻辑时从此入手。
- `land_detector.cpp`：着陆检测与状态判断。用法：影响自动降落、解除武装等决策。
- `landing_gear.cpp`：起落架控制接口（扩展/伺服函数）。用法：控制起落架的打开/收回逻辑。
- `inertia.cpp`：惯性校正/初始惯量参数相关计算与接口。用法：与姿态/动力学相关的参数调整参考。
- `heli.cpp`：传统直升机特有逻辑（若为直升机帧）。用法：仅在直升机配置中启用。
- `Parameters.cpp` / `Parameters.h`：Copter 参数定义、默认值与注册表。用法：查找和修改运行时可配置参数。
- `version.h`：固件版本信息宏与字符串。用法：编译时注入版本。
- `UserVariables.h` / `UserParameters.h` / `UserParameters.cpp` / `UserCode.cpp`：用户扩展点，允许自定义参数、变量与用户代码钩子。用法：添加自定义控制或参数定义。
- `tuning.cpp`：PID / 控制回路在线调参支持逻辑。用法：实现/触发 PID 调整功能。
- `toy_mode.h` / `toy_mode.cpp`：玩具/简化模式的实现（低性能演示/教学用途）。
- `terrain.cpp`：地形数据库与地形高度查询使用接口。用法：地形跟随/精确着陆依赖。
- `takeoff_check.cpp` / `takeoff.cpp`：起飞前检查与自动起飞相关逻辑。用法：分析自动起飞流程及安全检查点。
- `system.cpp`：系统级任务、初始化与周期性检查（全局性的运行时管理）。
- `surface_tracking.cpp`：地面/表面追踪（Surface Tracking）实现。用法：无人机低空跟随表面时使用。
- `standby.cpp`：待机/低功耗或挂起逻辑。用法：管理上电或地面待机行为。
- `sensors.cpp`：传感器读取与包装（气压计、激光测距、光流等）。用法：查看传感器数据更新时间和滤波策略。
- `ReleaseNotes.txt`：Copter 发布说明与变更记录。用法：查看版本间变更要点。
- `RC_Channel.h` / `RC_Channel.cpp`：遥控通道解析、辅助开关与模式切换实现。用法：理解 AUX 功能映射与模式开关行为。
- `radio.cpp`：无线电/遥控器相关的更高层逻辑（失败检测等）。
- `precision_landing.cpp`：精确着陆（基于视觉/地标/降落板）实现。
- `navigation.cpp`：导航主循环与导航相关的数据更新、航路点距离/方位计算。用法：查看导航决策何时被触发。
- `motor_test.cpp`：电机测试工具/命令支撑（地面测试）。
- `motors.cpp`：电机输出、武装/解武装判定、输出合成与安全逻辑。用法：查看 ESC 输出、武装流程与自动解武装实现。
- `mode_*` 文件（如 `mode_zigzag.cpp`, `mode_throw.cpp`, `mode_stabilize.cpp`, `mode_loiter.cpp`, `mode_rtl.cpp`, `mode_poshold.cpp`, `mode_land.cpp`, `mode_guided.cpp`, `mode_auto.cpp`, `mode_follow.cpp`, `mode_flowhold.cpp`, `mode_flip.cpp`, `mode_drift.cpp`, `mode_circle.cpp`, `mode_brake.cpp`, `mode_avoid_adsb.cpp`, `mode_autotune.cpp`, `mode_autorotate.cpp`, `mode_acro.cpp`, `mode_acro_heli.cpp`, 等）：每个实现一个具体的飞行模式控制器，用法：查看对应模式的 `init` / `run` / `exit` 行为并调试该模式的控制回路。
- `GCS_Mavlink.h`：MAVLink 通信的声明，供 `GCS_Mavlink.cpp` 使用。
- `fence.cpp`：围栏（地理围栏）检测与响应逻辑。用法：查看围栏触发后如何切换模式或降落。
- `failsafe.cpp`：各种故障（遥控丢失、传感器异常、EKF 错误、电池等）处理与应急行为。用法：理解失控时的自动响应策略。
- `events.cpp`：系统事件处理（如 failsafe 触发/恢复、告警广播等）。
- `esc_calibration.cpp`：ESC/电调校准流程实现。用法：地面校准电机时使用。
- `ekf_check.cpp`：EKF 状态检查与触发 EKF 相关 failsafe。用法：监控定位/姿态估计健康度。
- `defines.h`：Copter 特有的宏、常量、枚举等全局定义。用法：查找常量、默认值与编译开关。
- `crash_check.cpp`：坠机检测与记录逻辑。用法：分析坠机后的处理和日志录制。
- `AP_Rally.h` / `AP_Rally.cpp`：Rally 点（安全点）管理。用法：管理多个紧急点/返航点。
- `AP_State.cpp`：车辆状态机（全局状态标志与运行时状态变量）。
- `AP_Arming.h` / `AP_Arming.cpp`：武装（arming）逻辑与安全检查（包括 pre-arm 检查）。用法：查看武装条件与触发来源（遥控、舵机开关等）。
- `afs_copter.h` / `afs_copter.cpp`：高级 failsafe（Advanced Failsafe）相关实现（可选编译）。
- `autoyaw.cpp`：自动偏航控制逻辑（航向控制辅助）。
- `Attitude.cpp`：姿态控制器接口或支持函数（高频姿态环）。
- `avoidance.cpp` / `avoidance_adsb.cpp` / `avoidance_adsb.h`：避障/ADS-B 避让逻辑。用法：障碍探测与避让策略的实现点。
- `baro_ground_effect.cpp`：气压计相关地面效应处理。
- `commands.cpp`：处理来自 GCS/地面站的高层命令（如立即执行的 MAVLink 命令）。
- `compassmot.cpp`：磁力计去偏（compassmot）相关检测与处理。
- `crash_check.cpp`：重复列出（见上）——坠机检测。
- `inertia.cpp`：列出（见上）——惯性矩相关。
- `AP_ExternalControl_Copter.cpp`：列出（见上）——外部控制实现。
- `AP_State.cpp`：列出（见上）——车辆状态。
- `AP_Rally.cpp`：列出（见上）——Rally 点实现。
- `radio.cpp`：列出（见上）——无线电相关。
- `RC_Channel.cpp` / `RC_Channel.h`：遥控通道与辅助开关映射（见上）。
- `UserCode.cpp` / `UserParameters.*`：允许用户在不修改主固件的情况下增加自定义代码/参数的扩展点。

说明文件中的条目旨在作为浏览参考：要深入了解某一功能，请打开对应源文件并查找 `init` / `run` / `update` / `check` 等函数，或查看头文件中的类/接口说明。

---
补充建议：
- 如需我把每个文件扩展为 1-2 段详细说明（包含关键函数与调用关系），我可以逐个生成并追加到本文件中。
- 如需把说明扩展到子目录（如 `libraries/`、`modules/` 等），请告诉我优先级。

