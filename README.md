**任务描述：请基于ZigBee模块（黑板）实现可编程霓虹灯显示系统。系统通过按键SW1设置霓虹灯的工作、停止2种模式，按照串口助手下发的指令进行霓虹灯显示。**

**设备列表：**

1、ZigBee模块（黑板）1块+电源线

2、USB转串口线1根

3、CC Debugger仿真器1个

**任务要求：**

用USB转串口线将ZigBee模块（黑板）连接至PC机的串口。

在 “C:\\JoyWork\\01\\work\\Task\\”文件夹下创建IAR工程，工程名字和工作空间的名字均为：“Task+准考证号后12位”，把“Task” 文件夹下的“test.c”**添加到IAR工程中**，配置好工程选项参数，确保工程编译成功。（4分）。在“test.c”中添加代码实现以下功能（注意：题目中涉及到的变量均已在test.c中做了定义）：

1、系统有两种工作模式：霓虹灯工作模式和霓虹灯停止模式。

1）通过按键SW1切换workMode的值来改变工作模式。

2）workMode的值为0时是停止模式，为 1时是工作模式，默认是停止模式。

**2、通过**串口助手发送控制指令来控制霓虹灯的闪烁效果，指令格式说明如下（串口助手发送与接收均设置成HEX格式）：

发送指令： F8 01 03 07 0F 08 0C 0D 0E
