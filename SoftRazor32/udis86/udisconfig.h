#pragma once
#define __UD_STANDALONE__

/*定义指令类型*/
#define UDCA_Null             0x0000    //无
#define UDCA_Ignore           0x0001    //被忽略的
#define UDCA_Inst             0x0002    //一般指令 (mov add xor)
#define UDCA_StackOp          0x0003    //栈操作指令 (push pop)
#define UDCA_FMSOp            0x0004    //FPU MMX SSE 寄存器相关指令
#define UDCA_CJump            0x0005    //条件跳转
#define UDCA_Jump             0x0006    //跳转
#define UDCA_Call             0x0007    //调用
#define UDCA_Ret              0x0008    //返回
#define UDCA_PrivInst         0x0009    //特权指令
#define UDCA_Prefix           0x000A    //前缀
#define UDCA_DefClr           0x000B    //默认颜色
#define UDCA_Int              0x000C    //INT中断
#define UDCA_Data             0x000D    //数据
#define UDCA_Text             0x000E    //文本
#define UDCA_Symbol           0x000F    //符号名称
#define UDCA_Error            0x0010    //非法命令、错误指令
#define UDCA_GPRs             0x0011    //通用寄存器
#define UDCA_FSRs             0x0012    //FPU SSE 寄存器
#define UDCA_SSRs             0x0013    //段、系统 寄存器
#define UDCA_MStack           0x0014    //栈内存地址
#define UDCA_MOther           0x0015    //其它内存地址
#define UDCA_IMM              0x0016    //立即数
#define UDCA_IMMAddr          0x0017    //地址立即数
#define UDCA_Change           0x0020    //被修改的