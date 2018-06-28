#pragma once

typedef void    * POINTER;
typedef POINTER   PDESKTOPINFO;
typedef POINTER   PPROCESSINFO;
typedef POINTER   PTL;
typedef POINTER   PQ;
typedef POINTER   PKL;
typedef POINTER   PCLIENTTHREADINFO;
typedef POINTER   PDESKTOP;
typedef POINTER   PCLIENTINFO;
typedef POINTER   PSMS;
typedef POINTER   PMENUSTATE;
typedef POINTER   PTDB;
typedef POINTER   PWINDOWSTATION;
typedef POINTER   PSVR_INSTANCE_INFO;
typedef POINTER   PMOVESIZEDATA;
typedef POINTER   PSBTRACK;
typedef POINTER   PKEVENT;
typedef POINTER   PWND;
typedef POINTER   PIMC;
typedef POINTER   PQMSG;
typedef POINTER   PCLS;
typedef POINTER   PWOWPROCESSINFO;
typedef POINTER   PDESKTOPVIEW;
typedef POINTER   PCURSOR;
typedef POINTER   PW32JOB;
typedef POINTER   KERNEL_ULONG_PTR;

typedef POINTER   PTHREADINFO;
typedef POINTER   PHOOK;
//#define  WINVER   0x0500
#define CWINHOOKS       (WH_MAX - WH_MIN + 1)
#if(WINVER >= 0x0400)
#define WH_MAX             12
#else
#define WH_MAX             11
#endif
#define WH_MIN              (-1)
#define CLIBS           32

typedef struct _UNICODE_STRING {
  USHORT Length;
  USHORT MaximumLength;
  PWSTR Buffer;
} UNICODE_STRING, *PUNICODE_STRING;

typedef struct tagMLIST {
  PQMSG pqmsgRead;
  PQMSG pqmsgWriteLast;
  DWORD cMsgs;
} MLIST, *PMLIST;

typedef struct _RTL_BITMAP {
  ULONG SizeOfBitMap; // Number of bits in bit map
  PULONG Buffer; // Pointer to the bit map itself
} RTL_BITMAP;
/*
* Private User Startupinfo
*/
typedef struct tagUSERSTARTUPINFO {
  DWORD   cb;
  DWORD   dwX;
  DWORD   dwY;
  DWORD   dwXSize;
  DWORD   dwYSize;
  DWORD   dwFlags;
  WORD    wShowWindow;
  WORD    cbReserved2;
} USERSTARTUPINFO, *PUSERSTARTUPINFO;

/*
*   This structure contains information regarding the
*   thread.  This is viewable from both the client and
*   kernel processes.
*/
typedef struct tagCLIENTTHREADINFO {
  UINT        CTIF_flags;
  WORD        fsChangeBits;           // Bits changes since last compared
  WORD        fsWakeBits;             // Bits currently available
  WORD        fsWakeBitsJournal;      // Bits saved while journalling
  WORD        fsWakeMask;             // Bits looking for when asleep
  LONG        timeLastRead;           // Time of last input read
} CLIENTTHREADINFO;

typedef struct tagTHREADINFO {    // W32THREAD;
  //***************************************** begin: USER specific fields
  PTL             ptl;                // Listhead for thread lock list
  PPROCESSINFO    ppi;                // process info struct for this thread
  PQ              pq;                 // keyboard and mouse input queue
  PKL             spklActive;         // active keyboard layout for this thread
  PCLIENTTHREADINFO pcti;             // Info that must be visible from client
  PDESKTOP        rpdesk;
  PDESKTOPINFO    pDeskInfo;          // Desktop info visible to client
  PCLIENTINFO     pClientInfo;        // Client info stored in TEB
  DWORD           TIF_flags;          // TIF_ flags go here.
  PUNICODE_STRING pstrAppName;        // Application module name.
  PSMS            psmsSent;           // Most recent SMS this thread has sent
  PSMS            psmsCurrent;        // Received SMS this thread is currently processing
  PSMS            psmsReceiveList;    // SMSs to be processed
  LONG            timeLast;           // Time, position, and ID of last message
  ULONG_PTR        idLast;
  int             cQuit;
  int             exitCode;
  HDESK           hdesk;              // Desktop handle
  int             cPaintsReady;
  UINT            cTimersReady;
  PMENUSTATE      pMenuState;
  union {
    PTDB            ptdb;           // Win16Task Schedule data for WOW thread
    PWINDOWSTATION  pwinsta;        // Window station for SYSTEM thread
  };
  PSVR_INSTANCE_INFO psiiList;        // thread DDEML instance list
  DWORD           dwExpWinVer;
  DWORD           dwCompatFlags;      // The Win 3.1 Compat flags
  DWORD           dwCompatFlags2;     // new DWORD to extend compat flags for NT5+ features
  PQ              pqAttach;           // calculation variabled used in
  // zzzAttachThreadInput()
  PTHREADINFO     ptiSibling;         // pointer to sibling thread info
  PMOVESIZEDATA   pmsd;
  DWORD           fsHooks;                // WHF_ Flags for which hooks are installed
  PHOOK           sphkCurrent;            // Hook this thread is currently processing
  PSBTRACK        pSBTrack;
  HANDLE          hEventQueueClient;
  PKEVENT         pEventQueueServer;
  LIST_ENTRY      PtiLink;            // Link to other threads on desktop
  int             iCursorLevel;       // keep track of each thread's level
  POINT           ptLast;
  PWND            spwndDefaultIme;    // Default IME Window for this thread
  PIMC            spDefaultImc;       // Default input context for this thread
  HKL             hklPrev;            // Previous active keyboard layout
  int             cEnterCount;
  MLIST           mlPost;             // posted message list.
  USHORT          fsChangeBitsRemoved;// Bits removed during PeekMessage
  WCHAR           wchInjected;        // character from last VK_PACKET
  DWORD           fsReserveKeys;      // Keys that must be sent to the active
  // active console window.
  PKEVENT        *apEvent;            // Wait array for xxxPollAndWaitForSingleObject
  ACCESS_MASK     amdesk;             // Granted desktop access
  UINT            cWindows;           // Number of windows owned by this thread
  UINT            cVisWindows;        // Number of visible windows on this thread
  PHOOK           aphkStart[CWINHOOKS];   // Hooks registered for this thread
  CLIENTTHREADINFO  cti;              // Use this when no desktop is available
} THREADINFO;

typedef struct tagPROCESSINFO {      //W32PROCESS;
  //***************************************** begin: USER specific fields
  PTHREADINFO     ptiList;                    // threads in this process
  PTHREADINFO     ptiMainThread;              // pti of "main thread"
  PDESKTOP        rpdeskStartup;              // initial desktop
  PCLS            pclsPrivateList;            // this processes' private classes
  PCLS            pclsPublicList;             // this processes' public classes
  PWOWPROCESSINFO pwpi;                       // Wow PerProcess Info
  PPROCESSINFO    ppiNext;                    // next ppi structure in start list
  PPROCESSINFO    ppiNextRunning;
  int             cThreads;                   // count of threads using this process info
  HDESK           hdeskStartup;               // initial desktop handle
  UINT            cSysExpunge;                // sys expunge counter
  DWORD           dwhmodLibLoadedMask;        // bits describing loaded hook dlls
  HANDLE          ahmodLibLoaded[CLIBS];      // process unique hmod array for hook dlls
  PWINDOWSTATION  prpwinsta; // process windowstation
  HWINSTA         hwinsta;                    // windowstation handle
  ACCESS_MASK     amwinsta;                   // windowstation accesses
  DWORD           dwHotkey;                   // hot key from progman
  HMONITOR        hMonitor;                   // monitor handle from CreateProcess
  PDESKTOPVIEW    pdvList;                    // list of desktop views
  UINT            iClipSerialNumber;          // clipboard serial number
  RTL_BITMAP      bmHandleFlags;              // per handle flags
  PCURSOR         pCursorCache;               // process cursor/icon cache
  PVOID           pClientBase;                // LEAVE THIS FOR HYDRA; offset to the shared section
  DWORD           dwLpkEntryPoints;           // user mode language pack installed
  PW32JOB         pW32Job;                    // pointer to the W32JOB structure
  DWORD           dwImeCompatFlags;           // per-process Ime Compatibility flags
  LUID            luidSession;                // logon session id
  USERSTARTUPINFO usi;                        // process startup info
#ifdef VALIDATEHANDLEQUOTA
  LONG lHandles;
#endif
#ifdef USE_MIRRORING
  DWORD           dwLayout;                   // the default Window orientation for this process
#endif
} PROCESSINFO;

typedef struct _HEAD {
  DWORD h; //<----------这里是我们得到的HHOOK
  DWORD cLockObj;
} HEAD, *PHEAD;

typedef struct _THROBJHEAD {
  HEAD hdr;
  DWORD pti;
} THROBJHEAD, *PTHROBJHEAD;

typedef struct _DESKHEAD {
  DWORD rpdesk;
  DWORD pSelf;
} DESKHEAD, *PDESKHEAD;

typedef struct _THRDESKHEAD {
  THROBJHEAD tohdr;
  DESKHEAD dhdr;
} THRDESKHEAD, *PTHRDESKHEAD;

typedef struct tagHOOK {   /* hk */
  THRDESKHEAD     head;
  PHOOK           phkNext;
  int             iHook;              // WH_xxx hook type
  KERNEL_ULONG_PTR offPfn;
  UINT            flags;              // HF_xxx flags
  int             ihmod;
  PTHREADINFO     ptiHooked;          // Thread hooked.
  PDESKTOP        rpdesk;             // Global hook pdesk. Only used when
  //  hook is locked and owner is destroyed
#ifdef HOOKBATCH
  DWORD           cEventMessages;     // Number of events in the cache
  DWORD           iCurrentEvent;      // Current cache event
  DWORD           CacheTimeOut;       // Timeout between keys
  PEVENTMSG       aEventCache;        // The array of Events
#endif // HOOKBATCH
} HOOK;

#define WIN32_LEAN_AND_MEAN
#define WINVER 0x0500
#include <windows.h>

#pragma pack(push,8)

typedef struct _PEB_LDR_DATA
{
  ULONG               Length;
  BOOLEAN             Initialized;
  PVOID               SsHandle;
  LIST_ENTRY          InLoadOrderModuleList;
  LIST_ENTRY          InMemoryOrderModuleList;
  LIST_ENTRY          InInitializationOrderModuleList;
} PEB_LDR_DATA, *PPEB_LDR_DATA;

typedef struct _LDR_MODULE {
  LIST_ENTRY InLoadOrderModuleList;
  LIST_ENTRY InMemoryOrderModuleList;
  LIST_ENTRY InInitializationOrderModuleList;
  PVOID BaseAddress;
  PVOID EntryPoint;
  ULONG SizeOfImage;
  UNICODE_STRING FullDllName;
  UNICODE_STRING BaseDllName;
  ULONG Flags;
  SHORT LoadCount;
  SHORT TlsIndex;
  LIST_ENTRY HashTableEntry;
  ULONG TimeDateStamp;
} LDR_MODULE, *PLDR_MODULE;

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

typedef VOID(_stdcall *PPEBLOCKROUTINE)(PVOID);

typedef struct _PEB_FREE_BLOCK
{
  struct _PEB_FREE_BLOCK* Next;
  ULONG Size;
} PEB_FREE_BLOCK, *PPEB_FREE_BLOCK;

struct PEB
{
  BOOLEAN InheritedAddressSpace;
  BOOLEAN ReadImageFileExecOptions;
  BOOLEAN BeingDebugged;
  BOOLEAN Spare;
  HANDLE Mutant;
  PVOID ImageBaseAddress;
  PPEB_LDR_DATA LoaderData;
  PRTL_USER_PROCESS_PARAMETERS ProcessParameters;
  PVOID SubSystemData;
  PVOID ProcessHeap;
  PVOID FastPebLock;
  PPEBLOCKROUTINE FastPebLockRoutine;
  PPEBLOCKROUTINE FastPebUnlockRoutine;
  ULONG EnvironmentUpdateCount;
  PVOID *KernelCallbackTable;
  PVOID EventLogSection;
  PVOID EventLog;
  PPEB_FREE_BLOCK FreeList;
  ULONG TlsExpansionCounter;
  PVOID TlsBitmap;
  ULONG TlsBitmapBits[0x2];
  PVOID ReadOnlySharedMemoryBase;
  PVOID ReadOnlySharedMemoryHeap;
  PVOID *ReadOnlyStaticServerData;
  PVOID AnsiCodePageData;
  PVOID OemCodePageData;
  PVOID UnicodeCaseTableData;
  ULONG NumberOfProcessors;
  ULONG NtGlobalFlag;
  BYTE Spare2[0x4];
  LARGE_INTEGER CriticalSectionTimeout;
  ULONG HeapSegmentReserve;
  ULONG HeapSegmentCommit;
  ULONG HeapDeCommitTotalFreeThreshold;
  ULONG HeapDeCommitFreeBlockThreshold;
  ULONG NumberOfHeaps;
  ULONG MaximumNumberOfHeaps;
  PVOID **ProcessHeaps;
  PVOID GdiSharedHandleTable;
  PVOID ProcessStarterHelper;
  PVOID GdiDCAttributeList;
  PVOID LoaderLock;
  ULONG OSMajorVersion;
  ULONG OSMinorVersion;
  ULONG OSBuildNumber;
  ULONG OSPlatformId;
  ULONG ImageSubSystem;
  ULONG ImageSubSystemMajorVersion;
  ULONG ImageSubSystemMinorVersion;
  ULONG GdiHandleBuffer[0x22];
  ULONG PostProcessInitRoutine;
  ULONG TlsExpansionBitmap;
  BYTE TlsExpansionBitmapBits[0x80];
  ULONG SessionId;
};

typedef struct _CLIENT_ID
{
  HANDLE UniqueProcess;
  HANDLE UniqueThread;
} CLIENT_ID, *PCLIENT_ID;

typedef struct _GDI_TEB_BATCH
{
  ULONG Offset;
  ULONG HDc;
  ULONG Buffer[0x136];
} GDI_TEB_BATCH, *PGDI_TEB_BATCH;

struct TEB
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
  LONG LastStatusValue;           /* BF4h */
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
};

#pragma pack(pop)