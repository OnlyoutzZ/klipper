# Klipper MCU Commands 使用手册

## 1. 文档说明

本文档整理 Klipper MCU 协议支持命令。

每个命令包含：

-   命令格式
-   功能作用
-   参数说明
-   使用示例
-   测试用途

适用于： - Klipper MCU 移植学习 - 自研 MCU 固件适配 - MCU command
通信测试

# 一、MCU基础管理命令

## allocate_oids

格式：

    allocate_oids count=%c

作用：

申请 MCU 硬件对象 ID。

参数：

-   count：申请对象数量

示例：

    allocate_oids count=4

测试：

初始化 stepper、sensor 等对象前执行。

## identify

格式：

    identify offset=%u count=%c

作用：

读取 MCU identification 信息。

示例：

    identify offset=0 count=32

## finalize_config

格式：

    finalize_config crc=%u

作用：

完成 MCU 配置。

示例：

    finalize_config crc=123456

## get_clock

作用：

获取 MCU 当前时钟。

示例：

    get_clock

## get_config

作用：

读取 MCU 配置信息。

示例：

    get_config

## get_uptime

作用：

读取 MCU 运行时间。

示例：

    get_uptime

## reset

作用：

复位 MCU。

示例：

    reset

## clear_shutdown

作用：

清除 shutdown 状态。

示例：

    clear_shutdown

## emergency_stop

作用：

紧急停止所有运动输出。

示例：

    emergency_stop

# 二、Stepper步进电机模块

## config_stepper

格式：

    config_stepper oid=%c step_pin=%c dir_pin=%c invert_step=%c step_pulse_ticks=%u

作用：

配置步进电机。

参数：

-   oid：对象ID
-   step_pin：STEP引脚
-   dir_pin：方向引脚
-   invert_step：步进电平反转
-   step_pulse_ticks：脉冲宽度

示例：

    allocate_oids count=1

    config_stepper oid=0 step_pin=2 dir_pin=3 invert_step=0 step_pulse_ticks=100

## queue_step

格式：

    queue_step oid=%c interval=%u count=%hu add=%hi

作用：

加入步进脉冲队列。

示例：

    queue_step oid=0 interval=500 count=1000 add=0

## set_next_step_dir

格式：

    set_next_step_dir oid=%c dir=%c

作用：

设置下一次运动方向。

示例：

    set_next_step_dir oid=0 dir=1

## reset_step_clock

格式：

    reset_step_clock oid=%c clock=%u

作用：

同步stepper时间基准。

## stepper_get_position

格式：

    stepper_get_position oid=%c

作用：

读取步进位置。

## stepper_stop_on_trigger

格式：

    stepper_stop_on_trigger oid=%c trsync_oid=%c

作用：

绑定触发停止。

# 三、Endstop限位模块

## config_endstop

格式：

    config_endstop oid=%c pin=%c pull_up=%c

作用：

配置限位输入。

示例：

    config_endstop oid=0 pin=20 pull_up=1

## endstop_home

格式：

    endstop_home oid=%c clock=%u sample_ticks=%u sample_count=%c rest_ticks=%u pin_value=%c trsync_oid=%c trigger_reason=%c

作用：

执行回零检测。

## endstop_query_state

格式：

    endstop_query_state oid=%c

作用：

查询限位状态。

# 四、GPIO输出模块

## config_digital_out

格式：

    config_digital_out oid=%c pin=%u value=%c default_value=%c max_duration=%u

作用：

配置数字输出。

示例：

    config_digital_out oid=0 pin=10 value=0 default_value=0 max_duration=0

## set_digital_out

格式：

    set_digital_out pin=%u value=%c

作用：

立即控制GPIO。

示例：

    set_digital_out pin=10 value=1

## queue_digital_out

格式：

    queue_digital_out oid=%c clock=%u on_ticks=%u

作用：

按照MCU时间控制输出。

## update_digital_out

格式：

    update_digital_out oid=%c value=%c

作用：

更新数字输出。

## set_digital_out_pwm_cycle

格式：

    set_digital_out_pwm_cycle oid=%c cycle_ticks=%u

作用：

设置PWM周期。

# 五、Counter模块

## config_counter

格式：

    config_counter oid=%c pin=%u pull_up=%c

作用：

配置输入计数。

## query_counter

格式：

    query_counter oid=%c clock=%u poll_ticks=%u sample_ticks=%u

作用：

查询计数状态。

# 六、Buttons按键模块

## config_buttons

    config_buttons oid=%c button_count=%c

作用：

配置按键组。

## buttons_add

    buttons_add oid=%c pos=%c pin=%u pull_up=%c

作用：

添加按键。

## buttons_query

    buttons_query oid=%c clock=%u rest_ticks=%u retransmit_count=%c invert=%c

作用：

查询按键状态。

## buttons_ack

    buttons_ack oid=%c count=%c

作用：

确认按键事件。

# 七、显示模块

## HD44780

### config_hd44780

配置LCD。

    config_hd44780 oid=0 rs_pin=1 e_pin=2 d4_pin=3 d5_pin=4 d6_pin=5 d7_pin=6 delay_ticks=10

### hd44780_send_cmds

发送LCD命令。

### hd44780_send_data

发送LCD数据。

## ST7920

### config_st7920

配置ST7920。

### st7920_send_cmds

发送命令。

### st7920_send_data

发送数据。

# 八、NeoPixel模块

## config_neopixel

配置RGB LED。

## neopixel_send

发送RGB数据。

## neopixel_update

格式：

    neopixel_update oid=%c pos=%hu data=%*s

示例：

    neopixel_update oid=0 pos=0 data="\xff0000"

# 九、传感器模块

## HX711

### config_hx71x

配置HX711。

### query_hx71x

读取HX711数据。

### query_hx71x_status

读取HX711状态。

### hx71x_attach_trigger_analog

绑定模拟触发。

# 十、TMC UART模块

## config_tmcuart

配置TMC UART。

## tmcuart_send

格式：

    tmcuart_send oid=%c write=%*s read=%c

示例：

    tmcuart_send oid=0 write="\x05\x00" read=4

# 十一、模拟触发模块

## config_trigger_analog

配置模拟触发。

## trigger_analog_home

执行模拟回零。

## trigger_analog_query_state

查询状态。

## trigger_analog_set_raw_range

设置ADC范围。

## trigger_analog_set_trigger

设置触发条件。

# 十二、TRSYNC同步模块

## config_trsync

配置同步触发对象。

## trsync_start

启动同步。

## trsync_set_timeout

设置超时。

## trsync_trigger

触发同步。

# 十三、SOS Filter滤波模块

## config_sos_filter

配置二阶滤波器。

## sos_filter_set_active

启用滤波。

## sos_filter_set_offset_scale

设置偏移比例。

## sos_filter_set_section

设置滤波器参数。

## sos_filter_set_state

设置滤波状态。

# 十四、Debug命令

## debug_nop

空操作。

## debug_ping

测试通信。

示例：

    debug_ping data=test

## debug_read

读取地址。

示例：

    debug_read order=0 addr=0x40000000

## debug_write

写地址。

示例：

    debug_write order=0 addr=0x40000000 val=1

# 十五、Artificial Commands

## DELAY

延时。

## DUMP

输出调试信息。

## FILEDUMP

文件输出。

## FLOOD

连续发送测试。

## HELP

显示帮助。

## LIST

列出对象。

## SET

设置变量。

## STATS

显示统计。

## SUPPRESS

禁止输出。

# 测试推荐流程

    reset

    ↓

    get_config

    ↓

    debug_ping

    ↓

    allocate_oids

    ↓

    GPIO测试

    ↓

    Stepper测试

    ↓

    Endstop测试

    ↓

    Sensor测试

    ↓

    Display测试

    ↓

    Trigger测试
