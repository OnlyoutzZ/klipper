
/**
*\*\name   AFIO_ConfigPinAnalogSignalChannelGPIOA.
*\*\fun    Enable or disable the analog signal channel for GPIOA pin.
*\*\param  Pin :
*\*\           - AFIO_ANAEN_PA0_C
*\*\           - AFIO_ANAEN_PA0
*\*\           - AFIO_ANAEN_PA1_C
*\*\           - AFIO_ANAEN_PA1
*\*\           - AFIO_ANAEN_PA2
*\*\           - AFIO_ANAEN_PA3
*\*\           - AFIO_ANAEN_PA4
*\*\           - AFIO_ANAEN_PA5
*\*\           - AFIO_ANAEN_PA6
*\*\           - AFIO_ANAEN_PA7
*\*\param  cmd :
*\*\           - DISABLE
*\*\           - ENABLE
*\*\return none
**/
voidAFIO_ConfigPinAnalogSignalChannelGPIOA(uint32_t Pin, FunctionalState cmd)
{
    uint32_t tempReg;

    tempReg = AFIO->ANAEN_CFG0;

    if (cmd != DISABLE)
    {
        tempReg |= Pin;
    }
    else
    {
        tempReg &= ~(Pin);
    }

    AFIO->ANAEN_CFG0 = tempReg;
}

/**
*\*\name   AFIO_ConfigPinAnalogSignalChannelGPIOB.
*\*\fun    Enable or disable the analog signal channel for GPIOB pin.
*\*\param  Pin :
*\*\           - AFIO_ANAEN_PB0
*\*\           - AFIO_ANAEN_PB1
*\*\param  cmd :
*\*\           - DISABLE
*\*\           - ENABLE
*\*\return none
**/
void AFIO_ConfigPinAnalogSignalChannelGPIOB(uint32_t Pin, FunctionalState cmd)
{
    uint32_t tempReg;

    tempReg = AFIO->ANAEN_CFG1;

    if (cmd != DISABLE)
    {
        tempReg |= Pin;
    }
    else
    {
        tempReg &= ~(Pin);
    }

    AFIO->ANAEN_CFG1 = tempReg;
}
/**
*\*\name   AFIO_ConfigPinAnalogSignalChannelGPIOC.
*\*\fun    Enable or disable the analog signal channel for GPIOC pin.
*\*\param  Pin :
*\*\           - AFIO_ANAEN_PC0
*\*\           - AFIO_ANAEN_PC1
*\*\           - AFIO_ANAEN_PC2
*\*\           - AFIO_ANAEN_PC2_C
*\*\           - AFIO_ANAEN_PC3
*\*\           - AFIO_ANAEN_PC3_C
*\*\           - AFIO_ANAEN_PC4
*\*\           - AFIO_ANAEN_PC5
*\*\           - AFIO_ANAEN_PC6
*\*\           - AFIO_ANAEN_PC8
*\*\           - AFIO_ANAEN_PC13
*\*\param  cmd :
*\*\           - DISABLE
*\*\           - ENABLE
*\*\return none
**/
void AFIO_ConfigPinAnalogSignalChannelGPIOC(uint32_t Pin, FunctionalState cmd)
{
    uint32_t tempReg;

    tempReg = AFIO->ANAEN_CFG2;

    if (cmd != DISABLE)
    {
        tempReg |= Pin;
    }
    else
    {
        tempReg &= ~(Pin);
    }

    AFIO->ANAEN_CFG2 = tempReg;
}

/**
*\*\name   AFIO_ConfigPinAnalogSignalChannelGPIOF.
*\*\fun    Enable or disable the analog signal channel for GPIOF pin.
*\*\param  Pin :
*\*\           - AFIO_ANAEN_PF3
*\*\           - AFIO_ANAEN_PF4
*\*\           - AFIO_ANAEN_PF5
*\*\           - AFIO_ANAEN_PF6
*\*\           - AFIO_ANAEN_PF7
*\*\           - AFIO_ANAEN_PF8
*\*\           - AFIO_ANAEN_PF9
*\*\           - AFIO_ANAEN_PF10
*\*\           - AFIO_ANAEN_PF11
*\*\           - AFIO_ANAEN_PF12
*\*\           - AFIO_ANAEN_PF13
*\*\           - AFIO_ANAEN_PF14
*\*\param  cmd :
*\*\           - DISABLE
*\*\           - ENABLE
*\*\return none
**/
void AFIO_ConfigPinAnalogSignalChannelGPIOF(uint32_t Pin, FunctionalState cmd)
{
    uint32_t tempReg;

    tempReg = AFIO->ANAEN_CFG3;

    if (cmd != DISABLE)
    {
        tempReg |= Pin;
    }
    else
    {
        tempReg &= ~(Pin);
    }

    AFIO->ANAEN_CFG3 = tempReg;
}

/**
*\*\name   AFIO_ConfigPinAnalogSignalChannelGPIOH.
*\*\fun    Enable or disable the analog signal channel for GPIOH pin.
*\*\param  Pin :
*\*\           - AFIO_ANAEN_PH2
*\*\           - AFIO_ANAEN_PH3
*\*\           - AFIO_ANAEN_PH4
*\*\           - AFIO_ANAEN_PH5
*\*\param  cmd :
*\*\           - DISABLE
*\*\           - ENABLE
*\*\return none
**/
void AFIO_ConfigPinAnalogSignalChannelGPIOH(uint32_t Pin, FunctionalState cmd)
{
    uint32_t tempReg;

    tempReg = AFIO->ANAEN_CFG4;

    if (cmd != DISABLE)
    {
        tempReg |= Pin;
    }
    else
    {
        tempReg &= ~(Pin);
    }

    AFIO->ANAEN_CFG4 = tempReg;
}

/**
*\*\name   AFIO_ConfigPinAnalogSignalChannelGPIOI.
*\*\fun    Enable or disable the analog signal channel for GPIOI pin.
*\*\param  Pin :
*\*\           - AFIO_ANAEN_PI8
*\*\           - AFIO_ANAEN_PI15
*\*\param  cmd :
*\*\           - DISABLE
*\*\           - ENABLE
*\*\return none
**/
void AFIO_ConfigPinAnalogSignalChannelGPIOI(uint32_t Pin, FunctionalState cmd)
{
    uint32_t tempReg;

    tempReg = AFIO->ANAEN_CFG5;

    if (cmd != DISABLE)
    {
        tempReg |= Pin;
    }
    else
    {
        tempReg &= ~(Pin);
    }

    AFIO->ANAEN_CFG5 = tempReg;
}

/**
*\*\name   AFIO_ConfigPinAnalogSignalChannelGPIOJ.
*\*\fun    Enable or disable the analog signal channel for GPIOJ pin.
*\*\param  Pin :
*\*\           - AFIO_ANAEN_PJ0
*\*\           - AFIO_ANAEN_PJ3
*\*\           - AFIO_ANAEN_PJ4
*\*\           - AFIO_ANAEN_PJ5
*\*\           - AFIO_ANAEN_PJ6
*\*\           - AFIO_ANAEN_PJ7
*\*\param  cmd :
*\*\           - DISABLE
*\*\           - ENABLE
*\*\return none
**/
void AFIO_ConfigPinAnalogSignalChannelGPIOJ(uint32_t Pin, FunctionalState cmd)
{
    uint32_t tempReg;

    tempReg = AFIO->ANAEN_CFG6;

    if (cmd != DISABLE)
    {
        tempReg |= Pin;
    }
    else
    {
        tempReg &= ~(Pin);
    }

    AFIO->ANAEN_CFG6 = tempReg;
}




### AFIO IO端口模拟信号通道配置寄存器0（AFIO_ANAEN_CFG0）

偏移地址: 0x28

复位值: 0x0000 0000

{bitfield}

|**位域**|**名称**|**属性**|**描述**|
|:^--- 8%|:^--- 21%|:^--- 8%|:--- 63%|
|31:10|Reserved|-|保留，必须保持复位值|
|9|PA7ANAEN|rw|具体描述参考 PA1ANAEN|
|8|PA6ANAEN|rw|具体描述参考 PA1ANAEN|
|7|PA5ANAEN|rw|具体描述参考 PA1ANAEN|
|6|PA4ANAEN|rw|具体描述参考 PA1ANAEN|
|5|PA3ANAEN|rw|具体描述参考 PA1ANAEN|
|4|PA2ANAEN|rw|具体描述参考 PA1ANAEN|
|3|PA1ANAEN|rw|PAx 端口模拟信号通道使能位 (x = 1 … 7)<br>0：禁用端口 PAx 的模拟信号通道<br>1：启用端口 PAx 的模拟信号通道|
|2|PA1_C_ANAEN|rw|PA1\_C 端口模拟信号通道使能位<br>0：禁用 PA1\_C 端口的模拟信号通道<br>1：启用 PA1\_C 端口的模拟信号通道|
|1|PA0ANAEN|rw|PA0端口模拟信号通道使能位<br>0：禁用PA0端口模拟信号通道<br>1：启用PA0端口模拟信号通道|
|0|PA0_C_ANAEN|rw|PA0\_C 端口模拟信号通道使能位<br>0：禁用 PA0\_C 端口的模拟信号通道<br>1：启用 PA0\_C 端口的模拟信号通道|

### AFIO IO端口模拟信号通道配置寄存器1（AFIO_ANAEN_CFG1）

偏移地址: 0x2C

复位值: 0x0000 0000

{bitfield}

|**位域**|**名称**|**属性**|**描述**|
|:^--- 8%|:^--- 21%|:^--- 8%|:--- 63%|
|31:2|Reserved|-|保留，必须保持复位值|
|1|PB1ANAEN|rw|具体描述参考 PB0ANAEN|
|0|PB0ANAEN|rw|PBx端口模拟信号通道使能位 (x = 0 … 1)<br>0：禁用端口PBx的模拟信号通道<br>1：启用端口PBx的模拟信号通道|

### AFIO IO端口模拟信号通道配置寄存器2（AFIO_ANAEN_CFG2）

偏移地址: 0x30

复位值: 0x0000 0000

{bitfield}

|**位域**|**名称**|**属性**|**描述**|
|:^--- 8%|:^--- 21%|:^--- 8%|:--- 63%|
|31:11|Reserved|-|保留，必须保持复位值|
|10|PC13ANAEN|rw|PC13 端口模拟信号通道使能位<br>0：禁用 PC13 端口的模拟信号通道<br>1：启用 PC13 端口的模拟信号通道|
|9|PC8ANAEN|rw|PC8 端口模拟信号通道使能位<br>0：禁用 PC8 端口的模拟信号通道<br>1：启用 PC8 端口的模拟信号通道|
|8|PC6ANAEN|rw|具体描述参考 PC4ANAEN|
|7|PC5ANAEN|rw|具体描述参考 PC4ANAEN|
|6|PC4ANAEN|rw|PCx端口模拟信号通道使能位 (x = 4… 6)<br>0：禁用端口PCx的模拟信号通道<br>1：启用端口PCx的模拟信号通道|
|5|PC3_C_ANAEN|rw|PC3\_C 端口模拟信号通道使能位<br>0：禁用 PC3\_C 端口的模拟信号通道<br>1：启用 PC3\_C 端口的模拟信号通道|
|4|PC3ANAEN|rw|PC3 端口模拟信号通道使能位<br>0：禁用 PC3 端口的模拟信号通道<br>1：启用 PC3 端口的模拟信号通道|
|3|PC2_C_ANAEN|rw|PC2\_C 端口模拟信号通道使能位<br>0：禁用 PC2\_C 端口的模拟信号通道<br>1：启用 PC2\_C 端口的模拟信号通道|
|2|PC2ANAEN|rw|具体描述参考 PC0ANAEN|
|1|PC1ANAEN|rw|具体描述参考 PC0ANAEN|
|0|PC0ANAEN|rw|PCx端口模拟信号通道使能位 (x = 0 … 2)<br>0：禁用端口PCx的模拟信号通道<br>1：启用端口PCx的模拟信号通道|

### AFIO SHRTIM1 故障配置寄存器（AFIO_SHRT1_FALT_CFG）

偏移地址: 0x34

复位值: 0x0000 0000

{bitfield}

|**位域**|**名称**|**属性**|**描述**|
|:^--- 8%|:^--- 21%|:^--- 8%|:--- 63%|
|31:24|Reserved|-|保留，必须保持复位值|
|23:20|SHRT_FALT6[3:0]|rw|具体描述参考 SHRT_FALT1[3:0]|
|19:16|SHRT_FALT5[3:0]|rw|具体描述参考 SHRT_FALT1[3:0]|
|15:12|SHRT_FALT4[3:0]|rw|具体描述参考 SHRT_FALT1[3:0]|
|11:8|SHRT_FALT3[3:0]|rw|具体描述参考 SHRT_FALT1[3:0]|
|7:4|SHRT_FALT2[3:0]|rw|具体描述参考 SHRT_FALT1[3:0]|
|3:0|SHRT_FALT1[3:0]|rw|配置故障通道 SHRTIM1\_FALTx (x=1…6)：<br>0001: PA15<br>0010: PB3<br>0011: PC11<br>0100: PD4<br>0101: PE4<br>0110: PG9<br>0111: PG10<br>1000: PI6<br>1001: PI15<br>1010 : PK2<br>Others: Reserved|

### AFIO SHRTIM2 故障配置寄存器（AFIO_SHRT2_FALT_CFG）

偏移地址: 0x38

复位值: 0x0000 0000

{bitfield}

|**位域**|**名称**|**属性**|**描述**|
|:^--- 8%|:^--- 21%|:^--- 8%|:--- 63%|
|31:24|Reserved|-|保留，必须保持复位值|
|23:20|SHRT_FALT6[3:0]|rw|具体描述参考 SHRT_FALT1[3:0]|
|19:16|SHRT_FALT5[3:0]|rw|具体描述参考 SHRT_FALT1[3:0]|
|15:12|SHRT_FALT4[3:0]|rw|具体描述参考 SHRT_FALT1[3:0]|
|11:8|SHRT_FALT3[3:0]|rw|具体描述参考 SHRT_FALT1[3:0]|
|7:4|SHRT_FALT2[3:0]|rw|具体描述参考 SHRT_FALT1[3:0]|
|3:0|SHRT_FALT1[3:0]|rw|SHRTIM2\_FALTx 配置故障通道（x=1…6）:<br>0001: PC5<br>0010: PD1<br>0011: PD15<br>0100: PF9<br>0101: PF13<br>0110: PG1<br>0111: PI0<br>1000: PI9<br>1001: PI13<br>1010 : PJ0<br>1011 : PK1<br>1100 : PK6<br>Others : Reserved|

### AFIO IO端口模拟信号通道配置寄存器3（AFIO_ANAEN_CFG3）

偏移地址: 0x3C

复位值: 0x0000 0000

{bitfield}

|**位域**|**名称**|**属性**|**描述**|
|:^--- 8%|:^--- 21%|:^--- 8%|:--- 63%|
|31:12|Reserved|-|保留，必须保持复位值|
|11|PF14ANAEN|rw|具体描述参考 PF3ANAEN|
|10|PF13ANAEN|rw|具体描述参考 PF3ANAEN|
|9|PF12ANAEN|rw|具体描述参考 PF3ANAEN|
|8|PF11ANAEN|rw|具体描述参考 PF3ANAEN|
|7|PF10ANAEN|rw|具体描述参考 PF3ANAEN|
|6|PF9ANAEN|rw|具体描述参考 PF3ANAEN|
|5|PF8ANAEN|rw|具体描述参考 PF3ANAEN|
|4|PF7ANAEN|rw|具体描述参考 PF3ANAEN|
|3|PF6ANAEN|rw|具体描述参考 PF3ANAEN|
|2|PF5ANAEN|rw|具体描述参考 PF3ANAEN|
|1|PF4ANAEN|rw|具体描述参考 PF3ANAEN|
|0|PF3ANAEN|rw|PFx 端口模拟信号通道使能位 (x = 3 … 14)<br>0：禁用端口 PFx 的模拟信号通道<br>1：启用端口 PFx 的模拟信号通道|

### AFIO IO端口模拟信号通道配置寄存器4（AFIO_ANAEN_CFG4）

偏移地址: 0x44

复位值: 0x0000 0000

{bitfield}

|**位域**|**名称**|**属性**|**描述**|
|:^--- 8%|:^--- 21%|:^--- 8%|:--- 63%|
|31:4|Reserved|-|保留，必须保持复位值|
|3|PH5ANAEN|rw|具体描述参考 PH2ANAEN|
|2|PH4ANAEN|rw|具体描述参考 PH2ANAEN|
|1|PH3ANAEN|rw|具体描述参考 PH2ANAEN|
|0|PH2ANAEN|rw|PHx端口模拟信号通道使能位 (x = 2 … 5)<br>0：禁用端口PHx的模拟信号通道<br>1：启用端口PHx的模拟信号通道|

### AFIO IO端口模拟信号通道配置寄存器5（AFIO_ANAEN_CFG5）

偏移地址: 0x48

复位值: 0x0000 0000

{bitfield}

|**位域**|**名称**|**属性**|**描述**|
|:^--- 8%|:^--- 21%|:^--- 8%|:--- 63%|
|31:2|Reserved|-|保留，必须保持复位值|
|1|PI15ANAEN|rw|PI15端口模拟信号通道使能位<br>0：禁用PI15端口模拟信号通道<br>1：启用PI15端口模拟信号通道|
|0|PI8ANAEN|rw|PI8端口模拟信号通道使能位<br>0：禁用PI8端口模拟信号通道<br>1：启用PI8端口模拟信号通道|

### AFIO IO端口模拟信号通道配置寄存器6（AFIO_ANAEN_CFG6）

偏移地址: 0x4C

复位值: 0x0000 0000

{bitfield}

|**位域**|**名称**|**属性**|**描述**|
|:^--- 8%|:^--- 21%|:^--- 8%|:--- 63%|
|31:6|Reserved|-|保留，必须保持复位值|
|5|PJ7ANAEN|rw|具体描述参考 PJ3ANAEN|
|4|PJ6ANAEN|rw|具体描述参考 PJ3ANAEN|
|3|PJ5ANAEN|rw|具体描述参考 PJ3ANAEN|
|2|PJ4ANAEN|rw|具体描述参考 PJ3ANAEN|
|1|PJ3ANAEN|rw|PJx端口模拟信号通道使能位 (x = 3 … 7)<br>0：禁用PJx端口的模拟信号通道<br>1：启用PJx端口的模拟信号通道|
|0|PJ0ANAEN|rw|PJ0端口模拟信号通道使能位<br>0：禁用PJ0端口模拟信号通道<br>1：启用PJ0端口模拟信号通道|