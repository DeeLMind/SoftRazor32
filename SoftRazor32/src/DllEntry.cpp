#include "../sr_pch.h"

UINT StepInlineHook(HMODULE hMod);

BOOL WINAPI DllEntry(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved)
{
  if (dwReason == DLL_PROCESS_ATTACH)
  {
    StepInlineHook((HMODULE)hDllHandle);
    DllBaseAddr = (HMODULE)hDllHandle;
    __asm
    {
      call dword ptr[GetVersion]
      xchg al, ah
      mov NtVer, ax
    }
  }
  return _DllMainCRTStartup(hDllHandle, dwReason, lpreserved);
}

UINT StepInlineHook(HMODULE hMod)
{
  UINT ic = 0;
  UINT i = 0;
  UINT j;
  DWORD OldProtect;
  PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)hMod;

  if (pDos->e_magic != IMAGE_DOS_SIGNATURE) return 0;
  PIMAGE_NT_HEADERS pNtHeader = (PIMAGE_NT_HEADERS)((UINT)hMod + pDos->e_lfanew);
  if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) return 0;
  if (pNtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) return 0;
  if (pNtHeader->FileHeader.SizeOfOptionalHeader < 0xE0U) return 0;
  PIMAGE_OPTIONAL_HEADER32 pOptHead = &pNtHeader->OptionalHeader;
  PIMAGE_IMPORT_DESCRIPTOR pImpDir = (PIMAGE_IMPORT_DESCRIPTOR)((UINT)hMod + pOptHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);
  
  if (!VirtualProtect((PVOID)((UINT)hMod + pOptHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress),
    pOptHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size, PAGE_EXECUTE_READWRITE, &OldProtect)) return 0;

  PIMAGE_THUNK_DATA32 pThunk32;
  POPDWORD pOpcode;

  while (pImpDir[i].OriginalFirstThunk)
  {
    j = 0;

    pThunk32 = (PIMAGE_THUNK_DATA32)((UINT)hMod + pImpDir[i].FirstThunk);
    while (pThunk32[j].u1.Function)
    {
      pOpcode = (POPDWORD)pThunk32[j].u1.Function;

      if (pOpcode->Word[0] == 0xFF8B || (pOpcode->Byte[0] == 0x74 && pOpcode->Byte[2] == 0x55))
      {
        pThunk32[j].u1.Function += 2;
        ic++;
      }
      j++;
    }
    i++;
  }

  VirtualProtect((PVOID)((UINT)hMod + pOptHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].VirtualAddress),
    pOptHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IAT].Size, OldProtect, (DWORD *)&j);
  return ic;
}