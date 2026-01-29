# ArduCopter 启动流程详解

## 概述

ArduCopter 的启动过程遵循 Arduino 风格的启动模式，分为两个主要阶段：
- **setup()** - 初始化阶段（执行一次）
- **loop()** - 主循环（无限循环）

---

## 完整启动流程

### 第一阶段：硬件启动 (HAL 层)

```
设备上电
    ↓
STM32/ARM 处理器启动 bootloader
    ↓
加载固件到内存
    ↓
执行 main() 函数
    ↓
AP_HAL 初始化硬件（时钟、GPIO、串口、中断）
```

### 第二阶段：应用启动 (AP_Vehicle 层)

#### 1?? **main() 函数入口**

文件：[libraries/AP_HAL/AP_HAL_Main.h](libraries/AP_HAL/AP_HAL_Main.h)

```cpp
#define AP_HAL_MAIN_CALLBACKS(CALLBACKS) extern "C" { \
    int AP_MAIN(int argc, char* const argv[]); \
    int AP_MAIN(int argc, char* const argv[]) { \
        hal.run(argc, argv, CALLBACKS);     // 启动HAL \
        return 0; \
    } \
}
```

在 [ArduCopter/Copter.cpp#L886](ArduCopter/Copter.cpp#L886) 调用：

```cpp
AP_HAL_MAIN_CALLBACKS(&copter);  // 注册 Copter 对象的 setup() 和 loop() 回调
```

#### 2?? **setup() 初始化函数**

文件：[libraries/AP_Vehicle/AP_Vehicle.cpp#L267](libraries/AP_Vehicle/AP_Vehicle.cpp#L267)

**执行顺序：**

```cpp
void AP_Vehicle::setup()
{
    // 1?? 加载参数默认值
    AP_Param::setup_sketch_defaults();

    // 2?? 初始化 Debug 串口
    serial_manager.init_console();
    DEV_PRINTF("\n\nInit %s", AP::fwversion().fw_string);
    
    // 3?? 加载持久化参数（从EEPROM）
    AP_Param::check_var_info();
    load_parameters();  // ? 从 EEPROM 读取参数
    
    // 4?? 初始化任务调度器
    const AP_Scheduler::Task *tasks;
    uint8_t task_count;
    uint32_t log_bit;
    get_scheduler_tasks(tasks, task_count, log_bit);  // ? 获取任务列表
    AP::scheduler().init(tasks, task_count, log_bit);
    
    // 5?? 初始化 RC 通道
    set_control_channels();
    
    // 6?? 初始化地面站通讯
    gcs().init();
    serial_manager.init();  // ? 初始化所有串口（Telemetry、Lidar等）
    
    // 7?? 运行车型特定初始化
    init_ardupilot();  // ? 核心初始化函数，在 Copter.cpp 或 system.cpp 中定义
}
```

#### 3?? **init_ardupilot() 核心初始化**

文件：[ArduCopter/system.cpp#L17](ArduCopter/system.cpp#L17)

**初始化顺序（关键）：**

```cpp
void Copter::init_ardupilot()
{
    // ========== 1. 硬件配置 ==========
    BoardConfig.init();           // 初始化电源板配置
    can_mgr.init();               // CAN 总线初始化
    g2.gripper.init();            // 夹爪初始化
    g2.winch.init();              // 绞盘初始化

    // ========== 2. Notify 系统 ==========
    notify.init();                // ? LED/蜂鸣器初始化
    notify_flight_mode();         // 显示初始飞行模式

    // ========== 3. 电源系统 ==========
    battery.init();               // 电池监测初始化
    rssi.init();                  // RSSI 初始化

    // ========== 4. 传感器系统 ==========
    barometer.init();             // 气压计初始化
    gcs().setup_uarts();          // 配置串口
    osd.init();                   // OSD 初始化

#if HAL_LOGGING_ENABLED
    log_init();                   // 日志系统初始化
#endif

    // ========== 5. 直升机特定初始化 ==========
#if FRAME_CONFIG == HELI_FRAME
    heli_init();
#endif

    // ========== 6. RC 系统初始化 ==========
    init_rc_in();                 // ? 初始化 RC 输入通道
    rc().convert_options(...);    // RC 选项转换
    rc().init();                  // RC 初始化
    init_rc_out();                // ? 初始化 RC 输出（ESC/Servo）

    // ========== 7. ESC 校准检查 ==========
    esc_calibration_startup_check();  // ESC 启动校准检查
    ap.initialised_params = true;

    // ========== 8. 保护系统 ==========
    relay.init();                 // 继电器初始化
    hal.scheduler->register_timer_failsafe(...);  // 注册看门狗

    // ========== 9. 导航系统 ==========
    gps.init(serial_manager);     // GPS 初始化
    AP::compass().init();         // 罗盘初始化
    
#if AP_AIRSPEED_ENABLED
    airspeed.init();              // 空速管初始化
#endif

    attitude_control->parameter_sanity_check();  // 姿态控制参数检查

#if AP_OPTICALFLOW_ENABLED
    optflow.init(MASK_LOG_OPTFLOW);  // 光流传感器初始化
#endif

#if HAL_MOUNT_ENABLED
    camera_mount.init();          // 相机云台初始化
#endif

#if AP_CAMERA_ENABLED
    camera.init();                // 相机初始化
#endif

    // ========== 10. 高度系统 ==========
    barometer.calibrate();        // 气压计地面校准
    
#if RANGEFINDER_ENABLED == ENABLED
    init_rangefinder();           // 激光测距仪初始化
#endif

    // ========== 11. 位置系统 ==========
#if HAL_PROXIMITY_ENABLED
    g2.proximity.init();          // 接近传感器初始化
#endif

#if AP_BEACON_ENABLED
    g2.beacon.init();             // 信标初始化
#endif

    // ========== 12. 使命规划 ==========
#if MODE_AUTO_ENABLED == ENABLED
    mode_auto.mission.init();     // 自动模式任务初始化
#endif

#if MODE_SMARTRTL_ENABLED == ENABLED
    g2.smart_rtl.init();          // SmartRTL 初始化
#endif

    // ========== 13. 惯性导航系统 ==========
    startup_INS_ground();         // ? IMU 地面启动初始化（最重要！）

    // ========== 14. 脚本 ==========
#if AP_SCRIPTING_ENABLED
    g2.scripting.init();          // Lua 脚本引擎初始化
#endif
}
```

---

## 参数加载机制（RCS 文件）

### 参数存储位置

ArduCopter 的参数**不存储在 .rcs 文件**中，而是存储在以下位置：

| 存储位置 | 说明 | 文件类型 |
|---------|------|---------|
| **EEPROM/Flash** | 飞控上的持久化存储 | 二进制格式 |
| **MAVLink 参数** | 通过 PARAM_VALUE 消息发送 | 网络协议 |
| **参数文件** | QGroundControl/Mission Planner | `.params` / `.txt` |
| **默认参数** | 编译时定义 | C++ 代码 |

### 参数加载流程

```cpp
void AP_Vehicle::setup()
{
    // 步骤 1: 加载默认值（代码中定义）
    AP_Param::setup_sketch_defaults();
    
    // 步骤 2: 检查参数表的完整性和版本
    AP_Param::check_var_info();
    
    // 步骤 3: 从 EEPROM 加载参数
    load_parameters();  // 虚函数，由各车型实现
}

void Copter::load_parameters(void)
{
    // 检查格式版本
    if (!g.format_version.load() ||
        g.format_version != Parameters::k_format_version) {
        
        // 如果版本不匹配，清除所有参数
        DEV_PRINTF("Firmware change: erasing EEPROM...\n");
        StorageManager::erase();
        AP_Param::erase_all();
        g.format_version.set_and_save(Parameters::k_format_version);
    }

    // 从 EEPROM 加载所有参数
    AP_Param::load_all();
    
    // 转换旧参数格式
    AP_Param::convert_old_parameters(&conversion_table[0], 
                                      ARRAY_SIZE(conversion_table));
    
    // 车型特定的参数转换
#if AP_LANDINGGEAR_ENABLED
    convert_lgr_parameters();
#endif
}
```

### 参数存储结构

EEPROM 中的参数使用 **StorageManager** 管理：

```
EEPROM Layout:
┌─────────────────────┐
│ Format Version      │  (1 项)
│ Flight Mode Config  │  (多项)
│ PID 参数            │  (多项)
│ Sensor Calibration  │  (多项)
│ Safety Parameters   │  (多项)
│ ...                 │
└─────────────────────┘
```

**参数持久化：**
```cpp
// 设置并立即保存到 EEPROM
g.format_version.set_and_save(value);

// 在变量修改时自动保存
AP_Param::set_value(param_name, new_value, true);
```

---

## 第三阶段：主循环 (loop())

文件：[libraries/AP_Vehicle/AP_Vehicle.cpp#L455](libraries/AP_Vehicle/AP_Vehicle.cpp#L455)

```cpp
void AP_Vehicle::loop()
{
    // 1?? 执行调度任务
    scheduler.loop();  // ? 运行任务调度器
    
    // 2?? 更新时间增量
    G_Dt = scheduler.get_loop_period_s();
    
    // 3?? 安全初始化（仅第一次）
    if (!done_safety_init) {
        done_safety_init = true;
        BoardConfig.init_safety();  // 禁用安全开关
        
        // 显示 RC 输出模式
        char banner_msg[50];
        if (hal.rcout->get_output_mode_banner(banner_msg, sizeof(banner_msg))) {
            GCS_SEND_TEXT(MAV_SEVERITY_INFO, "%s", banner_msg);
        }
    }
    
    // 4?? 错误检查
    const uint32_t new_internal_errors = AP::internalerror().errors();
    if(_last_internal_errors != new_internal_errors) {
        LOGGER_WRITE_ERROR(LogErrorSubsystem::INTERNAL_ERROR, 
                          LogErrorCode::INTERNAL_ERRORS_DETECTED);
        GCS_SEND_TEXT(MAV_SEVERITY_CRITICAL, "Internal Errors 0x%x", 
                     (unsigned)new_internal_errors);
        _last_internal_errors = new_internal_errors;
    }
}
```

### 任务调度器执行流程

文件：[ArduCopter/Copter.cpp#L55-L250](ArduCopter/Copter.cpp#L55-L250)

**任务表定义：**

```cpp
const AP_Scheduler::Task Copter::scheduler_tasks[] = {
    // ========== FAST_TASK (必须始终运行) ==========
    FAST_TASK_CLASS(AP_InertialSensor, &copter.ins, update),     // IMU 更新
    FAST_TASK(run_rate_controller),                              // 速率控制器
    FAST_TASK(motors_output),                                    // 电机输出
    FAST_TASK(read_AHRS),                                        // AHRS 更新
    FAST_TASK(update_flight_mode),                               // 飞行模式更新

    // ========== 标准任务 ==========
    SCHED_TASK(rc_loop, 250, 130, 3),                            // 250Hz - RC 输入
    SCHED_TASK(throttle_loop, 50, 75, 6),                        // 50Hz - 油门
    SCHED_TASK_CLASS(AP_GPS, &copter.gps, update, 50, 200, 9),  // 50Hz - GPS
    SCHED_TASK(update_batt_compass, 10, 120, 15),               // 10Hz - 电池罗盘
    SCHED_TASK(arm_motors_check, 10, 50, 21),                   // 10Hz - 武装检查
    SCHED_TASK(one_hz_loop, 1, 100, 81),                        // 1Hz - 系统检查
    
    // ========== 条件任务 ==========
#if HAL_LOGGING_ENABLED
    SCHED_TASK(ten_hz_logging_loop, 10, 350, 114),             // 日志记录
#endif
    
    // ... 更多任务 ...
};
```

**任务参数说明：**
- 第1参数：任务函数指针
- 第2参数：执行频率 (Hz)
- 第3参数：最大执行时间 (μs)
- 第4参数：优先级 (0=最高)

**高优先级任务** (FAST_TASK)：
- 始终执行，不会被跳过
- 用于实时控制（IMU、电机、AHRS）

**低优先级任务**：
- 如果时间超过预算，会被延迟或跳过
- 用于非实时操作（GPS、日志、通讯）

---

## 启动时间线

```
启动事件                    耗时        累计时间
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
电源上电                    0ms         0ms
硬件初始化                  ~100ms      100ms
main() 函数启动             -           100ms
setup() 开始               -           100ms
  - 参数加载                ~50ms       150ms
  - 任务调度器初始化        ~10ms       160ms
  - 串口初始化              ~20ms       180ms
  - IMU/GPS 初始化          ~200ms      380ms
  - 罗盘初始化              ~100ms      480ms
  - Barometer 校准          ~500ms      980ms
setup() 完成                -           ~1000ms
loop() 开始循环             -           ~1000ms
第一个任务执行              -           ~1000ms

典型总启动时间：~1-2 秒（从上电到可飞行）
```

---

## 文件位置速查表

| 功能 | 文件位置 | 行号 |
|------|---------|------|
| **main() 入口** | [AP_HAL_Main.h](libraries/AP_HAL/AP_HAL_Main.h) | 35 |
| **setup() 实现** | [AP_Vehicle.cpp](libraries/AP_Vehicle/AP_Vehicle.cpp) | 267 |
| **loop() 实现** | [AP_Vehicle.cpp](libraries/AP_Vehicle/AP_Vehicle.cpp) | 455 |
| **init_ardupilot()** | [system.cpp](ArduCopter/system.cpp) | 17 |
| **任务表定义** | [Copter.cpp](ArduCopter/Copter.cpp) | 55-250 |
| **参数定义** | [Parameters.cpp](ArduCopter/Parameters.cpp) | 1-1000 |
| **参数加载** | [Parameters.cpp](ArduCopter/Parameters.cpp) | 1353 |
| **回调注册** | [Copter.cpp](ArduCopter/Copter.cpp) | 886 |

---

## 常见初始化问题

### 1?? 参数加载失败

**症状：** 启动后参数全部重置为默认值

**原因：**
- EEPROM 损坏
- 固件版本更新（格式版本不匹配）
- 参数定义与编译的固件不一致

**解决：**
```cpp
// 检查 EEPROM 状态
load_parameters() 会自动检测并清除不匹配的参数

// 手动擦除 EEPROM
StorageManager::erase();
AP_Param::erase_all();
```

### 2?? IMU 校准失败

**症状：** "IMU not healthy" 消息

**原因：**
- IMU 未成功初始化
- 传感器故障
- 加速度计偏置过大

**解决：**
在 `startup_INS_ground()` 中重新校准加速度计和陀螺仪

### 3?? 任务超时

**症状：** "SCHED: Loop rate 400Hz, "

**原因：**
- 某个任务执行时间超过预算
- 任务优先级设置不当

**调试：**
```cpp
// 在 loop() 中打印任务执行统计
AP::scheduler().print_stats();
```

---

## 启动状态 LED 指示

初始化过程中 LED 显示状态：

```
启动阶段                LED 状态
━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━━
上电                    红灯闪烁 (初始化中)
初始化完成              红灯常亮 (等待 GPS)
GPS 定位中              蓝灯闪烁 (搜索 GPS)
GPS 已定位              蓝灯常亮
可以解锁                绿灯常亮
已解锁                  绿灯快速闪烁
```

---

## 总结

**关键启动步骤：**

1. **硬件初始化** (HAL) → 时钟、GPIO、中断
2. **setup()** → 参数加载、任务调度器初始化
3. **init_ardupilot()** → 车型特定初始化（最耗时）
4. **loop()** → 无限任务循环

**参数存储：**
- 不使用 `.rcs` 文件
- 参数存储在 EEPROM/Flash
- 通过 MAVLink 或地面站修改和导出

**优化启动时间：**
- 减少不必要的传感器初始化
- 异步初始化非关键传感器
- 使用快速 EEPROM 访问

**最后更新：** 2026年1月28日  
**适用版本：** ArduCopter 4.5.7
