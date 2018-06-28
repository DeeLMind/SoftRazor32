#include "SafeCall.h"

#define SCCO_SKIPHOTPATCH       1

#define SCFL_INITED           0x00000001

static int                    sc_flag = 0;
static DWORD                  sc_last = 0;
static PRtlCreateHeap         func_RtlCreateHeap = NULL;
static PRtlAllocateHeap       func_RtlAllocateHeap = NULL;
static PRtlFreeHeap           func_RtlFreeHeap = NULL;
static PRtlDestroyHeap        func_RtlDestroyHeap = NULL;
static HANDLE                 hh_data = NULL;
static HANDLE                 hh_code = NULL;


Naked static PVOID WINAPI GetFuncAddr(HMODULE hModule, LPCSTR lpProcName)
{
  _asm
  {
    push ebp
    mov ebp, esp
    sub esp, 0x10                 //为局部变量开辟空间
    push ebx
    push esi
    push edi
    mov ebx, [ebp + 0x08]
    mov eax, [ebx + 0x3c]         //dosheader->e_lfanew
    mov eax, [ebx + eax + 0x78]    //导出表地址
    test eax, eax                //判断导出表地址是否为空
    je ReturnNull
    add eax, ebx                  //加模块基址
    //取出输出表中一些有用的值   
    mov  ebx, [eax + 0x18]
    mov[ebp - 0x04], ebx
    mov  ebx, [eax + 0x1C]
    add  ebx, [ebp + 0x08]
    mov[ebp - 0x08], ebx
    mov  ebx, [eax + 0x20]
    add  ebx, [ebp + 0x08]
    mov[ebp - 0x0C], ebx
    mov  ebx, [eax + 0x24]
    add  ebx, [ebp + 0x08]
    mov[ebp - 0x10], ebx

    mov esi, [ebp + 0x0C]
    test esi, 0xFFFF0000
    jne Get_API_AddressByName
    mov eax, esi
    dec eax
    jmp Get_API_AddressByIndex

    //函数名取地址
Get_API_AddressByName :
    xor eax, eax
    mov edi, [ebp - 0x0C]
    mov ecx, [ebp - 0x04]
LoopNumberOfName :
    mov esi, [ebp + 0x0C]
    push eax
    mov ebx, [edi]
    add ebx, [ebp + 0x08]
Match_API :
    mov al, byte ptr[ebx]
    cmp al, [esi]
    jnz Not_Match
    or al, 0x00
    jz Get_API_Index_Found
    inc ebx
    inc esi
    jmp Match_API
Not_Match :
    pop eax
    inc eax
    add edi, 0x04
    loop LoopNumberOfName
    jmp ReturnNull

Get_API_Index_Found :
    pop eax

Get_API_AddressByIndex :
    mov ebx, [ebp - 0x10]
    movzx eax, word ptr[ebx + eax * 0x02]
    imul eax, 0x04
    add  eax, [ebp - 0x08]
    mov  eax, [eax]
    add  eax, [ebp + 0x08]
    jmp ReturnVal

ReturnNull :
    xor eax, eax

ReturnVal :
    pop edi
    pop esi
    pop ebx
    add esp, 0x10
    pop ebp
    retn 0x08
  }
}

static PVOID WINAPI sc_GetNtdllProcAddress(LPCSTR lpProcName)
{
  HMODULE hNtdllMod;

  _asm
  {
    mov eax, dword ptr fs : [0x30]
    mov eax, dword ptr ds : [eax + 0x0C]
    mov eax, dword ptr ds : [eax + 0x1C]
    mov eax, dword ptr ds : [eax + 0x08]   //ebx = ntdll base
    mov hNtdllMod, eax
  }

  return GetFuncAddr(hNtdllMod, lpProcName);
}

BOOL WINAPI sc_InitData()
{
  if ((sc_flag & SCFL_INITED) == SCFL_INITED)
    return TRUE;

  if (!func_RtlCreateHeap)
  {
    //初始化堆创建函数地址
    func_RtlCreateHeap = (PRtlCreateHeap)sc_GetNtdllProcAddress("RtlCreateHeap");

    if (!func_RtlCreateHeap)
      return FALSE;

#if SCCO_SKIPHOTPATCH
    if (*(WORD *)func_RtlCreateHeap == 0xFF8B)
      func_RtlCreateHeap = (PRtlCreateHeap)((uint32_t)func_RtlCreateHeap + 2);
#endif
  }

  if (!func_RtlAllocateHeap)
  {
    //初始化堆申请函数地址
    func_RtlAllocateHeap = (PRtlAllocateHeap)sc_GetNtdllProcAddress("RtlAllocateHeap");

    if (!func_RtlAllocateHeap)
      return FALSE;

#if SCCO_SKIPHOTPATCH
    if (*(WORD *)func_RtlAllocateHeap == 0xFF8B)
      func_RtlAllocateHeap = (PRtlAllocateHeap)((uint32_t)func_RtlAllocateHeap + 2);
#endif
  }

  if (!func_RtlFreeHeap)
  {
    //初始化堆释放函数地址
    func_RtlFreeHeap = (PRtlFreeHeap)sc_GetNtdllProcAddress("RtlFreeHeap");

    if (!func_RtlFreeHeap)
      return FALSE;

#if SCCO_SKIPHOTPATCH
    if (*(WORD *)func_RtlFreeHeap == 0xFF8B)
      func_RtlFreeHeap = (PRtlFreeHeap)((uint32_t)func_RtlFreeHeap + 2);
#endif
  }

  if (!func_RtlDestroyHeap)
  {
    //初始化堆销毁函数地址
    func_RtlDestroyHeap = (PRtlDestroyHeap)sc_GetNtdllProcAddress("RtlDestroyHeap");

    if (!func_RtlDestroyHeap)
    return FALSE;

#if SCCO_SKIPHOTPATCH
    if (*(WORD *)func_RtlDestroyHeap == 0xFF8B)
      func_RtlDestroyHeap = (PRtlDestroyHeap)((uint32_t)func_RtlDestroyHeap + 2);
#endif
  }

  if (!hh_data) 
  {
    //创建数据堆
    hh_data = func_RtlCreateHeap(HEAP_GROWABLE, NULL, 0, 524288, NULL, NULL);

    if (!hh_data)
      return FALSE;
  }

  if (!hh_code)
  {
    //创建代码堆
    hh_code = func_RtlCreateHeap(HEAP_CREATE_ENABLE_EXECUTE | HEAP_GROWABLE, NULL, 0, 262144, NULL, NULL);

    if (!hh_code)
      return FALSE;
  }

  sc_flag |= SCFL_INITED;
  return TRUE;
}

PSCMS WINAPI sc_CreateModuleShadow(HMODULE hMod, uint32_t uiFlag)
{
  PSCMS mstmp;

  if (((sc_flag & SCFL_INITED) == 0) && !sc_InitData())
  {
    sc_last = 1;
    return NULL;
  }

  mstmp = (PSCMS)func_RtlAllocateHeap(hh_data, HEAP_ZERO_MEMORY, sizeof(SCMS));

}

