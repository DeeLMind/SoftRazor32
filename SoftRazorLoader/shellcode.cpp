#include "stdafx.h"

#pragma code_seg(".shcode")


ASM DWORD WINAPI LoadSoftRazor(LPVOID lpParam)
{
  __asm push ebp
  __asm mov ebp, esp
  __asm push ebx
  __asm call CallMe
CallMe :
  __asm pop ecx
  __asm mov eax, fs : [0x30]
  __asm mov eax, [eax + 0x0C]
  __asm mov eax, [eax + 0x1C]
  __asm mov ebx, [eax + 0x08]   //ebx = ntdll base
  __asm push ecx                //备份ecx
  __asm push ecx
  __asm add dword ptr[esp], 0xF3
  __asm push ebx
  __asm call GetFuncAddr
  __asm test eax, eax
  __asm je RetnFunc
  __asm mov ebx, eax             //ebx = LdrLoadDll
  __asm xor eax, eax
  __asm push eax                //临时变量
  __asm push esp                //临时变量地址
  __asm push lpParam
  __asm push eax
  __asm push eax
  __asm call ebx
  __asm test eax, 0x80000000     //je成功 jne失败
  __asm pop eax                 //取出临时变量 HMODULE
  __asm jne RetnFunc
  __asm add dword ptr[esp], 0xFE
  __asm push eax
  __asm call GetFuncAddr
  __asm test eax, eax
  __asm je RetnFunc
  __asm push 0
  __asm call eax

RetnFunc :
  __asm xor eax, eax
  __asm pop ebx
  __asm mov esp, ebp
  __asm pop ebp
  __asm retn 4
  __asm int 3
  __asm int 3
  __asm int 3
  __asm int 3
  __asm int 3

GetFuncAddr:
  _asm
  {
    //Start
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

  __asm int 3
  __asm int 3
  __asm int 3
  __asm int 3
  __asm int 3

  //FuncName0 :
  __asm _emit 'L'
  __asm _emit 'd'
  __asm _emit 'r'
  __asm _emit 'L'
  __asm _emit 'o'
  __asm _emit 'a'
  __asm _emit 'd'
  __asm _emit 'D'
  __asm _emit 'l'
  __asm _emit 'l'
  __asm _emit 0

  //FuncName1 :
  __asm _emit 'L'
  __asm _emit 'a'
  __asm _emit 'u'
  __asm _emit 'n'
  __asm _emit 'c'
  __asm _emit 'h'
  __asm _emit 'D'
  __asm _emit 'e'
  __asm _emit 'b'
  __asm _emit 'u'
  __asm _emit 'g'
  __asm _emit 'g'
  __asm _emit 'e'
  __asm _emit 'r'
  __asm _emit 0
}


ASM DWORD WINAPI LoadSoftRazor2(LPVOID lpParam)
{
  __asm push ebp
  __asm mov ebp, esp
  __asm push ebx
  __asm call CallMe
CallMe :
  __asm pop ecx
  __asm mov eax, fs : [0x30]
    __asm mov eax, [eax + 0x0C]
    __asm mov eax, [eax + 0x1C]
    __asm mov ebx, [eax + 0x08]   //ebx = ntdll base
    __asm push ecx                //备份ecx
  __asm push ecx
  __asm add dword ptr[esp], 0xF3
  __asm push ebx
  __asm call GetFuncAddr
  __asm test eax, eax
  __asm je RetnFunc
  __asm mov ebx, eax             //ebx = LdrLoadDll
  __asm xor eax, eax
  __asm push eax                //临时变量
  __asm push esp                //临时变量地址
  __asm push lpParam
  __asm push eax
  __asm push eax
  __asm call ebx
  __asm test eax, 0x80000000     //je成功 jne失败
  __asm pop eax                 //取出临时变量 HMODULE
  __asm jne RetnFunc
  __asm add dword ptr[esp], 0xFE
  __asm push eax
  __asm call GetFuncAddr
  __asm test eax, eax
  __asm je RetnFunc
  __asm push 0
  __asm call eax

RetnFunc :
  __asm xor eax, eax
  __asm pop ebx
  __asm mov esp, ebp
  __asm pop ebp
  __asm retn 4
  __asm int 3
  __asm int 3
  __asm int 3
  __asm int 3
  __asm int 3

GetFuncAddr:
  _asm
  {
    //Start
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

  __asm int 3
  __asm int 3
  __asm int 3
  __asm int 3
  __asm int 3

  //FuncName0 :
  __asm _emit 'L'
  __asm _emit 'd'
  __asm _emit 'r'
  __asm _emit 'L'
  __asm _emit 'o'
  __asm _emit 'a'
  __asm _emit 'd'
  __asm _emit 'D'
  __asm _emit 'l'
  __asm _emit 'l'
  __asm _emit 0

  //FuncName1 :
  __asm _emit 'L'
  __asm _emit 'a'
  __asm _emit 'u'
  __asm _emit 'n'
  __asm _emit 'c'
  __asm _emit 'h'
  __asm _emit 'D'
  __asm _emit 'e'
  __asm _emit 'b'
  __asm _emit 'u'
  __asm _emit 'g'
  __asm _emit 'g'
  __asm _emit 'e'
  __asm _emit 'r'
  __asm _emit 0
}

#pragma code_seg()