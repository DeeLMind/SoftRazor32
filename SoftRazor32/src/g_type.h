#pragma once

#ifndef _SR_INC_COMMON_H_
#include "common.h"
#endif

#include "vbstruct.h"

enum SRDBG_TS : signed int
{
  TS_Terminated = -1,
  TS_Running = 0,
  TS_Suspended = 1,
  TS_Wait = 2,
  TS_Unknown = 3
};

enum SRDBG_STATE : unsigned int
{
  DS_Idle,
  DS_Busy
};

enum LOGTYPE : unsigned int
{
  LT_Normal,
  LT_Notice,
  LT_Warning,
  LT_Error
};

typedef int (WINAPI * func_qsym)(int qaddr, wchar_t * iText, size_t maxc);
typedef int (WINAPI * func_logcb)(UINT lIdx, UINT Event, LPARAM lParam);
typedef BOOL CFGB;

typedef union _UNICRC64
{
  ULONG           cNum32[2];
  ULONGLONG       cNum64;
} *PUNICRC64;

//类型定义:

typedef struct _LOG_TYPE
{
  PWCHAR                pText;
  LOGTYPE               LogType;
  UINT                  lState;
  LPARAM                lParam;
  func_logcb            CallBack;
  struct _LOG_TYPE    * Next;
} LOG_TYPE, *PLOG_TYPE;

typedef struct _LV_TYPE         //通用LV类型
{
  int                         Item;
  COLORREF                    ForeColor;
  COLORREF                    BackColor;
} LV_TYPE, *PLV_TYPE;

typedef struct _TV_TYPE         //通用TV类型
{
  DWORD                       Sign;
  WORD                        NCode;      //通知码
  WORD                        Index;      //索引
  HTREEITEM                   hItem;
  DWORD                       lParam;
} TV_TYPE, *PTV_TYPE;

typedef struct _CFPARAM
{
  HWND                        hCtl;
  INT                         SubItem;
} CFPARAM, *PCFPARAM;

typedef struct _MEMORY_LV_TYPE
{
  LV_TYPE                     LV;
  PVOID                       BlockAddress;
} MEMORY_LV_TYPE, *PMEMORY_LV_TYPE;

typedef struct _MODULE_LV_TYPE
{
  LV_TYPE                     LV;
  PVOID                       ModuleAddress;
} MODULE_LV_TYPE, *PMODULE_LV_TYPE;

typedef struct _THREAD_LV_TYPE
{
  LV_TYPE                     LV;
  DWORD                       TID;
  SRDBG_TS                    State;
} THREAD_LV_TYPE, *PTHREAD_LV_TYPE;

typedef struct _APIHOOK_LV_TYPE
{
  LV_TYPE                     LV;
  DWORD                       hType;
  INT_PTR                     oAddr;
  INT_PTR                     cAddr;
  DWORD                       fOffset;
} APIHOOK_LV_TYPE, *PAPIHOOK_LV_TYPE;

typedef struct _VB_LV_TYPE
{
  LV_TYPE                     LV;
} VB_LV_TYPE, *PVB_LV_TYPE;

typedef struct _VB_TV_TYPE
{
  TV_TYPE                     TV;
  LPARAM                      ExParam;
} VB_TV_TYPE, *PVB_TV_TYPE;

typedef struct _CLASS_TABLE
{
  HWND						            hWindow;
  PVOID						            pClass;
  struct _CLASS_TABLE			  * Next;
} CLASS_TABLE, *PCLASS_TABLE;

typedef struct _MEMORY_MODIFY
{
  PBYTE						            BeginAddr;        //开始地址
  PBYTE                       poMem;            //原始内存字节
  UINT						            BlockSize;        //块大小
  WORD                        Flag0;            //标志0
  WORD                        Flag1;            //标志1
  struct _MEMORY_MODIFY		  * Next;             //Next指针
} MEMORY_MODIFY, *PMEMORY_MODIFY;

typedef struct _THREAD_LIST
{
  DWORD                       TID;
  SRDBG_TS                    State;
} THREAD_LIST, *PTHREAD_LIST;

typedef struct _DISASM_ITEM
{
  PVOID						            lpVirtAddr;
  UINT						            uiInstSize;
  WCHAR                       szAddrText[8];
  PCHAR                       lpszInstText;
  UINT                        uiCount;
  PWCHAR                      lpNote;
  struct _DISASM_ITEM         *Next;
} DISASM_ITEM, *PDISASM_ITEM, *PDAI , **PPDAI;

typedef struct _VB_PROC_LIST
{
  PVBCTRL                     pVbCtl;
  WORD                        iIndex;    //事件Idx
  PVBPDI                      ProcDesc;
  PBYTE                       ProcEntry;
  PWCHAR                      EvtName;
  DWORD                       Flag0;
  DWORD                       Flag1;
  struct _VB_PROC_LIST      * Next;
} VB_PROC_LIST, VBPL, *PVBPL;

typedef struct _VB_OBJECT_LIST
{
  PVBHDR                      VbHead;
  PVBPUBOBJ                      VbObj;
  DWORD                       iIndex;   //Obj Idx
  PWCHAR                      ObjName;
  PVBPL                       pl_Head;
  PVBPL                       pl_Last;
  DWORD                       Flag0;
  DWORD                       Flag1;
  struct _VB_OBJECT_LIST    * Next;
} VB_OBJECT_LIST, VBOBJLST, *PVBOBJLST;

typedef struct _PP_PROC_LIST
{
  void                      * pAddr;
  int                         cbidx;
  struct _PP_PROC_LIST      * Next;
  WCHAR                       ProcName[256];
} PP_PROC_LIST , PPLST , *PPPLST;


typedef union _OPDWORD
{
  BYTE      Byte[4];
  WORD      Word[2];
  DWORD     DWord;
} OPDWORD , *POPDWORD;

typedef enum _SYSTEM_INFORMATION_CLASS
{
  SystemBasicInformation = 0,
  SystemCpuInformation = 1,
  SystemPerformanceInformation = 2,
  SystemTimeOfDayInformation = 3, /* was SystemTimeInformation */
  Unknown4,
  SystemProcessInformation = 5,
  Unknown6,
  Unknown7,
  SystemProcessorPerformanceInformation = 8,
  Unknown9,
  Unknown10,
  SystemModuleInformation = 11,
  Unknown12,
  Unknown13,
  Unknown14,
  Unknown15,
  SystemHandleInformation = 16,
  Unknown17,
  SystemPageFileInformation = 18,
  Unknown19,
  Unknown20,
  SystemCacheInformation = 21,
  Unknown22,
  SystemInterruptInformation = 23,
  SystemDpcBehaviourInformation = 24,
  SystemFullMemoryInformation = 25,
  SystemNotImplemented6 = 25,
  SystemLoadImage = 26,
  SystemUnloadImage = 27,
  SystemTimeAdjustmentInformation = 28,
  SystemTimeAdjustment = 28,
  SystemSummaryMemoryInformation = 29,
  SystemNotImplemented7 = 29,
  SystemNextEventIdInformation = 30,
  SystemNotImplemented8 = 30,
  SystemEventIdsInformation = 31,
  SystemCrashDumpInformation = 32,
  SystemExceptionInformation = 33,
  SystemCrashDumpStateInformation = 34,
  SystemKernelDebuggerInformation = 35,
  SystemContextSwitchInformation = 36,
  SystemRegistryQuotaInformation = 37,
  SystemCurrentTimeZoneInformation = 44,
  SystemTimeZoneInformation = 44,
  SystemLookasideInformation = 45,
  SystemSetTimeSlipEvent = 46,
  SystemCreateSession = 47,
  SystemDeleteSession = 48,
  SystemInvalidInfoClass4 = 49,
  SystemRangeStartInformation = 50,
  SystemVerifierInformation = 51,
  SystemAddVerifier = 52,
  SystemSessionProcessesInformation = 53,
  SystemInformationClassMax
} SYSTEM_INFORMATION_CLASS, *PSYSTEM_INFORMATION_CLASS;

