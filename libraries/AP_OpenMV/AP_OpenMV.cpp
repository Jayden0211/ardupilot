
/*
   OpenMV library
*/

#define AP_SERIALMANAGER_OPEN_MV_BAUD         115200
#define AP_SERIALMANAGER_OPENMV_BUFSIZE_RX        64
#define AP_SERIALMANAGER_OPENMV_BUFSIZE_TX        64

#include "AP_OpenMV.h"

extern const AP_HAL::HAL& hal;

//constructor 构造函数
AP_OpenMV::AP_OpenMV(void)
{
    _port = NULL;  //串口
    _step = 0;
}

// init - perform require initialisation including detecting which protocol to use
void AP_OpenMV::init(const AP_SerialManager& serial_manager)
{
    // check for DEVO_DPort
    //串口管理器查找 类别为 OPEN_MV 的串口  并且赋值为port  需要在ap_serialmanager中配置好SerialProtocol_OPEN_MV
    if ((_port = serial_manager.find_serial(AP_SerialManager::SerialProtocol_OPEN_MV, 0))) 
    {   //不为空后进行设置   
        _port->set_flow_control(AP_HAL::UARTDriver::FLOW_CONTROL_DISABLE);   //流控制关掉
        // initialise uart
        _port->begin(AP_SERIALMANAGER_OPEN_MV_BAUD, AP_SERIALMANAGER_OPENMV_BUFSIZE_RX, AP_SERIALMANAGER_OPENMV_BUFSIZE_TX);
    }
}

//更新程序
bool AP_OpenMV::update()
{
    if(_port == NULL)   //接口判断是否为空
        return false;

    int16_t numc = _port->available();   //读取串口字节个数
    uint8_t data;                       //一个字符
    uint8_t checksum = 0;

    //解析数据
    for (int16_t i = 0; i < numc; i++) {
        data = _port->read();

        switch(_step) {
        case 0:     //找帧头
            if(data == 0xA5)
                _step = 1;
            break;

        case 1:     //第二个帧头
            if(data == 0x5A)
                _step = 2;
            else
                _step = 0;    //重新找帧头
            break;

        case 2:     //获取cx
            _cx_temp = data;
            _step = 3;
            break;

        case 3:
            _cy_temp = data;
            _step = 4;
            break;

        case 4:
            _step = 0;          //重现找帧头  必须在最后进行
            checksum = _cx_temp + _cy_temp;
            if(checksum == data) {
                cx = _cx_temp;  
                cy = _cy_temp;
                last_frame_ms = AP_HAL::millis();  //记录最后收到帧的时间
                return true;
            }
            break;

        default:
            _step = 0;
        }
    }

    return false;
}
