#pragma once

#ifndef _SR_INC_COMMON_H_
#include "common.h"
#endif

#define STATUS_SUCCESS    ((NTSTATUS)0x000000000L)
#define STATUS_INFO_LENGTH_MISMATCH  ((NTSTATUS)0xC0000004L)

typedef LONG KPRIORITY;
typedef LONG NTSTATUS;

/*TypeDefine*/
typedef enum _THREADINFOCLASS
{
    ThreadBasicInformation,
    ThreadTimes,
    ThreadPriority,
    ThreadBasePriority,
    ThreadAffinityMask,
    ThreadImpersonationToken,
    ThreadDescriptorTableEntry,
    ThreadEnableAlignmentFaultFixup,
    ThreadEventPair_Reusable,
    ThreadQuerySetWin32StartAddress,
    ThreadZeroTlsCell,
    ThreadPerformanceCount,
    ThreadAmILastThread,
    ThreadIdealProcessor,
    ThreadPriorityBoost,
    ThreadSetTlsArrayAddress,
    ThreadIsIoPending,
    ThreadHideFromDebugger,
    ThreadBreakOnTermination,
    ThreadSwitchLegacyState,
    ThreadIsTerminated,
    MaxThreadInfoClass
} THREADINFOCLASS;

typedef enum _THREAD_STATE
{
    StateInitialized,
    StateReady,
    StateRunning,
    StateStandby,
    StateTerminated,
    StateWait,
    StateTransition,
    StateDeferredReady,
    StateUnknown
} THREAD_STATE;

typedef enum _KWAIT_REASON
{
    Executive,
    FreePage,
    PageIn,
    PoolAllocation,
    DelayExecution,
    Suspended,
    UserRequest,
    WrExecutive,
    WrFreePage,
    WrPageIn,
    WrPoolAllocation,
    WrDelayExecution,
    WrSuspended,
    WrUserRequest,
    WrEventPair,
    WrQueue,
    WrLpcReceive,
    WrLpcReply,
    WrVirtualMemory,
    WrPageOut,
    WrRendezvous,
    Spare2,
    Spare3,
    Spare4,
    Spare5,
    Spare6,
    WrKernel
} KWAIT_REASON;

typedef struct _CMP_BYTE
{
    bool        IsValid;
    BYTE        Data;
} CMP_BYTE, *PCMP_BYTE;

typedef struct _UNICODE_STRING
{
    USHORT  Length;
    USHORT  MaximumLength;
    PWSTR  Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct _CLIENT_ID
{
  ULONG UniqueProcess;
  ULONG UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _VM_COUNTERS
{
    ULONG PeakVirtualSize;
    ULONG VirtualSize;
    ULONG PageFaultCount;
    ULONG PeakWorkingSetSize;
    ULONG WorkingSetSize;
    ULONG QuotaPeakPagedPoolUsage;
    ULONG QuotaPagedPoolUsage;
    ULONG QuotaPeakNonPagedPoolUsage;
    ULONG QuotaNonPagedPoolUsage;
    ULONG PagefileUsage;
    ULONG PeakPagefileUsage;
} VM_COUNTERS, *PVM_COUNTERS;

typedef struct _SYSTEM_THREADS
{
    LARGE_INTEGER KernelTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER CreateTime;
    ULONG WaitTime;
    PVOID StartAddress;
    CLIENT_ID ClientId;
    //KPRIORITY Priority;
    LONG Priority;
    //KPRIORITY BasePriority;
    LONG BasePriority;
    ULONG ContextSwitchCount;
    THREAD_STATE dwState;
    //DWORD dwState;
    KWAIT_REASON dwWaitReason;
    //DWORD dwWaitReason;
} SYSTEM_THREADS, *PSYSTEM_THREADS;

typedef struct _THREAD_BASIC_INFORMATION
{
    NTSTATUS ExitStatus;
    struct _TEB * TebBaseAddress;
    CLIENT_ID ClientId;
    ULONG_PTR AffinityMask;
    KPRIORITY Priority;
    LONG BasePriority;
} THREAD_BASIC_INFORMATION;

typedef struct _SYSTEM_PROCESSES
{ // Information Class 5
    ULONG NextEntryDelta;
    ULONG ThreadCount;
    ULONG Reserved1[6];
    LARGE_INTEGER CreateTime;
    LARGE_INTEGER UserTime;
    LARGE_INTEGER KernelTime;
    UNICODE_STRING ProcessName;
    //KPRIORITY BasePriority;
    LONG BasePriority;
    ULONG ProcessId;
    ULONG InheritedFromProcessId;
    ULONG HandleCount;
    ULONG Reserved2[2];
    VM_COUNTERS VmCounters;
    IO_COUNTERS IoCounters; // Windows 2000 only
    SYSTEM_THREADS Threads[1];
} SYSTEM_PROCESSES, *PSYSTEM_PROCESSES;

typedef NTSTATUS (WINAPI *QuerySystemInformation)(
    IN ULONG SysInfoClass,
    IN OUT PVOID SystemInformation,
    IN ULONG SystemInformationLength,
    OUT PULONG nRet);

typedef NTSTATUS (WINAPI *QueryInformationThread)(
    __in HANDLE ThreadHandle,
    __in THREADINFOCLASS ThreadInformationClass,
    __out_bcount(ThreadInformationLength) PVOID ThreadInformation,
    __in ULONG ThreadInformationLength,
    __out_opt PULONG ReturnLength);

typedef struct RTL_DRIVE_LETTER_CURDIR
{
  USHORT              Flags;
  USHORT              Length;
  ULONG               TimeStamp;
  UNICODE_STRING      DosPath;
} RTL_DRIVE_LETTER_CURDIR, *PRTL_DRIVE_LETTER_CURDIR;

typedef struct _RTL_USER_PROCESS_PARAMETERS
{
  ULONG               AllocationSize;
  ULONG               Size;
  ULONG               Flags;
  ULONG               DebugFlags;
  HANDLE              hConsole;
  ULONG               ProcessGroup;
  HANDLE              hStdInput;
  HANDLE              hStdOutput;
  HANDLE              hStdError;
  UNICODE_STRING      CurrentDirectoryName;
  HANDLE              CurrentDirectoryHandle;
  UNICODE_STRING      DllPath;
  UNICODE_STRING      ImagePathName;
  UNICODE_STRING      CommandLine;
  PWSTR               Environment;
  ULONG               dwX;
  ULONG               dwY;
  ULONG               dwXSize;
  ULONG               dwYSize;
  ULONG               dwXCountChars;
  ULONG               dwYCountChars;
  ULONG               dwFillAttribute;
  ULONG               dwFlags;
  ULONG               wShowWindow;
  UNICODE_STRING      WindowTitle;
  UNICODE_STRING      Desktop;
  UNICODE_STRING      ShellInfo;
  UNICODE_STRING      RuntimeInfo;
  RTL_DRIVE_LETTER_CURDIR DLCurrentDirectory[0x20];
} RTL_USER_PROCESS_PARAMETERS, *PRTL_USER_PROCESS_PARAMETERS;

typedef struct _PEB_LDR_DATA
{
  ULONG               Length;
  BOOLEAN             Initialized;
  PVOID               SsHandle;
  LIST_ENTRY          InLoadOrderModuleList;
  LIST_ENTRY          InMemoryOrderModuleList;
  LIST_ENTRY          InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef VOID(_stdcall *PPEBLOCKROUTINE)(PVOID);

typedef struct _PEB_FREE_BLOCK
{
  struct _PEB_FREE_BLOCK* Next;
  ULONG Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

typedef struct _PEB { // Size: 0x1D8

  /*000*/ UCHAR InheritedAddressSpace;

  /*001*/ UCHAR ReadImageFileExecOptions;

  /*002*/ UCHAR BeingDebugged;

  /*003*/ UCHAR SpareBool; // Allocation size

  /*004*/ HANDLE Mutant;

  /*008*/ HINSTANCE ImageBaseAddress; // Instance

  /*00C*/ PPEB_LDR_DATA LoaderData;

  /*010*/ RTL_USER_PROCESS_PARAMETERS *ProcessParameters;

  /*014*/ ULONG SubSystemData;

  /*018*/ HANDLE DefaultHeap;

  /*01C*/ KSPIN_LOCK FastPebLock;

  /*020*/ PPEBLOCKROUTINE FastPebLockRoutine;

  /*024*/ PPEBLOCKROUTINE FastPebUnlockRoutine;

  /*028*/ ULONG EnvironmentUpdateCount;

  /*02C*/ PVOID * KernelCallbackTable;

  /*030*/ LARGE_INTEGER SystemReserved;

  /*038*/ PPEB_FREE_BLOCK FreeList;

  /*03C*/ ULONG TlsExpansionCounter;

  /*040*/ ULONG TlsBitmap;

  /*044*/ LARGE_INTEGER TlsBitmapBits;

  /*04C*/ ULONG ReadOnlySharedMemoryBase;

  /*050*/ ULONG ReadOnlySharedMemoryHeap;

  /*054*/ PVOID * ReadOnlyStaticServerData;

  /*058*/ ULONG AnsiCodePageData;

  /*05C*/ ULONG OemCodePageData;

  /*060*/ ULONG UnicodeCaseTableData;

  /*064*/ ULONG NumberOfProcessors;

  /*068*/ LARGE_INTEGER NtGlobalFlag; // Address of a local copy

  /*070*/ LARGE_INTEGER CriticalSectionTimeout;

  /*078*/ ULONG HeapSegmentReserve;

  /*07C*/ ULONG HeapSegmentCommit;

  /*080*/ ULONG HeapDeCommitTotalFreeThreshold;

  /*084*/ ULONG HeapDeCommitFreeBlockThreshold;

  /*088*/ ULONG NumberOfHeaps;

  /*08C*/ ULONG MaximumNumberOfHeaps;

  /*090*/ PVOID ** ProcessHeaps;

  /*094*/ ULONG GdiSharedHandleTable;

  /*098*/ ULONG ProcessStarterHelper;

  /*09C*/ ULONG GdiDCAttributeList;

  /*0A0*/ KSPIN_LOCK LoaderLock;

  /*0A4*/ ULONG OSMajorVersion;

  /*0A8*/ ULONG OSMinorVersion;

  /*0AC*/ USHORT OSBuildNumber;

  /*0AE*/ USHORT OSCSDVersion;

  /*0B0*/ ULONG OSPlatformId;

  /*0B4*/ ULONG ImageSubsystem;

  /*0B8*/ ULONG ImageSubsystemMajorVersion;

  /*0BC*/ ULONG ImageSubsystemMinorVersion;

  /*0C0*/ ULONG ImageProcessAffinityMask;

  /*0C4*/ ULONG GdiHandleBuffer[0x22];

  /*14C*/ ULONG PostProcessInitRoutine;

  /*150*/ ULONG TlsExpansionBitmap;

  /*154*/ UCHAR TlsExpansionBitmapBits[0x80];

  /*1D4*/ ULONG SessionId;

} PEB, *PPEB;

typedef struct _VECTORED_LIST   //表链结构
{
  struct _VECTORED_LIST       * pNext;//表链的向后指针
  struct _VECTORED_LIST       * pPrevious;  //表链的向前指针
  DWORD                         dwMark;//一个有效标记，
  PVECTORED_EXCEPTION_HANDLER   VectoredFunction;//加密后的异常处理函数指针
} VECTORED_LIST, *PVECTORED_LIST;

typedef struct _VECTORED_LIST_ENTR  //表链入口
{
  DWORD               dwUnknown;  //没弄明白是干啥的
  PVECTORED_LIST      Head;       //记录了VECTORED_LIST表链的第一个
  PVECTORED_LIST      Tail;       //记录了VECTORED_LIST表链的最后一个
} VECTORED_LIST_ENTR, *PVECTORED_LIST_ENTR;

typedef struct _GDI_TEB_BATCH
{
  ULONG Offset;
  ULONG HDc;
  ULONG Buffer[0x136];
} GDI_TEB_BATCH, *PGDI_TEB_BATCH;

typedef struct _TEB
{
  NT_TIB Tib;                         /* 00h */
  PVOID EnvironmentPointer;           /* 1Ch */
  CLIENT_ID Cid;                      /* 20h */
  PVOID ActiveRpcInfo;                /* 28h */
  PVOID ThreadLocalStoragePointer;    /* 2Ch */
  PEB *Peb;                           /* 30h */
  ULONG LastErrorValue;               /* 34h */
  ULONG CountOfOwnedCriticalSections; /* 38h */
  PVOID CsrClientThread;              /* 3Ch */
  void* Win32ThreadInfo; /* 40h */
  ULONG Win32ClientInfo[0x1F];        /* 44h */
  PVOID WOW32Reserved;                /* C0h */
  LCID CurrentLocale;                 /* C4h */
  ULONG FpSoftwareStatusRegister;     /* C8h */
  PVOID SystemReserved1[0x36];        /* CCh */
  PVOID Spare1;                       /* 1A4h */
  LONG ExceptionCode;                 /* 1A8h */
  UCHAR SpareBytes1[0x28];            /* 1ACh */
  PVOID SystemReserved2[0xA];         /* 1D4h */
  GDI_TEB_BATCH GdiTebBatch;          /* 1FCh */
  ULONG gdiRgn;                       /* 6DCh */
  ULONG gdiPen;                       /* 6E0h */
  ULONG gdiBrush;                     /* 6E4h */
  CLIENT_ID RealClientId;             /* 6E8h */
  PVOID GdiCachedProcessHandle;       /* 6F0h */
  ULONG GdiClientPID;                 /* 6F4h */
  ULONG GdiClientTID;                 /* 6F8h */
  PVOID GdiThreadLocaleInfo;          /* 6FCh */
  PVOID UserReserved[5];              /* 700h */
  PVOID glDispatchTable[0x118];       /* 714h */
  ULONG glReserved1[0x1A];            /* B74h */
  PVOID glReserved2;                  /* BDCh */
  PVOID glSectionInfo;                /* BE0h */
  PVOID glSection;                    /* BE4h */
  PVOID glTable;                      /* BE8h */
  PVOID glCurrentRC;                  /* BECh */
  PVOID glContext;                    /* BF0h */
  LONG LastStatusValue;               /* BF4h */
  UNICODE_STRING StaticUnicodeString; /* BF8h */
  WCHAR StaticUnicodeBuffer[0x105];   /* C00h */
  PVOID DeallocationStack;            /* E0Ch */
  PVOID TlsSlots[0x40];               /* E10h */
  LIST_ENTRY TlsLinks;                /* F10h */
  PVOID Vdm;                          /* F18h */
  PVOID ReservedForNtRpc;             /* F1Ch */
  PVOID DbgSsReserved[0x2];           /* F20h */
  ULONG HardErrorDisabled;            /* F28h */
  PVOID Instrumentation[0x10];        /* F2Ch */
  PVOID WinSockData;                  /* F6Ch */
  ULONG GdiBatchCount;                /* F70h */
  USHORT Spare2;                      /* F74h */
  BOOLEAN IsFiber;                    /* F76h */
  UCHAR Spare3;                       /* F77h */
  ULONG Spare4;                       /* F78h */
  ULONG Spare5;                       /* F7Ch */
  PVOID ReservedForOle;               /* F80h */
  ULONG WaitingOnLoaderLock;          /* F84h */
  ULONG Unknown[11];                  /* F88h */
  PVOID FlsSlots;                     /* FB4h */
  PVOID WineDebugInfo;                /* Needed for WINE DLL's  */
} TEB , *PTEB;

/*API Define*/
PVOID WINAPI RtlAcquireSRWLockExclusive(DWORD dwAddress);
PVOID WINAPI RtlReleaseSRWLockExclusive(DWORD dwAddress);