#include "../sr_pch.h"
#pragma code_seg(push, ".ehcode")

typedef PVECTORED_LIST(WINAPI * Rtl_AVEH) (_In_ ULONG FirstHandler,
  _In_ PVECTORED_EXCEPTION_HANDLER VectoredHandler);

typedef PVECTORED_LIST(WINAPI * Rtl_AVCH) (_In_ ULONG FirstHandler,
  _In_ PVECTORED_EXCEPTION_HANDLER VectoredHandler);

LONG NTAPI FirstDispatchException(PEXCEPTION_POINTERS ExceptionInfo);
LONG NTAPI LastDispatchException(PEXCEPTION_POINTERS ExceptionInfo);

PVECTORED_EXCEPTION_HANDLER fHandler = FirstExceptionHandler;
PVECTORED_EXCEPTION_HANDLER lHandler = ContinueHandler;
PVECTORED_LIST  fvList = NULL;
PVECTORED_LIST  lvList = NULL;
LPTOP_LEVEL_EXCEPTION_FILTER lef = NULL;
CFGB pvh = TRUE;

LONG NTAPI SR_SetHandler()
{
  INT_PTR NtdllBase;
  Rtl_AVEH AVEH;
  Rtl_AVCH AVCH;
  LONG rval = 0;

  _asm
  {
    mov eax, fs:[0x30]
    mov eax, dword ptr[eax + 0x0C]
    mov eax, dword ptr[eax + 0x1C]
    mov eax, dword ptr[eax + 0x08]   //eax = ntdll base
    mov NtdllBase, eax
  }

  AVEH = (Rtl_AVEH)GetFuncAddr(NtdllBase, "RtlAddVectoredExceptionHandler");
  fvList = AVEH(1, FirstDispatchException);   //注册首次异常分发
  if (fvList) rval |= 1;

  lef = SetUnhandledExceptionFilter(LastDispatchException);

  if (NtVer >= _WIN32_WINNT_WS03)      //支持VCH
  {
    AVCH = (Rtl_AVCH)GetFuncAddr(NtdllBase, "RtlAddVectoredContinueHandler");
    if (AVCH)
    {
      lvList = AVCH(1, ContinueHandler);  //注册继续处理器
      if (lvList) rval |= 2;
    }
  }

  return rval;
}

void NTAPI SR_RemoveHandler()
{
  if (fvList)
  {
    RemoveVectoredExceptionHandler(fvList);
    fvList = NULL;
  }

  SetUnhandledExceptionFilter(lef);
  lef = NULL;

  if (NtVer >= _WIN32_WINNT_WS03)      //支持VCH
  {
    if (lvList)
    {
      RemoveVectoredContinueHandler(lvList);
      lvList = NULL;
    }
  }
}

LONG NTAPI FirstExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  switch (ExceptionInfo->ExceptionRecord->ExceptionCode)
  {
  case EXCEPTION_SINGLE_STEP:           //单步异常
    break;
  case EXCEPTION_ACCESS_VIOLATION:      //虚拟内存地址非法操作
    break;

  }
  return 0;
}

LONG NTAPI ContinueHandler(PEXCEPTION_POINTERS ExceptionInfo)
{
  return 0;
}

#pragma code_seg(".de")

ONLY_ASM LONG NTAPI FirstDispatchException(PEXCEPTION_POINTERS ExceptionInfo)
{
  _asm
  {
    jmp dword ptr[fHandler]
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    xor eax, eax
    retn
  }
}

ONLY_ASM LONG NTAPI LastDispatchException(PEXCEPTION_POINTERS ExceptionInfo)
{
  _asm
  {
    jmp dword ptr[lHandler]
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    mov edi, edi
    xor eax, eax
    retn
  }
}

#pragma code_seg(pop)