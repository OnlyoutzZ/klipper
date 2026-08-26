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

# 十六、SPI模块

SPI 总线在固件中运行时枚举（src/n32h7/spi.c 的 DECL_ENUMERATION），枚举名即 printer.cfg 中 `spi_bus` 的取值：

| bus 编号 | 枚举名 | MOSI | MISO | SCK |
|---------|--------|------|------|-----|
| 0 | spi1_PA6_PA7_PA5 | PA7 | PA6 | PA5 |
| 1 | spi2_PB14_PB15_PB13 | PB15 | PB14 | PB13 |
| 2 | spi3_PC11_PC12_PC10 | PC12 | PC11 | PC10 |
| 3 | spi4_PA6_PA7_PA5 | PA7 | PA6 | PA5 |
| 4 | spi5_PF8_PF9_PF7 | PF9 | PF8 | PF7 |
| 5 | spi6_PE13_PE14_PE12 | PE14 | PE13 | PE12 |
| 6 | spi7_PI13_PI14_PI8 | PI14 | PI13 | PI8 |

## config_spi_without_cs

格式：

    config_spi_without_cs oid=%c

作用：

创建 SPI 对象（无 CS 引脚）。回环测试用此命令最简。

前置：必须先执行 `allocate_oids`，否则报 "Can't assign oid" shutdown（oid_count 为 0）。

## config_spi

格式：

    config_spi oid=%c pin=%u cs_active_high=%c

作用：

创建带 CS 引脚的 SPI 对象，传输期间 CS 被拉低/拉高。

注意：

- `pin` 参数是枚举类型，console.py 中填**引脚名**（如 PB13），不能填数字编号（如 29），否则报 "Unknown value '29' in enumeration 'pin'"。底层编号换算：(端口-'A')*16+脚号，如 PB13=1*16+13=29。

## spi_set_bus

格式：

    spi_set_bus oid=%c spi_bus=%u mode=%u rate=%u

作用：

绑定 SPI 总线、模式和波特率。

注意：

- `spi_bus` 必须填枚举名（如 spi1_PA6_PA7_PA5），不能填数字编号（0/1/...），否则报 "Unknown value in enumeration"。
- 每个 oid 只能调用一次 spi_set_bus，再次调用会触发 "Invalid spi config" shutdown。切换 mode/rate 需使用新的 oid。

## spi_transfer

格式：

    spi_transfer oid=%c data=%*s

作用：

全双工传输，并回显收到的数据（回显 `spi_transfer_response`，字节按 b'\x..' 显示）。

注意：

- `data` 为十六进制字符串，不带引号和 \x 转义。如 `data=deadbeef`，不能写 `data="\xde\xad\xbe\xef"`。

## spi_send

格式：

    spi_send oid=%c data=%*s

作用：

只发送不接收。

## SPI 回环测试推荐流程

前置条件：将 MOSI 与 MISO 短接（见上表接线，如 SPI1 短接 PA7↔PA6）。

    reset
    （重开 console）
    allocate_oids count=40
    config_spi_without_cs oid=0
    spi_set_bus oid=0 spi_bus=spi1_PA6_PA7_PA5 mode=0 rate=1000000
    spi_transfer oid=0 data=deadbeef

通过标准：回显 `spi_transfer_response oid=0 response=b'\xde\xad\xbe\xef'`，数据原样弹回。

多 mode/多总线测试要点：

- 每个 mode/rate 组合用新的 oid（allocate_oids count=40 可支持 40 个）。
- 切换总线时按上表重新接线。

## SPI 真实外设测试（W25Q128）

前置条件：硬件接线（以 SPI1 + CS=PB13 为例）。

| W25Q128 引脚 | 接 |
|-------------|-----|
| CS(1) | PB13 |
| DO(2)/MISO | PA6 |
| WP(3) | VCC |
| GND(4) | GND |
| DI(5)/MOSI | PA7 |
| CLK(6) | PA5 |
| HOLD(7) | VCC |
| VCC(8) | 3.3V（勿接 5V） |

流程：

    reset
    （重开 console）
    allocate_oids count=40
    config_spi oid=0 pin=PB13 cs_active_high=0
    spi_set_bus oid=0 spi_bus=spi1_PA6_PA7_PA5 mode=0 rate=1000000
    spi_transfer oid=0 data=9f000000

读 JEDEC ID（0x9F）。通过标准：第 2~4 字节为 EF 40 18（W25Q128JV）或 EF 20 18（W25Q128FV）：

    spi_transfer_response oid=0 response=b'\xff\xef@\x18'

注意：

- `config_spi` 的 `pin` 参数填引脚名（如 PB13），不能填数字编号。
- 读到全 FF = Flash 未响应，优先排查 CS 接线、供电、HOLD#/WP# 是否接 VCC。
- 读 ID 不需要写使能，是最简单的真实外设验证。

### 写 Flash 并读回（完整读写验证）

前置：JEDEC ID 已识别。命令：

    # ① 擦除扇区 0x000000（0x20 扇区擦除，4KB；会清掉该扇区原有数据）
    spi_transfer oid=0 data=06
    spi_transfer oid=0 data=20000000
    spi_transfer oid=0 data=0500     # 轮询 WIP，读到 0x00 即擦除完成

    # ② 确认擦除后为全 FF
    spi_transfer oid=0 data=0300000000000000

    # ③ 写使能 + 编程 deadbeef 到 0x000000
    spi_transfer oid=0 data=06
    spi_transfer oid=0 data=02000000deadbeef
    spi_transfer oid=0 data=0500     # 轮询 WIP

    # ④ 读回
    spi_transfer oid=0 data=0300000000000000

通过标准：

- 第②步读回全 FF（擦除生效）。
- 第④步读回 `b'\xff\xff\xff\xff\xde\xad\xbe\xef'`，写进去再读出来，写路径验证通过。

注意：

- NOR Flash 编程只能 1→0、不能 0→1。目标地址已有数据时必须先擦除（置 FF）再编程，否则读回仍是旧值（按位与结果）。
- 擦除/编程后状态寄存器（0x05）应回到 0x00（WIP=0、WEL=0）。读回全 0 往往是旧数据未擦除，不是驱动问题。
- 擦除会破坏该扇区原数据，测试芯片无妨。

# 测试推荐流程

    reset

    ↓

    get_config

    ↓

    debug_ping

    ↓

    allocate_oids

    ↓

    SPI测试

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
