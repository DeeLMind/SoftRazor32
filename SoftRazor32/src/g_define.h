#pragma once

#ifndef _SR_INC_COMMON_H_
#include "common.h"
#endif

//Global Define
#define MY_MAGIC            0x3940A512
#define VB_MAGIC            0x21354256

#define NVE_TEXT            L"����: Soft Razor �޷��ڵ�ǰϵͳ������!\n\n���ϵͳ�汾(NT�汾):\n�ͻ���: Win XP        �����: WinServer 2003"
#define ERR_TITLE           L"����"
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
#define T_REFRESH           L"ˢ��"
#define T_VIEW              L"�鿴"
#define T_ASK               L"ѯ��"
#define T_IGNORE            L"*����*"
#define T_NULL              L"\0\0"
#define T_UNKN1             L"?"
#define T_UNKN2             L"??"
#define T_UNKN4             L"????"
#define T_UNKN8             L"????????"
#define T_UNKNOWN           L"δ֪"
#define T_OTHER             L"����"
#define T_COMMIT            L"�ѷ���"
#define T_RESERVE           L"����"

#define T_FILEHEADER        L"*[Header]*"

#define T_M_IMAGE           L"{ӳ��}"
#define T_M_MAPPED          L"[ӳ��]"
#define T_M_PRIVATE         L"(˽��)"

#define T_P_NOACCESS        L"��ֹ"
#define T_P_READONLY        L"ֻ��"
#define T_P_READWRITE       L"��д"
#define T_P_WRITECOPY       L"������"
#define T_P_EXECUTE         L"��ִ��"
#define T_P_EXECUTE_R       L"ֻ��ִ��"
#define T_P_EXECUTE_RW      L"��дִ��"
#define T_P_EXECUTE_WC      L"����ִ��"
#define T_P_GUARD           L"�����쳣"
#define T_P_NOCACHE         L"��ֹ����"
#define T_P_WRITECOMBINE    L"���д��"

#define T_H_DEFAULT         L"����Ĭ�϶�"
#define T_H_SHARED          L"�����"

#define T_HB_FIXED          L"�ڴ�鲻���ƶ�"
#define T_HB_FREE           L"�ڴ��δʹ��"
#define T_HB_MOVEABLE       L"�ڴ����ƶ�"

#define T_TP_AN             L"���ڱ�׼"
#define T_TP_BN             L"���ڱ�׼"
#define T_TP_HE             L"���"
#define T_TP_IDLE           L"����"
#define T_TP_LE             L"���"
#define T_TP_NM             L"��׼"
#define T_TP_TC             L"ʵʱ"

#define T_TS_RUN            L"��������"
#define T_TS_SP             L"�ѹ���"
#define T_TS_WAIT           L"�ȴ�����"
#define T_TS_TIM            L"����ֹ"

#define T_DBGS_BUSY         L"SoftRazor æµ��,��ȴ����̽���..."

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
#define WN_MODULE           L"ģ���б�"
#define WN_MEMORY           L"�ڴ�ӳ��"
#define WN_HEAP             L"���̶�"
#define WN_THREAD           L"�߳��б�"
#define WN_DISASM           L"�����"
#define WN_VB               L"VB�ṹ"
#define WN_PCPROC           L"P-Code ����"

/* ListView Name */
#define LN_MODULE           L"LV_Process"
#define LN_SECTION          L"LV_Section"
#define LN_MEMORY           L"LV_Memory"
#define LN_HEAP             L"LV_Heap"
#define LN_THREAD           L"LV_Thread"
#define LN_VBLIST           L"LV_VbStruct"

/* TreeView Name */
#define TN_VBTREE           L"TV_VbStruct"

#define SC_EX               L"��ִ��"
#define SC_OR               L"ֻ��"
#define SC_RW               L"��д"
#define SC_SH               L"����"
#define SC_DC               L"�ɶ���"
#define SC_NP               L"����ҳ"
#define SC_RT               L"�ض�λ"
#define SC_ID               L"�ѳ�ʼ������"
#define SC_UD               L"δ��ʼ������"
#define SC_CD               L"����COMDAT"

