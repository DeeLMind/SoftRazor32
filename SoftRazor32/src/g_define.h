#pragma once

#ifndef _SR_INC_COMMON_H_
#include "common.h"
#endif

//Global Define
#define MY_MAGIC            0x3940A512
#define VB_MAGIC            0x21354256

#define NVE_TEXT            L"警告: Soft Razor 无法在当前系统中运行!\n\n最低系统版本(NT版本):\n客户端: Win XP        服务端: WinServer 2003"
#define ERR_TITLE           L"错误"
#define DEF_VCHILD          WS_CHILD | WS_VISIBLE
#define DEF_WINDOW          WS_CHILD | WS_CAPTION | WS_SYSMENU | WS_THICKFRAME | WS_MINIMIZEBOX | WS_MAXIMIZEBOX
#define LV_STYLE            DEF_VCHILD | LVS_REPORT
#define DEF_BTNSTYLE        DEF_VCHILD | BS_USERBUTTON
#define TV_STYLE            DEF_VCHILD | TVS_HASBUTTONS | TVS_LINESATROOT | TVS_HASLINES | TVS_SHOWSELALWAYS
#define MDI_STYLE           DEF_WINDOW | WS_VISIBLE | WS_CLIPSIBLINGS | WS_CLIPCHILDREN

//VB Form
#define VBPC_NULLITEM       0
#define VBPC_STRUCT         1
#define VBPC_OBJECT         2
#define VBPC_OBJINFO        3
#define VBPC_OBJOPTINFO     4
#define VBPC_ROOTFORM       5
#define VBPC_FORM           6
#define VBPC_FRMCTL         7
#define VBPC_BAS            8
#define VBPC_CLS            9
#define VBPC_UC             10
#define VBPC_UCCTL          11

#define VBII_NULLITEM       0
#define VBII_VBHDR          1
#define VBII_PRJI           2
#define VBII_COMD           3
#define VBII_COMI           4
#define VBII_COMISUB        5
#define VBII_OBJTAB         6
#define VBII_OBJECT         7

#define DefWidth            76

//Common
#define HEXFORMAT           L"%08X"
#define FT_TAH              L"Tahoma"
#define FT_VER              L"Verdana"
#define FT_FIX              L"Fixedsys"
#define T_NTDLL             L"ntdll.dll"

#define T_SEPARATOR         L"|"
#define T_REFRESH           L"刷新"
#define T_VIEW              L"查看"
#define T_ASK               L"询问"
#define T_IGNORE            L"*忽略*"
#define T_NULL              L"\0\0"
#define T_UNKN1             L"?"
#define T_UNKN2             L"??"
#define T_UNKN4             L"????"
#define T_UNKN8             L"????????"
#define T_UNKNOWN           L"未知"
#define T_OTHER             L"其它"
#define T_COMMIT            L"已分配"
#define T_RESERVE           L"保留"

#define T_FILEHEADER        L"*[Header]*"

#define T_M_IMAGE           L"{映像}"
#define T_M_MAPPED          L"[映射]"
#define T_M_PRIVATE         L"(私有)"

#define T_P_NOACCESS        L"禁止"
#define T_P_READONLY        L"只读"
#define T_P_READWRITE       L"读写"
#define T_P_WRITECOPY       L"仅共享"
#define T_P_EXECUTE         L"仅执行"
#define T_P_EXECUTE_R       L"只读执行"
#define T_P_EXECUTE_RW      L"读写执行"
#define T_P_EXECUTE_WC      L"共享执行"
#define T_P_GUARD           L"初次异常"
#define T_P_NOCACHE         L"禁止缓存"
#define T_P_WRITECOMBINE    L"组合写入"

#define T_H_DEFAULT         L"进程默认堆"
#define T_H_SHARED          L"共享堆"

#define T_HB_FIXED          L"内存块不可移动"
#define T_HB_FREE           L"内存块未使用"
#define T_HB_MOVEABLE       L"内存块可移动"

#define T_TP_AN             L"高于标准"
#define T_TP_BN             L"低于标准"
#define T_TP_HE             L"最高"
#define T_TP_IDLE           L"空闲"
#define T_TP_LE             L"最低"
#define T_TP_NM             L"标准"
#define T_TP_TC             L"实时"

#define T_TS_RUN            L"正在运行"
#define T_TS_SP             L"已挂起"
#define T_TS_WAIT           L"等待调度"
#define T_TS_TIM            L"已终止"

#define T_DBGS_BUSY         L"SoftRazor 忙碌中,请等待过程结束..."

/* Window Class */
#define WC_MAIN             L"sr_window"
#define WC_APIHOOK          L"sr_apihook"
#define WC_MODULE           L"sr_module"
#define WC_MEMORY           L"sr_memory"
#define WC_HEAP             L"sr_heap"
#define WC_THREAD           L"sr_thread"
#define WC_DISASM           L"sr_disasm"
#define WC_VB               L"sr_vbstruct"
#define WC_PCPROC           L"sr_pcodeproc"


/* Class Name */
#define CN_MDI              L"MDIClient"
#define CN_STATIC           L"Static"
#define CN_EDIT             L"Edit"
#define CN_BUTTON           L"Button"

/* Window Name */
#define WN_MAIN             L"SoftRazor Win32"
#define WN_MODULE           L"模块列表"
#define WN_MEMORY           L"内存映射"
#define WN_HEAP             L"进程堆"
#define WN_THREAD           L"线程列表"
#define WN_DISASM           L"反汇编"
#define WN_VB               L"VB结构"
#define WN_PCPROC           L"P-Code 过程"

/* ListView Name */
#define LN_MODULE           L"LV_Process"
#define LN_SECTION          L"LV_Section"
#define LN_MEMORY           L"LV_Memory"
#define LN_HEAP             L"LV_Heap"
#define LN_THREAD           L"LV_Thread"
#define LN_VBLIST           L"LV_VbStruct"

/* TreeView Name */
#define TN_VBTREE           L"TV_VbStruct"

#define SC_EX               L"可执行"
#define SC_OR               L"只读"
#define SC_RW               L"读写"
#define SC_SH               L"共享"
#define SC_DC               L"可丢弃"
#define SC_NP               L"不分页"
#define SC_RT               L"重定位"
#define SC_ID               L"已初始化数据"
#define SC_UD               L"未初始化数据"
#define SC_CD               L"包含COMDAT"

