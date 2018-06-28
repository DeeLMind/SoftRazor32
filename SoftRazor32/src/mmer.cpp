#include "../sr_pch.h"

PMEMORY_MODIFY    MemoryManager::MM_HEADER = NULL;

UINT MemoryManager::GetOriginalMemory(PBYTE MemAddr, PBYTE destAddr, UINT MemAndDestSize)
{
  if (!MemAddr || !destAddr || !MemAndDestSize) return FALSE;
  if ((UINT)destAddr >= (UINT)MemAddr && (UINT)destAddr < ((UINT)MemAddr + MemAndDestSize)) return FALSE;

  memcpy_s(destAddr, MemAndDestSize, MemAddr, MemAndDestSize);

  return TRUE;
}

BOOL MemoryManager::TryModifyMemory(PBYTE destAddr, PBYTE pData, UINT DataSize)
{
  SIZE_T cs;
  return WriteProcessMemory((HANDLE)-1, destAddr, pData, DataSize, &cs);
}