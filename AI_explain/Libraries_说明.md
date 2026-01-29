# Libraries 目录说明（概要）

此文档对 `libraries` 目录下的主要子模块做一行式说明，便于快速定位功能与用途。若需我可为指定库展开为 1–2 段的详细说明并加入关键文件链接。

- AC_AttitudeControl: 姿态控制器实现（多旋翼/直升机的姿态控制律）。
- AC_Autorotation: 直升机自旋降（autorotation）控制模块。
- AC_AutoTune: 自动整定（在线 PID 优化）支持。
- AC_Avoidance: 路径规划与障碍避让高层接口。
- AC_CustomControl: 自定义控制器接口，接入第三方控制算法。
- AC_Fence: 地理围栏检测与应对逻辑。
- AC_InputManager: 遥控/输入管理与映射。
- AC_PID: 通用 PID 控制器工具。
- AC_PrecLand: 精确着陆状态机与支持。
- AC_Sprayer: 农业喷洒器控制（喷洒任务支持）。
- AC_WPNav: 航点导航（航线管理与跟踪）。
- APM_Control: 高层控制兼容层与任务入口。
- AP_AccelCal: 加速度计校准工具。
- AP_ADC: 模数转换抽象与读取。
- AP_ADSB: ADS?B 空中交通感知支持。
- AP_AdvancedFailsafe: 高级故障备援扩展。
- AP_AHRS: 姿态航向参考系统（融合/接口）。
- AP_Airspeed: 空速传感器驱动与校准。
- AP_AIS: 船舶 AIS 支持（海事识别）。
- AP_Arming: 武装/解武装与 pre?arm 检查。
- AP_Avoidance: 低级避让/碰撞规避算法。
- AP_Baro: 气压计驱动与高度支持。
- AP_BattMonitor: 电池监测与告警逻辑。
- AP_Beacon: 信标/广播支持。
- AP_BLHeli: BLHeli ESC 通信与遥测。
- AP_BoardConfig: 板级配置与检测。
- AP_Button: 板上按钮（短/长按）处理。
- AP_Camera: 相机控制、RunCam/Onvif 等接口。
- AP_CANManager: CAN 总线管理与节点注册。
- AP_CheckFirmware: 固件校验与一致性检查。
- AP_Common: 通用工具、数据结构与基础设施。
- AP_Compass: 磁力计抽象、校准支持。
- AP_CSVReader: CSV 读取工具。
- AP_CustomRotations: 自定义电机旋转映射（混控）。
- AP_DDS: DDS 客户端支持（分布式数据）。
- AP_Declination: 磁偏角计算管理。
- AP_DroneCAN / AP_PiccoloCAN: CAN 协议（DroneCAN/Piccolo）支持。
- AP_EFI: 发动机 EFI/内燃机监控接口。
- AP_ESC_Telem: 电调遥测读取与汇总。
- AP_ExternalAHRS: 外部 AHRS 设备支持。
- AP_ExternalControl: 外部导航/控制主机接入接口。
- AP_Filesystem / AP_FlashStorage / AP_FlashIface: 存储、文件系统与 Flash 抽象。
- AP_Follow: 跟随（Follow）模式支持。
- AP_Frsky_Telem / AP_Hott_Telem / AP_LTM_Telem: 各种遥测协议支持。
- AP_Generator: 外围发生器/点火控制支持。
- AP_GPS: GPS 驱动与定位支持。
- AP_Gripper: 抓取器/机械手控制支持。
- AP_GyroFFT: 陀螺 FFT 分析与振动检测。
- AP_HAL / AP_HAL_*: 硬件抽象层与各平台实现（ChibiOS/SITL/Linux/ESP32 等）。
- AP_InertialNav / AP_InertialSensor: 惯性导航与传感器采集/滤波接口。
- AP_InternalError: 内部错误追踪与编码。
- AP_IOMCU: IO MCU（外部 IO 芯片）接口。
- AP_OpticalFlow / AP_VisualOdom / AP_IRLock: 视觉定位、光流与红外定位支持。
- AP_Landing / AP_LandingGear: 着陆流程与起落架控制。
- AP_Math: 向量/矩阵/数值运算基础库。
- AP_Mission: 航点任务管理与执行。
- AP_Motors / AR_Motors: 电机输出封装（多旋翼/UGV/定翼）。
- AP_Mount: 云台/相机安装与控制。
- AP_MSP: MSP（MultiWii Serial Protocol）兼容接口。
- AP_NavEKF / AP_NavEKF2 / AP_NavEKF3: EKF 状态估计器实现。
- AP_Navigation: 导航辅助工具与管理函数。
- AP_Networking: 网络接口（TCP/UDP）支持。
- AP_NMEA_Output: NMEA 格式输出支持。
- AP_Notify: 蜂鸣/LED/文本通知工具。
- AP_OpenDroneID: OpenDroneID 功能实现。
- AP_OpticalFlow: 光流传感器支持（定位辅助）。
- AP_Parachute: 伞释放控制逻辑。
- AP_Param: 参数注册/序列化/存储系统（EEPROM/Flash）。
- AP_Proximity: 接近传感器抽象。
- AP_RangeFinder: 距离传感器抽象（LIDAR/SONAR）。
- AP_RCMapper / RC_Channel / AP_RCProtocol: RC 输入映射与协议解析。
- AP_RPM: 转速传感器接口。
- AP_RSSI: RSSI 读取与校准。
- AP_RTC: 实时时钟支持。
- AP_Scheduler: 任务调度器实现（系统核心）。
- AP_Scripting: 脚本运行时（例如 Lua）支持。
- AP_SmartRTL: 智能返航扩展逻辑。
- AP_Terrain: 地形数据库与地形跟随支持。
- AP_Tuning / AP_Stats: 调试统计与调优工具。
- AP_Vehicle: 车辆通用协调层（初始化、调度与生命周期管理）。
  - 参考实现: libraries/AP_Vehicle/AP_Vehicle.cpp
- AP_VideoTX: 视频发射控制支持。
- AP_VisualOdom: 视觉里程计实现。
- Filter / PID / SRV_Channel / SRV_Channel: 滤波器、PID 子系统与伺服输出管理。
- GCS_MAVLink: MAVLink 通信栈与地面站接口。
- StorageManager: 存储管理抽象（参数/日志索引持久化）。
- 其他小库（例如 AP_Rally, AP_Notify, AP_TempCalibration, AP_Winch 等）: 提供具体外设或功能子系统支持。

---

文件已准备好；如果你需要：
- 我把每个库展开为 1–2 段详细说明并加入关键源码链接（例如 `libraries/AP_Vehicle/AP_Vehicle.cpp`）？
- 或者优先生成某些库的深度说明（请列出优先级）？



根据需求-添加
过一下类似驱动
修改
