#ifndef _INC_WINDOWS
#include <Windows.h>
#endif

#ifndef _STDINT
#include <stdint.h>
#endif

#ifndef UDIS86_H
#include "udis86.h"
#endif

#define Naked       __declspec(naked)

typedef NTSTATUS
(NTAPI * PRTL_HEAP_COMMIT_ROUTINE)(
__in        PVOID Base,
__inout     PVOID *CommitAddress,
__inout     PSIZE_T CommitSize
);

typedef struct _RTL_HEAP_PARAMETERS
{
  ULONG Length;
  SIZE_T SegmentReserve;
  SIZE_T SegmentCommit;
  SIZE_T DeCommitFreeBlockThreshold;
  SIZE_T DeCommitTotalFreeThreshold;
  SIZE_T MaximumAllocationSize;
  SIZE_T VirtualMemoryThreshold;
  SIZE_T InitialCommit;
  SIZE_T InitialReserve;
  PRTL_HEAP_COMMIT_ROUTINE CommitRoutine;
  SIZE_T Reserved[2];
} RTL_HEAP_PARAMETERS, *PRTL_HEAP_PARAMETERS;

typedef PVOID
(NTAPI * PRtlCreateHeap)(
_In_      ULONG Flags,
_In_opt_  PVOID HeapBase,
_In_opt_  SIZE_T ReserveSize,
_In_opt_  SIZE_T CommitSize,
_In_opt_  PVOID Lock,
_In_opt_  PRTL_HEAP_PARAMETERS Parameters
);

typedef PVOID
(NTAPI * PRtlAllocateHeap)(
_In_      PVOID HeapHandle,
_In_opt_  ULONG Flags,
_In_      SIZE_T Size
);

typedef BOOLEAN 
(NTAPI * PRtlFreeHeap)(
_In_      PVOID HeapHandle,
_In_opt_  ULONG Flags,
_In_      PVOID HeapBase
);

typedef PVOID 
(NTAPI * PRtlDestroyHeap)(
_In_      PVOID HeapHandle
);

typedef struct _SC_MODULE_SHADOW
{
  HMODULE             hModule;
  HANDLE              Handle;
  uint32_t            msFlag;
} SC_MODULE_SHADOW, SCMS, *PSCMS;

extern PSCMS WINAPI sc_CreateModuleShadow(HMODULE hMod, uint32_t uiFlag);