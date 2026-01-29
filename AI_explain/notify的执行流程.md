ArduCopter启动
    ↓
init_ardupilot() [system.cpp:17]
    ├─────────┬─────────────────────────────────────────────────────────┐
    ↓         ↓                                                           ↓
notify.init()  其他初始化...                                      之后进入main_loop
    ↓
AP_Notify::init()
    ↓ (调用)
add_backends() [AP_Notify.cpp:286]
    ↓ (遍历配置的_led_type位，创建设备)
for each LED type:
    ├─ ADD_BACKEND(new AP_BoardLED())         // 内置LED
    ├─ ADD_BACKEND(new ToshibaLED_I2C())      // I2C LED
    ├─ ADD_BACKEND(new OreoLED_I2C())         // OreoLED
    ├─ ADD_BACKEND(new NeoPixel())            // NeoPixel
    ├─ ADD_BACKEND(new ProfiLED_SPI())        // SPI LED
    └─ ADD_BACKEND(new XXX())                 // 其他类型LED...
    ↓ (每个backend都会调用)
add_backend_helper() [AP_Notify.cpp:276]
    ├─ _devices[_num_devices] = backend;
    ├─ backend->pNotify = this;               // 绑定AP_Notify指针
    └─ _devices[_num_devices]->init()         // ? 调用LED的硬件初始化


================== Main Loop 主循环 ==================
AP_Vehicle::loop() [AP_Vehicle.cpp:600]
    ↓ (每50ms执行一次，约50Hz)
AP_Vehicle::send_periodic_telemetry() [AP_Vehicle.cpp:618]
    ↓
notify.update() [AP_Vehicle.cpp:620]
    ↓
AP_Notify::update() [AP_Notify.cpp:469]
    ├─ for (each device in _devices[])
    │   └─ _devices[i]->update()  // ? 50Hz调用LED更新
    │       ├─ AP_BoardLED::update()
    │       ├─ ToshibaLED_I2C::update()
    │       ├─ OreoLED_I2C::update()
    │       ├─ NeoPixel::update()
    │       ├─ ProfiLED_SPI::update()
    │       └─ RGBLed::update() [RGBLed.cpp:约135行]
    │           ├─ update_colours()  // 根据flags判断应该显示什么颜色
    │           ├─ set_rgb()         // 设置RGB值到_red/_green/_blue
    │           └─ hw_set_rgb()      // 子类实现，设置硬件颜色
    └─ memset(&AP_Notify::events, 0, sizeof(...))  // 重置事件标志