# Log.cpp 详细讲解 - ArduCopter 日志系统

---

## 第一部分：头文件与编译条件

```cpp
#include "Copter.h"

#if HAL_LOGGING_ENABLED
```

**解释：**
- `#include "Copter.h"` - 包含主头文件，获得所有类和函数定义
- `#if HAL_LOGGING_ENABLED` - 条件编译：只有当硬件平台支持日志时，才编译这个文件
  - 这允许某些不支持日志的平台跳过编译

---

## 第二部分：日志包结构体定义

### 2.1 控制调谐日志结构（log_Control_Tuning）

```cpp
struct PACKED log_Control_Tuning {
    LOG_PACKET_HEADER;              // 日志包头（时间戳等信息）
    
    uint64_t time_us;               // 时间戳（微秒）
    float    throttle_in;           // 输入节流值（0-1）
    float    angle_boost;           // 角度提升系数
    float    throttle_out;          // 输出给电机的节流值
    float    throttle_hover;        // 悬停时的节流值
    float    desired_alt;           // 期望高度（米）
    float    inav_alt;              // 惯性导航计算的高度
    int32_t  baro_alt;              // 气压计测的高度
    float    desired_rangefinder_alt;// 期望测距仪高度
    float    rangefinder_alt;       // 测距仪实际高度
    float    terr_alt;              // 地形高度
    int16_t  target_climb_rate;     // 期望爬升速率（cm/s）
    int16_t  climb_rate;            // 实际爬升速率（cm/s）
};
```

**关键概念：**
- `PACKED` - 取消结构体对齐，节省内存，方便直接写入日志
- `LOG_PACKET_HEADER` - 宏展开为日志包头（通常包含消息类型ID、长度等）

---

## 第三部分：Log_Write_Control_Tuning() 函数详解

这个函数负责收集飞控的调谐数据，并写入日志。

```cpp
void Copter::Log_Write_Control_Tuning()
{
    // ========== 第1步：获取地形高度 ==========
    float terr_alt = 0.0f;                      // 初始化地形高度为0
    
#if AP_TERRAIN_AVAILABLE
    // 如果平台支持地形数据
    if (!terrain.height_above_terrain(terr_alt, true)) {
        // 如果获取失败，使用 NaN（非数字）表示无效数据
        terr_alt = logger.quiet_nan();
    }
#endif

    // ========== 第2步：获取期望高度和爬升速率 ==========
    float des_alt_m = 0.0f;
    int16_t target_climb_rate_cms = 0;
    
    // 只有在非手动油门模式下才有期望的高度和爬升速率
    if (!flightmode->has_manual_throttle()) {
        // 获取位置控制器的期望高度（cm转换为m）
        des_alt_m = pos_control->get_pos_target_z_cm() * 0.01f;
        
        // 获取位置控制器的期望爬升速率（cm/s）
        target_climb_rate_cms = pos_control->get_vel_target_z_cms();
    }
    
    // ========== 第3步：获取测距仪期望高度 ==========
    float desired_rangefinder_alt;
    
    // 获取表面跟踪的目标距离
    if (!surface_tracking.get_target_dist_for_logging(desired_rangefinder_alt)) {
        // 如果获取失败，使用 NaN
        desired_rangefinder_alt = AP::logger().quiet_nan();
    }

    // ========== 第4步：构建日志包 ==========
    struct log_Control_Tuning pkt = {
        LOG_PACKET_HEADER_INIT(LOG_CONTROL_TUNING_MSG),  // 初始化包头
        
        time_us             : AP_HAL::micros64(),        // ? 当前时间（微秒）
        throttle_in         : attitude_control->get_throttle_in(),        // 输入节流
        angle_boost         : attitude_control->angle_boost(),            // 角度提升
        throttle_out        : motors->get_throttle(),                     // 实际输出
        throttle_hover      : motors->get_throttle_hover(),               // 悬停值
        desired_alt         : des_alt_m,                                  // 期望高度
        inav_alt            : inertial_nav.get_position_z_up_cm() * 0.01f,// 当前高度转m
        baro_alt            : baro_alt,                                   // 气压计高度
        desired_rangefinder_alt : desired_rangefinder_alt,                // 期望测距仪高度
        rangefinder_alt     : surface_tracking.get_dist_for_logging(),    // 实际测距仪
        terr_alt            : terr_alt,                                   // 地形高度
        target_climb_rate   : target_climb_rate_cms,                      // 期望爬升率
        climb_rate          : int16_t(inertial_nav.get_velocity_z_up_cms()) // 实际爬升率
    };
    
    // ========== 第5步：写入日志 ==========
    logger.WriteBlock(&pkt, sizeof(pkt));  // 将整个包写入日志缓冲区
}
```

**执行流程图：**
```
Log_Write_Control_Tuning()
  ├─ 获取地形高度 (可选)
  ├─ 获取期望高度和爬升速率 (非手动模式)
  ├─ 获取测距仪数据
  └─ 构建结构体并写入日志
```

---

## 第四部分：其他日志写入函数

### 4.1 Log_Write_Attitude() - 姿态日志

```cpp
void Copter::Log_Write_Attitude()
{
    // 获取期望欧拉角（单位：厘度，即 0.01°）
    Vector3f targets = attitude_control->get_att_target_euler_cd();
    
    // 修正偏航角范围到 0-360°
    targets.z = wrap_360_cd(targets.z);
    
    // 通过 AHRS（姿态参考系统）写入实际姿态
    ahrs.Write_Attitude(targets);
    
    // 写入角速率和推力信息
    ahrs_view->Write_Rate(*motors, *attitude_control, *pos_control);
}
```

**含义：**
- 记录飞机的期望和实际姿态角（roll/pitch/yaw）

---

### 4.2 Log_Write_PIDS() - PID调谐日志

```cpp
void Copter::Log_Write_PIDS()
{
    // 只有当启用 PID 日志记录时才执行
    if (should_log(MASK_LOG_PID)) {
        // 记录滚转率 PID 信息
        logger.Write_PID(LOG_PIDR_MSG, attitude_control->get_rate_roll_pid().get_pid_info());
        
        // 记录俯仰率 PID 信息
        logger.Write_PID(LOG_PIDP_MSG, attitude_control->get_rate_pitch_pid().get_pid_info());
        
        // 记录偏航率 PID 信息
        logger.Write_PID(LOG_PIDY_MSG, attitude_control->get_rate_yaw_pid().get_pid_info());
        
        // 记录高度加速度 PID 信息
        logger.Write_PID(LOG_PIDA_MSG, pos_control->get_accel_z_pid().get_pid_info());
        
        // 如果需要导航调谐，记录水平速度 PID
        if (should_log(MASK_LOG_NTUN) && (flightmode->requires_GPS() || landing_with_GPS())) {
            logger.Write_PID(LOG_PIDN_MSG, pos_control->get_vel_xy_pid().get_pid_info_x());
            logger.Write_PID(LOG_PIDE_MSG, pos_control->get_vel_xy_pid().get_pid_info_y());
        }
    }
}
```

**含义：**
- 记录所有 PID 控制器的 P/I/D 系数、误差、输出等信息
- 用于地面分析和调谐

---

## 第五部分：通用数据日志结构体

这个文件定义了多个通用日志结构体，用于记录不同类型的数据：

### 5.1 16位整数日志

```cpp
struct PACKED log_Data_Int16t {
    LOG_PACKET_HEADER;
    uint64_t time_us;      // 时间戳
    uint8_t id;            // 数据ID（用来标识是哪种数据）
    int16_t data_value;    // 实际数据值（范围：-32768 到 32767）
};
```

### 5.2 32位整数日志

```cpp
struct PACKED log_Data_Int32t {
    LOG_PACKET_HEADER;
    uint64_t time_us;
    uint8_t id;
    int32_t data_value;    // 更大范围的整数
};
```

### 5.3 浮点日志

```cpp
struct PACKED log_Data_Float {
    LOG_PACKET_HEADER;
    uint64_t time_us;
    uint8_t id;
    float data_value;      // 浮点数据
};
```

---

## 第六部分：系统识别（SysID）日志

这用于飞行动力学识别，通常在自动调谐时使用：

```cpp
struct PACKED log_SysIdD {
    LOG_PACKET_HEADER;
    uint64_t time_us;
    float    waveform_time;   // 波形时间
    float    waveform_sample; // 波形采样值
    float    waveform_freq;   // 波形频率
    float    angle_x;         // X 轴角度（欧拉角）
    float    angle_y;         // Y 轴角度
    float    angle_z;         // Z 轴角度
    float    accel_x;         // X 轴加速度
    float    accel_y;         // Y 轴加速度
    float    accel_z;         // Z 轴加速度
};
```

---

## 第七部分：日志结构数组定义

```cpp
const struct LogStructure Copter::log_structure[] = {
    LOG_COMMON_STRUCTURES,  // 包含所有通用日志格式（来自库）
    
    // 每一项定义一种日志消息格式
    { 
        LOG_PARAMTUNE_MSG,              // 消息类型 ID
        sizeof(log_ParameterTuning),    // 数据包大小（字节）
        "PTUN",                         // 日志消息名称（4字符）
        "QBfff",                        // 数据类型格式字符串
        "TimeUS,Param,TunVal,TunMin,TunMax",  // 字段名称
        "s----",                        // 单位（s=秒, -=无单位）
        "F----"                         // 精度（F=64位, B=8位等）
    },
    
    { 
        LOG_CONTROL_TUNING_MSG, 
        sizeof(log_Control_Tuning),
        "CTUN", 
        "Qffffffefffhh",    // 格式说明：Q=uint64, f=float, e=int32, h=int16
        "TimeUS,ThI,ABst,ThO,ThH,DAlt,Alt,BAlt,DSAlt,SAlt,TAlt,DCRt,CRt",
        "s----mmmmmmnn",    // m=米, n=其他
        "F----00B000BB"     // B=8位, 0=其他精度
        , true              // 最后一个参数表示日志优先级
    },
    // ... 更多日志格式定义
};
```

**格式字符串说明：**
| 字符 | 类型 | 大小 |
|------|------|------|
| Q | uint64_t | 8字节 |
| I | uint32_t | 4字节 |
| i | int32_t | 4字节 |
| H | uint16_t | 2字节 |
| h | int16_t | 2字节 |
| B | uint8_t | 1字节 |
| f | float | 4字节 |
| d | double | 8字节 |
| e | int32_t | 4字节 |

---

## 第八部分：启动消息日志

```cpp
void Copter::Log_Write_Vehicle_Startup_Messages()
{
    // 获取飞行器型号信息，存储在缓冲区中
    char frame_and_type_string[30];
    copter.motors->get_frame_and_type_string(frame_and_type_string, ARRAY_SIZE(frame_and_type_string));
    
    // 写入机架和类型信息
    logger.Write_MessageF("%s", frame_and_type_string);
    
    // 写入当前飞行模式
    logger.Write_Mode((uint8_t)flightmode->mode_number(), control_mode_reason);
    
    // 写入家点和原点信息
    ahrs.Log_Write_Home_And_Origin();
    
    // 写入 GPS 启动消息
    gps.Write_AP_Logger_Log_Startup_messages();
}
```

**作用：** 在每次启动时记录飞行器的配置信息

---

## 第九部分：日志初始化

```cpp
void Copter::log_init(void)
{
    // 使用日志结构数组初始化日志系统
    logger.Init(
        log_structure,              // 日志格式定义数组
        ARRAY_SIZE(log_structure)   // 数组元素个数
    );
}
```

**作用：** 告诉日志系统有哪些消息类型以及它们的格式

---

## 总结：整个日志系统的工作流程

```
1. 初始化阶段 (log_init)
   └─ 将 log_structure 数组注册到日志系统

2. 飞行过程中（主循环调用）
   ├─ Log_Write_Control_Tuning()     每秒调用
   ├─ Log_Write_Attitude()            定期调用
   ├─ Log_Write_PIDS()                定期调用
   └─ ... 其他日志函数

3. 每个函数的工作流
   ├─ 收集当前传感器和控制器数据
   ├─ 填充结构体
   └─ logger.WriteBlock() 写入日志

4. 日志文件保存
   └─ 所有日志最终保存到存储设备（SD卡等）
```

---

## 关键概念总结

| 概念 | 说明 |
|------|------|
| **LogStructure** | 定义一种日志消息的格式（名称、字段、单位等） |
| **log_Control_Tuning** | 一个"包"（packet），包含一个时间点的多个数据 |
| **WriteBlock()** | 将数据包直接写入日志缓冲区 |
| **PACKED** | 确保结构体字段按顺序排列，没有填充字节 |
| **应条件编译** | #if/#endif 用于支持不同平台的编译 |

---

## 实际用途示例

当你分析日志时，可以看到：
- **CTUN** 消息：包含油门、高度、爬升率等控制调谐数据
- **PTUN** 消息：包含参数调谐值
- **SIDD** 消息：包含系统识别数据（用于自动调谐）
- **GUIP/GUIA** 消息：引导模式的目标位置和姿态

所有这些数据帮助开发者和用户：
- 调试飞控逻辑
- 优化 PID 参数
- 分析飞行性能
- 诊断故障
