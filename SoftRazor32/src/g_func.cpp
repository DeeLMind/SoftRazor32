#include "../sr_pch.h"

typedef int (WINAPI * func_LCIDToLocaleName) (
__in LCID     Locale,
__out_ecount_opt(cchName) LPWSTR  lpName,
__in int      cchName,
__in DWORD    dwFlags);

ONLY_ASM void TestFunc(DWORD IP)
{
  __asm
  {
    push ebp
      mov ebp, esp
      mov eax, [GetVersion]
      mov word ptr ds : [0x00401000], ax
      mov eax, [ecx]
      test eax, eax
      cmp eax, 0
      //je 0x00401000
  }
}

inline DWORD TestExpect(DWORD ExpectCode)
{
  return (ExpectCode == EXCEPTION_ACCESS_VIOLATION) ? EXCEPTION_EXECUTE_HANDLER : EXCEPTION_CONTINUE_SEARCH;
}

int If_LCIDToLocaleName(LCID Locale, LPWSTR lpName, int cchName, DWORD dwFlags)
{
  if (NtVer >= _WIN32_WINNT_VISTA)
  {
    func_LCIDToLocaleName func_LCIDTLN;

    func_LCIDTLN = (func_LCIDToLocaleName)GetProcAddress(GetModuleHandle(L"kernel32.dll"), "LCIDToLocaleName");
    if (func_LCIDTLN == NULL) return 0;
    if (NtVer < _WIN32_WINNT_WIN7) dwFlags = 0;

    return func_LCIDTLN(Locale, lpName, cchName, dwFlags);
  }
  return 0;
}

BOOL TryCopyWCharFromWChar(PWCHAR psDest, size_t dwMax, PWCHAR psSrc)
{
  if (psDest == NULL || dwMax == 0 || psSrc == NULL) return FALSE;

  __try
  {
    wcscpy_s(psDest, dwMax, psSrc);
    return TRUE;
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    return FALSE;
  }
}

BOOL TryCopyWCharFromChar(PWCHAR psDest, size_t dwMax, PCHAR psSrc)
{
  if (psDest == NULL || dwMax == 0 || psSrc == NULL) return FALSE;

  __try
  {
    mbstowcs_s(NULL, psDest, dwMax, psSrc, _TRUNCATE);
    return TRUE;
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    return FALSE;
  }
}

BOOL TryCatenateWCharFromWChar(PWCHAR psDest, size_t dwMax, PWCHAR psSrc)
{
  if (psDest == NULL || dwMax == 0 || psSrc == NULL) return FALSE;

  __try
  {
    wcscat_s(psDest, dwMax, psSrc);
    return TRUE;
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    return FALSE;
  }
}

BOOL TryCatenateWCharFromChar(PWCHAR psDest, size_t dwMax, PCHAR psSrc)
{
  if (psDest == NULL || dwMax == 0 || psSrc == NULL) return FALSE;
  PWCHAR pTmp = (PWCHAR)malloc(dwMax * sizeof(WCHAR));
  if (pTmp == NULL) return FALSE;

  __try
  {
    __try
    {
      pTmp[0] = 0;
      mbstowcs_s(NULL, pTmp, dwMax, psSrc, _TRUNCATE);
      wcscat_s(psDest, dwMax, pTmp);
      return TRUE;
    }
    __except (TEXPECT_NOACCESS(GetExceptionCode()))
    {
      return FALSE;
    }
  }
  __finally
  {
    free(pTmp);
  }
}

BOOL GetModuleName(PVOID dwAddr, PWCHAR oBuffer, UINT sMax)
{
  PWCHAR pos;
  MEMORY_BASIC_INFORMATION MBI;
  WCHAR stmp[MAX_PATH];

  if (!VirtualQuery(dwAddr, &MBI, sizeof(MEMORY_BASIC_INFORMATION))) return FALSE;
  if (MBI.State != MEM_COMMIT) return FALSE;

  if (!GetModuleFileName((HMODULE)MBI.AllocationBase, stmp, MAX_PATH)) return FALSE;
  pos = wcsrchr(stmp, L'\\');

  if (pos)
  {
    pos++;
    wcscpy_s(oBuffer, sMax, pos);
  }
  else
    wcscpy_s(oBuffer, sMax, stmp);

  return TRUE;
}

BOOL CheckPEFormatByBuffer(PBYTE pBuffer, DWORD bMax, DWORD cFlag)
{
  __try
  {
    if (!pBuffer || !bMax) return FALSE;

    PIMAGE_DOS_HEADER pDosHdr = PIMAGE_DOS_HEADER(&pBuffer);
    DWORD cSize;
    PDWORD pSign;
    PIMAGE_FILE_HEADER pFileHdr;
    PIMAGE_SECTION_HEADER pSection;
    WORD idx;

    if (pDosHdr->e_magic != IMAGE_DOS_SIGNATURE) return FALSE;
    cSize = pDosHdr->e_lfanew + 4;
    if (cSize >= bMax) return FALSE;
    pSign = PDWORD(&pBuffer[pDosHdr->e_lfanew]);
    if (*pSign != IMAGE_NT_SIGNATURE) return FALSE;

    pFileHdr = PIMAGE_FILE_HEADER(&pBuffer[pDosHdr->e_lfanew]);

    cSize += IMAGE_SIZEOF_FILE_HEADER + pFileHdr->SizeOfOptionalHeader;
    if (cSize >= bMax) return FALSE;

    if (!CHK_FLAG(cFlag, CHKPE_NOCHKOPTHDR))     //未开启不检查可选头
    {
      if (cSize >= bMax)
        return FALSE;
    }

    pSection = PIMAGE_SECTION_HEADER(&pBuffer[cSize]);
    cSize += pFileHdr->NumberOfSections * IMAGE_SIZEOF_SECTION_HEADER;

    if (!CHK_FLAG(cFlag, CHKPE_NOCHKSECTION))    //未开启不检查区段
    {
      if (cSize >= bMax)
        return FALSE;

      /* 检查区段大小 */
      for (idx = 0; idx < pFileHdr->NumberOfSections; idx++)
      {
        if (pSection[idx].PointerToRawData + pSection[idx].SizeOfRawData > bMax)
          return FALSE;
      }
    }

    return TRUE;
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    return FALSE;
  }
}

BOOL HitModule(PVOID pAddr, PUINT pBase, PUINT pSize)
{
  MEMORY_BASIC_INFORMATION MBI;

  if (!VirtualQuery(pAddr, &MBI, sizeof(MEMORY_BASIC_INFORMATION))) return FALSE;
  if (MBI.State != MEM_COMMIT) return FALSE;
  if (MBI.Type != MEM_IMAGE) return FALSE;

  if (pBase != NULL)
    *pBase = (UINT)MBI.AllocationBase;

  if (pSize != NULL)
  {
    PIMAGE_NT_HEADERS32 pNtHeader;

    pNtHeader = (PIMAGE_NT_HEADERS32)
      ((UINT)MBI.AllocationBase + ((PIMAGE_DOS_HEADER)MBI.AllocationBase)->e_lfanew);

    if (pNtHeader->FileHeader.SizeOfOptionalHeader < 0x3C &&
      pNtHeader->FileHeader.SizeOfOptionalHeader > 0xE0) return FALSE;

    *pSize = pNtHeader->OptionalHeader.SizeOfImage;
  }

  return TRUE;
}

INT_PTR GetModuleBaseAddr(PVOID pAddr)
{
  MEMORY_BASIC_INFORMATION MBI;

  if (!VirtualQuery(pAddr, &MBI, sizeof(MEMORY_BASIC_INFORMATION))) return NULL;
  if (MBI.State != MEM_COMMIT) return NULL;
  if (MBI.Type != MEM_IMAGE) return NULL;
  return (INT_PTR)MBI.AllocationBase;
}

BOOL GetThreadDescriptor(HANDLE hThread, DWORD dwSelector, PDWORD pBase, PDWORD pLimit, PBOOL Is4KB)
{
  if (!hThread || !pBase || !pLimit) return FALSE;

  LDT_ENTRY ldte;

  if (!GetThreadSelectorEntry(hThread, dwSelector, &ldte)) return FALSE;

  *pBase = DWORD(ldte.HighWord.Bytes.BaseHi << 24 | ldte.HighWord.Bytes.BaseMid << 16 | ldte.BaseLow);
  *pLimit = DWORD(ldte.HighWord.Bits.LimitHi << 16 | ldte.LimitLow);

  if (ldte.HighWord.Bits.Granularity) *Is4KB = TRUE;
  else *Is4KB = FALSE;

  return TRUE;
}

UINT GetModuleSize(HMODULE hMod, BOOL FormImageSize)
{
  __try
  {
    if (FormImageSize)
    {
      PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hMod;
      PIMAGE_NT_HEADERS32 pNtHeader;

      pNtHeader = (PIMAGE_NT_HEADERS32)((UINT)hMod + pDosHeader->e_lfanew);
      if (pNtHeader->FileHeader.SizeOfOptionalHeader >= 0x3C &&
        pNtHeader->FileHeader.SizeOfOptionalHeader <= 0xE0)
        return pNtHeader->OptionalHeader.SizeOfImage;
      else
        return 0;
    }
    else
    {
      PVOID AllocBase = (PVOID)hMod;
      PVOID CurBase = (PVOID)hMod;
      UINT cSize = 0;
      MEMORY_BASIC_INFORMATION MBI;

      while (VirtualQuery(CurBase, &MBI, sizeof(MEMORY_BASIC_INFORMATION)))
      {
        if (MBI.AllocationBase != AllocBase) return cSize;
        cSize += MBI.RegionSize;
        CurBase = (PVOID)((UINT)CurBase + MBI.RegionSize);
      }
      return 0;

    }
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    return 0;
  }
}

UINT GetModuleEntry(HMODULE hMod)
{
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hMod;
  PIMAGE_NT_HEADERS32 pNtHeader;

  __try
  {
    pNtHeader = (PIMAGE_NT_HEADERS32)((UINT)hMod + pDosHeader->e_lfanew);
    return (UINT)hMod + pNtHeader->OptionalHeader.AddressOfEntryPoint;
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    return 0;
  }
}

PVBHDR GetVbHeaderByModule(HMODULE hMod)
{
  PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)hMod;
  if (pDos->e_magic != IMAGE_DOS_SIGNATURE) return NULL;
  PIMAGE_NT_HEADERS pNt = (PIMAGE_NT_HEADERS)((DWORD)hMod + pDos->e_lfanew);
  PBYTE OEP = (PBYTE)((DWORD)hMod + pNt->OptionalHeader.AddressOfEntryPoint);
  PVBHDR VBH = (PVBHDR)*(PDWORD)((DWORD)OEP + 1);

  if (OEP[0] != 0x68 || OEP[5] != 0xE8) return NULL;
  if (VBH->Sign.dwSign != VB_MAGIC) return NULL;
  return VBH;
}

PIMAGE_SECTION_HEADER GetSectionsAddress(HMODULE hMod)
{
  if (!hMod) return NULL;

  PIMAGE_DOS_HEADER pDos;
  PIMAGE_NT_HEADERS32 pNtHeader;

  pDos = (PIMAGE_DOS_HEADER)hMod;
  if (pDos->e_magic != IMAGE_DOS_SIGNATURE) return NULL;
  pNtHeader = (PIMAGE_NT_HEADERS32)((UINT)hMod + pDos->e_lfanew);
  if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) return NULL;
  return (PIMAGE_SECTION_HEADER)((UINT)pNtHeader + 0x18 + pNtHeader->FileHeader.SizeOfOptionalHeader);
}

WORD GetSectionNumber(HMODULE hMod)
{
  if (!hMod) return NULL;

  PIMAGE_DOS_HEADER pDos;
  PIMAGE_NT_HEADERS32 pNtHeader;

  pDos = (PIMAGE_DOS_HEADER)hMod;
  if (pDos->e_magic != IMAGE_DOS_SIGNATURE) return NULL;
  pNtHeader = (PIMAGE_NT_HEADERS32)((UINT)hMod + pDos->e_lfanew);
  if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) return NULL;
  return pNtHeader->FileHeader.NumberOfSections;
}

PVOID GetSectionAddrByName(HMODULE hMod, PCHAR pSecName, DWORD * pSecSize)
{
  if (!hMod || !pSecName) return NULL;

  PIMAGE_DOS_HEADER pDos;
  PIMAGE_NT_HEADERS32 pNtHeader;
  PIMAGE_SECTION_HEADER pSection;
  WORD sNum;
  WORD i;

  pDos = (PIMAGE_DOS_HEADER)hMod;
  if (pDos->e_magic != IMAGE_DOS_SIGNATURE) return NULL;
  pNtHeader = (PIMAGE_NT_HEADERS32)((UINT)hMod + pDos->e_lfanew);
  if (pNtHeader->Signature != IMAGE_NT_SIGNATURE) return NULL;
  pSection = (PIMAGE_SECTION_HEADER)((UINT)pNtHeader + 0x18 + pNtHeader->FileHeader.SizeOfOptionalHeader);
  sNum = pNtHeader->FileHeader.NumberOfSections;

  for (i = 0; i < sNum; i++)
  {
    if (strcmp((PCHAR)pSection[i].Name, pSecName) == 0)
    {
      *pSecSize = pSection[i].Misc.VirtualSize;
      return (PVOID)((UINT)hMod + pSection[i].VirtualAddress);
    }
  }
  return NULL;
}

BOOL GetSectionBaseAddr(DWORD dwAddr, PDWORD opSectAddr, PDWORD opSectSize)
{
  PIMAGE_DOS_HEADER pDosHeader;
  PIMAGE_NT_HEADERS32 pNtHeader;
  PIMAGE_SECTION_HEADER pSection;
  DWORD SVA;
  MEMORY_BASIC_INFORMATION MBI;
  WORD sNum;

  __try
  {
    if (!VirtualQuery((LPVOID)dwAddr, &MBI, sizeof(MEMORY_BASIC_INFORMATION))) return FALSE;
    pDosHeader = (PIMAGE_DOS_HEADER)MBI.AllocationBase;
    pNtHeader = (PIMAGE_NT_HEADERS32)((DWORD)MBI.AllocationBase + pDosHeader->e_lfanew);
    sNum = pNtHeader->FileHeader.NumberOfSections;
    pSection = (PIMAGE_SECTION_HEADER)((DWORD)pNtHeader + 24 + pNtHeader->FileHeader.SizeOfOptionalHeader);

    while (sNum)
    {
      /* 取区段基地址 */
      SVA = (DWORD)MBI.AllocationBase + pSection->VirtualAddress;

      if (dwAddr >= SVA && (dwAddr < SVA + pSection->Misc.VirtualSize))
      {
        *opSectAddr = SVA;
        *opSectSize = pSection->Misc.VirtualSize;
        return TRUE;
      }
      sNum--;
      pSection++;
    }

    *opSectAddr = (DWORD)MBI.BaseAddress;
    *opSectSize = (DWORD)MBI.RegionSize;
    return TRUE;
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    return FALSE;
  }
}

BOOL GetSectionNameA(HMODULE hMod, PVOID pSect, PCHAR OutName, rsize_t sMax)
{
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hMod;
  PIMAGE_NT_HEADERS32 pNtHeader;
  PIMAGE_SECTION_HEADER pSection;
  DWORD SVA;
  WORD sNum;
  
  __try
  {
    if (!hMod || !pSect || !OutName) return FALSE;

    pNtHeader = (PIMAGE_NT_HEADERS32)((DWORD)hMod + pDosHeader->e_lfanew);
    pSection = (PIMAGE_SECTION_HEADER)((DWORD)pNtHeader + 24 + pNtHeader->FileHeader.SizeOfOptionalHeader);
    sNum = pNtHeader->FileHeader.NumberOfSections;

    while (sNum)
    {
      SVA = (DWORD)hMod + pSection->VirtualAddress;
      if (((DWORD)pSect >= SVA) && ((DWORD)pSect < SVA + pSection->Misc.VirtualSize))
      {
        strncpy_s(OutName, sMax, (const char *)pSection->Name, 8);
        return TRUE;
      }
      sNum--;
      pSection++;
    }
    return FALSE;
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    return FALSE;
  }
}

BOOL GetSectionNameW(HMODULE hMod, PVOID pSect, PWCHAR OutName, rsize_t sMax)
{
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hMod;
  PIMAGE_NT_HEADERS32 pNtHeader;
  PIMAGE_SECTION_HEADER pSection;
  size_t blen;
  DWORD SVA;
  WORD sNum;

  __try
  {
    if (!hMod || !pSect || !OutName) return FALSE;

    pNtHeader = (PIMAGE_NT_HEADERS32)((DWORD)hMod + pDosHeader->e_lfanew);
    pSection = (PIMAGE_SECTION_HEADER)((DWORD)pNtHeader + 24 + pNtHeader->FileHeader.SizeOfOptionalHeader);
    sNum = pNtHeader->FileHeader.NumberOfSections;

    while (sNum)
    {
      SVA = (DWORD)hMod + pSection->VirtualAddress;
      if (((DWORD)pSect >= SVA) && ((DWORD)pSect < SVA + pSection->Misc.VirtualSize))
      {
        mbstowcs_s(&blen, OutName, sMax, (const char *)pSection->Name, 8);
        return TRUE;
      }
      sNum--;
      pSection++;
    }
    return FALSE;
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    return FALSE;
  }
}

BOOL GetDataSection(HMODULE hMod, PDWORD dsAddr, __in_opt PDWORD dsSize)
{
  PIMAGE_DOS_HEADER pDosHeader = (PIMAGE_DOS_HEADER)hMod;
  PIMAGE_NT_HEADERS32 pNtHeader;
  PIMAGE_SECTION_HEADER pSection;
  WORD sNum;

  __try
  {
    if (!hMod || !dsAddr) return FALSE;

    pNtHeader = (PIMAGE_NT_HEADERS32)((DWORD)hMod + pDosHeader->e_lfanew);
    pSection = (PIMAGE_SECTION_HEADER)((DWORD)pNtHeader + 24 + pNtHeader->FileHeader.SizeOfOptionalHeader);
    sNum = pNtHeader->FileHeader.NumberOfSections;
    
    while (sNum)      //优先根据名称查找数据段
    {
      if (strncmp((const char *)pSection->Name, ".data\0\0\0", 8) == 0)
      {
        *dsAddr = (DWORD)hMod + pSection->VirtualAddress;
        if (dsSize)
          *dsSize = pSection->Misc.VirtualSize;
        return TRUE;
      }

      sNum--;
      pSection++;
    }

    pSection = (PIMAGE_SECTION_HEADER)((DWORD)pNtHeader + 24 + pNtHeader->FileHeader.SizeOfOptionalHeader);
    sNum = pNtHeader->FileHeader.NumberOfSections;

    while (sNum)      //根据段属性查找数据段
    {
      if (CHK_FLAG(pSection->Characteristics, IMAGE_SCN_CNT_INITIALIZED_DATA) &&
        CHK_FLAG(pSection->Characteristics, IMAGE_SCN_MEM_READ) &&
        CHK_FLAG(pSection->Characteristics, IMAGE_SCN_MEM_WRITE))
      {
        *dsAddr = (DWORD)hMod + pSection->VirtualAddress;
        if (dsSize)
          *dsSize = pSection->Misc.VirtualSize;
        return TRUE;
      }

      sNum--;
      pSection++;
    }

    return FALSE;
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    return FALSE;
  }
}

int ImageList_Add32bpp(HIMAGELIST hImageList, HINSTANCE hInst, UINT nId, UINT uType)
{
  HBITMAP hBitmap = (HBITMAP)LoadImage(hInst, MAKEINTRESOURCE(nId), uType, 0, 0, LR_CREATEDIBSECTION);
  int i = ImageList_Add(hImageList, hBitmap, NULL);
  DeleteObject(hBitmap);
  return i;
}

void GetFileName(PWCHAR FilePath)
{
  DWORD fpl = wcslen(FilePath);
  PWCHAR pfp = FilePath;
  PWCHAR los = 0;
  DWORD os;
  WCHAR stmp[MAX_PATH];

  if (!FilePath || !fpl) return;

  while (*pfp)
  {
    if (memcmp(pfp, L"\\\0", sizeof(WCHAR)) == 0)
      los = pfp;
    pfp++;
    if (pfp > FilePath + MAX_PATH * sizeof(WCHAR))
      break;
  }

  if (los)
  {
    los++;
    os = los - FilePath;
    os = fpl * sizeof(WCHAR) - os;
    ZeroMemory(stmp, MAX_PATH * sizeof(WCHAR));
    memcpy(stmp, los, os);
    wcscpy_s(FilePath, MAX_PATH, stmp);
  }
}

void GetPageProtectString(DWORD PP, PWCHAR _Out, DWORD _Max)
{
  if (PP == 0)
  {
    wcscpy_s(_Out, _Max, T_UNKNOWN);
    return;
  }

  if ((PP & PAGE_NOACCESS) == PAGE_NOACCESS)
  {
    wcscpy_s(_Out, _Max, T_P_NOACCESS);
  }
  else if ((PP & PAGE_READONLY) == PAGE_READONLY)
  {
    wcscpy_s(_Out, _Max, T_P_READONLY);
  }
  else if ((PP & PAGE_READWRITE) == PAGE_READWRITE)
  {
    wcscpy_s(_Out, _Max, T_P_READWRITE);
  }
  else if ((PP & PAGE_WRITECOPY) == PAGE_WRITECOPY)
  {
    wcscpy_s(_Out, _Max, T_P_WRITECOPY);
  }
  else if ((PP & PAGE_EXECUTE) == PAGE_EXECUTE)
  {
    wcscpy_s(_Out, _Max, T_P_EXECUTE);
  }
  else if ((PP & PAGE_EXECUTE_READ) == PAGE_EXECUTE_READ)
  {
    wcscpy_s(_Out, _Max, T_P_EXECUTE_R);
  }
  else if ((PP & PAGE_EXECUTE_READWRITE) == PAGE_EXECUTE_READWRITE)
  {
    wcscpy_s(_Out, _Max, T_P_EXECUTE_RW);
  }
  else if ((PP & PAGE_EXECUTE_WRITECOPY) == PAGE_EXECUTE_WRITECOPY)
  {
    wcscpy_s(_Out, _Max, T_P_EXECUTE_WC);
  }
  else
  {
    wcscpy_s(_Out, _Max, T_UNKNOWN);
  }

  if ((PP & PAGE_GUARD) == PAGE_GUARD)
  {
    wcscat_s(_Out, _Max, T_SEPARATOR);
    wcscat_s(_Out, _Max, T_P_GUARD);
  }

  if ((PP & PAGE_NOCACHE) == PAGE_NOCACHE)
  {
    wcscat_s(_Out, _Max, T_SEPARATOR);
    wcscat_s(_Out, _Max, T_P_NOCACHE);
  }

  if ((PP & PAGE_WRITECOMBINE) == PAGE_WRITECOMBINE)
  {
    wcscat_s(_Out, _Max, T_SEPARATOR);
    wcscat_s(_Out, _Max, T_P_WRITECOMBINE);
  }
}

DWORD GetThreadInformation(HANDLE hThread, THREADINFOCLASS TIC, PVOID pData, DWORD dLen)
{
  DWORD OutVal;
  QueryInformationThread QIT = (QueryInformationThread)GetProcAddress(GetModuleHandle(T_NTDLL), "NtQueryInformationThread");

  if (!hThread) return FALSE;
  if (!QIT) return FALSE;

  if (QIT(hThread, TIC, pData, dLen, &OutVal) != STATUS_SUCCESS) return 0;
  return OutVal;
}

DWORD GetThreadStartAddress(HANDLE hThread)
{
  DWORD ret;

  if (GetThreadInformation(hThread, ThreadQuerySetWin32StartAddress, &ret, sizeof(DWORD)))
    return ret;
  else
    return 0;
}

PVOID GetThreadEnvironmentBlockAddress(HANDLE hThread)
{
  THREAD_BASIC_INFORMATION TBI;

  if (GetThreadInformation(hThread, ThreadBasicInformation, &TBI, sizeof(THREAD_BASIC_INFORMATION)))
    return TBI.TebBaseAddress;
  else
    return 0;
}

SRDBG_TS GetThreadState(ULONG ulPID, ULONG ulTID)
{
  QuerySystemInformation pNtQSI;
  ULONG n = 0x100;
  BOOL done = FALSE;

  pNtQSI = (QuerySystemInformation)GetProcAddress(GetModuleHandle(T_NTDLL), "NtQuerySystemInformation");
  if (pNtQSI == NULL) return TS_Unknown;

  //枚举得到所有进程

  PSYSTEM_PROCESSES sp = new SYSTEM_PROCESSES[n];
  PSYSTEM_PROCESSES p;

  while (pNtQSI(SystemProcessInformation, sp, n * sizeof(SYSTEM_PROCESSES), 0) == STATUS_INFO_LENGTH_MISMATCH)
  {
    delete[] sp;
    sp = new SYSTEM_PROCESSES[n = n * 2];
  }

  p = sp;

  //遍历进程列表
  while (!done)
  {
    if (p->ProcessId == ulPID)
    {
      ULONG i;

      for (i = 0; i < p->ThreadCount; i++)
      {
        SYSTEM_THREADS systemThread = p->Threads[i];

        if (systemThread.ClientId.UniqueThread == ulTID) //找到线程    
        {
          delete[] sp;

          if (systemThread.dwState == StateTerminated)
            return TS_Terminated;

          if (systemThread.dwState == StateWait)
          {
            if (systemThread.dwWaitReason == Suspended || systemThread.dwWaitReason == WrSuspended)
              return TS_Suspended;
            else
              return TS_Wait;
          }

          return TS_Running;
        }
      }
    }

    done = (p->NextEntryDelta == 0);
    p = PSYSTEM_PROCESSES(PCHAR(p) + p->NextEntryDelta);
  }

  delete[] sp;
  return TS_Unknown;
}

size_t casmlen(char *src, size_t max)
{
  size_t i, len;

  if (src == NULL || max == 0)
    return 0;

  for (i = 0; i < max; i++)
  {
    /*如果类型符为CD_NULL*/
    if (src[i] == CD_NULL) return i;

    /*如果字符串首字符已越界*/
    if (i + 1 >= max) return i;

    i++;

    /*字符串长度不为0*/
    if ((len = strnlen(&src[i], max - i)) != 0)
    {
      /*如果超过max*/
      if (i + len >= max)
        return i;
      else
        i += len;
    }
  }
  return i;
}


size_t casmcpy(char *dest, char *src, size_t max)
{
  size_t i, len;

  if (dest == NULL || src == NULL || max == 0)
    return 0;

  for (i = 0; i < max; i++)
  {
    /*如果类型符为CD_NULL*/
    if (src[i] == CD_NULL)
    {
      dest[i] = CD_NULL;
      break;
    }

    //复制颜色索引
    dest[i] = src[i]; 
    i++;

    //判断字符串长度
    if ((len = strnlen(&src[i], max - i)) == 0) break;

    //如果当前位置加字符串的长度大于等于最大计数,则返回
    if (i + len >= max)
    {
      dest[--i] = CD_NULL;
      break;
    }

    //复制字串符*
    memcpy_s(&dest[i], len, &src[i], len);
    //为当前索引加上字符串长度
    i += len;
    //置最后一位为0
    dest[i] = 0;
  }

  return i;
}

/*
void casmcpy(PWCHAR dest, PWCHAR src, size_t scount)
{
  size_t i;
  size_t wlen;

  if (!dest || !src || !scount) return;

  for (i = 0; i < scount; i++)
  {
    if (src[i] == 0)
    {
      dest[i] = 0;
      break;
    }

    dest[i] = src[i]; //复制颜色索引
    i++;

    //判断字符串长度
    if (!(wlen = wcslen(&src[i]))) break;
    //如果当前位置加字符串的长度大于等于最大计数,则返回
    if (i + wlen >= scount) break;
    //复制字串符*
    wmemcpy_s(&dest[i], wlen, &src[i], wlen);
    //为当前索引加上字符串长度
    i += wlen;
    //置最后一位为0
    dest[i] = 0;
  }
}


void casmcpy_a(PWCHAR *dest, PWCHAR src, size_t scount)
{
  size_t i;
  size_t wlen;

  if (!dest || !src || !scount) return;
  PWCHAR pmem = (PWCHAR)malloc(scount * sizeof(wchar_t));
  if (!pmem) return;
  if (*dest) free(*dest);
  *dest = 0;

  for (i = 0; i < scount; i++)
  {
    if (src[i] == 0) break;
    pmem[i] = src[i];
    i++;

    if (!(wlen = wcslen(&src[i]))) break;
    if (i + wlen >= scount) break;
    wmemcpy_s(&pmem[i], wlen, &src[i], wlen);
    i += wlen;
    pmem[i] = 0;
  }
  *dest = pmem;
}
*/

BOOL WCharIsHex(PWCHAR src)
{
  if (!src) return FALSE;
  UINT slen = wcslen(src);
  if (!slen) return FALSE;
  UINT i;

  for (i = 0; i < slen; i++)
  {
    if (!(src[i] >= L'0' && src[i] <= L'9' || src[i] >= L'A' && src[i] <= L'F' || src[i] >= L'a' && src[i] <= L'f'))
      return FALSE;
  }
  return TRUE;
}

BOOL WCharTrim(PWCHAR src)
{
  if (!src) return FALSE;
  UINT slen = wcslen(src);
  if (!slen) return FALSE;
  PWCHAR stmp = (PWCHAR)calloc(slen + 1, sizeof(wchar_t));
  UINT i;
  int cb = -1;
  int ce = -1;

  for (i = 0; i < slen; i++)
  {
    if (src[i] == L' ' || src[i] == L'\t') continue;
    if (cb == -1) cb = i;
    ce = i;
  }

  if (cb == -1)
  {
    free(stmp);
    return FALSE;
  }

  wmemcpy(stmp, &src[cb], ce - cb + 1);
  wcscpy_s(src, slen + 1, stmp);
  free(stmp);
  return TRUE;
}


PVOID WINAPI MemMem(PBYTE pMem1, SIZE_T MemSize1, PBYTE FindMem, SIZE_T fmSize)
{
  UINT i;

  if (fmSize >= MemSize1) return NULL;
  for (i = 0; i <= MemSize1 - fmSize; i++)
  {
    if (memcmp(&pMem1[i], FindMem, fmSize) == 0)
      return &pMem1[i];
  }
  return NULL;
}

BOOL GetDataMD5Sum(PBYTE pBuffer, UINT bMax, PUNIMD5 poMD5)
{
  MD5_CTX mctx;

  if (!pBuffer || !bMax || !poMD5) return FALSE;

  MD5_Init(&mctx);
  MD5_Update(&mctx, pBuffer, bMax);
  MD5_Final(poMD5->digest_byte, &mctx);
  return TRUE;
}

ONLY_ASM UINT WINAPI FindMemory(PBYTE destAddr, UINT destSize, PCMP_BYTE pCB, UINT cbCount, BOOL Absolute)
{
  __asm
  {
    push ebp
      mov ebp, esp
      push ebx
      push esi
      push edi
      push eax                            //i
      cmp destAddr, 0                      //判断参数是否为NULL
      je BackErr
      cmp destSize, 0
      je BackErr
      cmp pCB, 0
      je BackErr
      cmp cbCount, 0
      je BackErr
      mov eax, cbCount
      cmp eax, destSize
      ja BackErr
      mov edi, destAddr
    dLoop :
    mov esi, pCB
      xor eax, eax
      mov[ebp - 0x10], eax
    sLoop :
    mov ax, word ptr[esi]               //取CMP_BYTE
      test al, al                          //判断IsValid
      je sNext
      mov ebx, [ebp - 0x10]
      cmp ah, byte ptr[edi + ebx]
      jne dNext

    sNext :
    inc dword ptr[ebp - 0x10]            //自加计数
      add esi, 0x02                        //自加结构指针
      mov eax, dword ptr[ebp - 0x10]
      cmp eax, cbCount                     //比较cbCount和计数
      jb sLoop                            //计数 < cbCount 跳转
      mov eax, Absolute
      test eax, eax
      mov eax, edi
      jne Return
      sub eax, destAddr
      jmp Return
    dNext :
    inc edi                             //srcAddr++
      mov eax, destSize
      sub eax, cbCount                     //eax = srcSize - cbCount
      add eax, destAddr
      cmp edi, eax                         //比较
      jbe dLoop

    BackErr :
    xor eax, eax
      dec eax
    Return :
    pop ecx
      pop edi
      pop esi
      pop ebx
      mov esp, ebp
      pop ebp
      retn 0x14
  }
}

ONLY_ASM INT_PTR WINAPI GetFuncAddr(INT_PTR hMod, const PCHAR pName)
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
      mov [ebp - 0x04], ebx
      mov  ebx, [eax + 0x1C]
      add  ebx, [ebp + 0x08]
      mov [ebp - 0x08], ebx
      mov  ebx, [eax + 0x20]
      add  ebx, [ebp + 0x08]
      mov [ebp - 0x0C], ebx
      mov  ebx, [eax + 0x24]
      add  ebx, [ebp + 0x08]
      mov [ebp - 0x10], ebx

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