#include "../sr_pch.h"

MC_ApiHook * MC_ApiHook::This = NULL;

MC_ApiHook::~MC_ApiHook()
{
  if (this->m_hWnd)
  {
    DestroyWindow(this->m_hWnd);
    this->m_hWnd = NULL;
  }
  This = NULL;
}

HWND MC_ApiHook::CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style)
{
  if (this->m_hWnd)
  {
    this->SetTop();
    return this->m_hWnd;
  }

  MDICREATESTRUCT MCS;

  MCS.szClass = (PWCHAR)mf->rcls_mc_apihook;
  MCS.szTitle = wName;
  MCS.hOwner = mf->MdlBase;
  MCS.style = Style;
  MCS.cx = cx;
  MCS.cy = cy;
  MCS.x = x;
  MCS.y = y;
  MCS.lParam = (LPARAM)this;

  this->m_hWnd = (HWND)::SendMessage(hw_MDI, WM_MDICREATE, 0, (LPARAM)&MCS);
  return this->m_hWnd;
}

void MC_ApiHook::InsertColumn()
{
  LV_COLUMN lvc;
  ZeroMemory(&lvc, sizeof(LV_COLUMN));
  lvc.mask = LVCF_TEXT | LVCF_WIDTH;

  lvc.pszText = L"模块";
  lvc.cx = 108;
  ListView_InsertColumn(lv_hook, 0, &lvc);

  lvc.pszText = L"挂钩对象";
  lvc.cx = 226;
  ListView_InsertColumn(lv_hook, 1, &lvc);

  lvc.pszText = L"挂钩类型";
  lvc.cx = 64;
  ListView_InsertColumn(lv_hook, 2, &lvc);

  lvc.pszText = L"挂钩地址";
  lvc.cx = DefWidth;
  ListView_InsertColumn(lv_hook, 3, &lvc);

  lvc.pszText = L"原始值";
  lvc.cx = 192;
  ListView_InsertColumn(lv_hook, 4, &lvc);

  lvc.pszText = L"当前值";
  lvc.cx = 192;
  ListView_InsertColumn(lv_hook, 5, &lvc);
}

void MC_ApiHook::UpdateModule()
{
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
  DWORD i = 1;
  MODULEENTRY32 me;

  ComboBox_DeleteAll(cb_mod);
  SendMessage(cb_mod, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)L"<全部模块>");
  if (hSnapshot == INVALID_HANDLE_VALUE) return;
  me.dwSize = sizeof(MODULEENTRY32);

  if (Module32First(hSnapshot, &me))
  {
    do
    {
      SendMessage(cb_mod, CB_ADDSTRING, 0, (LPARAM)me.szModule);
      SetProgress(i, 100);
      i++;
    } while (Module32Next(hSnapshot, &me));
    SetProgress(100, 100);
  }
  CloseHandle(hSnapshot);
  SetProgress(0, 0);
}

void MC_ApiHook::SetStateText(const PWCHAR ptext)
{
  RECT rect;

  if (ptext)
    wcscpy_s(StateText, 64, ptext);
  else
    StateText[0] = 0;

  GetClientRect(m_hWnd, &rect);
  rect.left++;
  rect.top = rect.bottom - 26;
  rect.right--;
  rect.bottom--;
  InvalidateRect(this->m_hWnd, &rect, TRUE);
  UpdateWindow(this->m_hWnd);
}

void MC_ApiHook::SetProgress(DWORD Cur, DWORD Max)
{
  RECT rect;

  drCur = Cur;
  drMax = Max;

  GetClientRect(m_hWnd, &rect);

  if (!drMax)
    drpl = 0;
  else
    drpl = (rect.right - rect.left - 4) / drMax * drCur;

  rect.left++;
  rect.top = rect.bottom - 26;
  rect.right--;
  rect.bottom--;
  InvalidateRect(this->m_hWnd, &rect, TRUE);
  UpdateWindow(this->m_hWnd);
}

BOOL MC_ApiHook::AddItem(PWCHAR hMod, PWCHAR hObj, PWCHAR hType, PWCHAR hAddr, PWCHAR hOVal, PWCHAR hCVal, PAPIHOOK_LV_TYPE lParam)
{
  if (!lParam) return FALSE;
  LV_ITEM lvi;
  PAPIHOOK_LV_TYPE palt = (PAPIHOOK_LV_TYPE)malloc(sizeof(APIHOOK_LV_TYPE));
  if (!palt) return FALSE;

  memcpy_s(palt, sizeof(APIHOOK_LV_TYPE), lParam, sizeof(APIHOOK_LV_TYPE));

  lvi.mask = LVIF_TEXT | LVIF_PARAM;
  lvi.iItem = iidx;
  lvi.iSubItem = 0;
  lvi.pszText = hMod;
  lvi.lParam = (LPARAM)palt;
  ListView_InsertItem(lv_hook, &lvi);

  lvi.mask = LVIF_TEXT;
  lvi.iSubItem = 1;
  lvi.pszText = hObj;
  ListView_SetItem(lv_hook, &lvi);

  lvi.iSubItem = 2;
  lvi.pszText = hType;
  ListView_SetItem(lv_hook, &lvi);

  lvi.iSubItem = 3;
  lvi.pszText = hAddr;
  ListView_SetItem(lv_hook, &lvi);

  lvi.iSubItem = 4;
  lvi.pszText = hOVal;
  ListView_SetItem(lv_hook, &lvi);

  lvi.iSubItem = 5;
  lvi.pszText = hCVal;
  ListView_SetItem(lv_hook, &lvi);

  return TRUE;
}

void MC_ApiHook::CheckIAT(HMODULE hMod, LPMODULEENTRY32 lpme)
{
  __try
  {
    if (!hMod) return;
    PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)hMod;
    PIMAGE_NT_HEADERS pNtHeader = PIMAGE_NT_HEADERS(UINT(hMod) + pDos->e_lfanew);
    PIMAGE_OPTIONAL_HEADER32 pOptHead = &pNtHeader->OptionalHeader;
    PIMAGE_IMPORT_DESCRIPTOR pImpDir;
    PIMAGE_THUNK_DATA32 pOTD32;
    PIMAGE_THUNK_DATA32 pTD32;
    PCHAR pfName;
    UINT i = 0;
    UINT j;
    INT_PTR oAddr;
    INT_PTR rAddr;
    INT_PTR mBase;
    HANDLE hFile = NULL;
    HANDLE hFileMap = NULL;
    size_t bval;
    DWORD dwtmp;
    APIHOOK_LV_TYPE alt;
    WCHAR stmp0[MAX_PATH];
    WCHAR stmp1[128];
    WCHAR stmp2[128];
    WCHAR stmp3[128];
    WCHAR stmp4[128];

    if (pNtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386) return;
    if (pNtHeader->FileHeader.SizeOfOptionalHeader < 0x0070) return;
    if (pOptHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress == 0) return;

    pImpDir = PIMAGE_IMPORT_DESCRIPTOR(UINT(hMod) + pOptHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_IMPORT].VirtualAddress);

    /* 循环每一个dll */
    while (pImpDir[i].OriginalFirstThunk)
    {
      DWORD hSize;
      DWORD lSize;
      PBYTE pFile = NULL;

      GetModuleFileName(GetModuleHandleA(PCHAR(UINT(hMod) + pImpDir[i].Name)), stmp0, MAX_PATH);
      hFile = CreateFile(stmp0, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_READONLY, NULL);
      if (hFile != INVALID_HANDLE_VALUE)
      {
        lSize = GetFileSize(hFile, &hSize);
        SetFilePointer(hFile, 0x3CU, 0, FILE_BEGIN);
        ReadFile(hFile, &dwtmp, 4, (PDWORD)&bval, NULL);
        SetFilePointer(hFile, dwtmp, 0, FILE_BEGIN);
        ReadFile(hFile, &dwtmp, 4, (PDWORD)&bval, NULL);
        SetFilePointer(hFile, 0, 0, FILE_BEGIN);

        if (dwtmp == IMAGE_NT_SIGNATURE && !hSize && lSize <= 0x20000000)        //判断符合读取文件的条件
        {
          hFileMap = CreateFileMapping(hFile, NULL,
            (NtVer >= _WIN32_WINNT_WIN8) ? SEC_IMAGE_NO_EXECUTE | PAGE_READONLY : SEC_IMAGE | PAGE_READONLY,
            0, lSize, NULL);

          if (hFileMap)
          {
            pOTD32 = PIMAGE_THUNK_DATA32(UINT(hMod) + pImpDir[i].OriginalFirstThunk);
            pTD32 = PIMAGE_THUNK_DATA32(UINT(hMod) + pImpDir[i].FirstThunk);

            pFile = (PBYTE)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);
            if (pFile)
            {
              j = 0;
              /* 映射文件成功,循环每一个函数 */
              while (pOTD32[j].u1.AddressOfData)
              {
                CheckMessageQueue();
                if (blStop)
                {
                  UnmapViewOfFile(pFile);
                  CloseHandle(hFileMap);
                  CloseHandle(hFile);
                  return;
                }

                if (IMAGE_SNAP_BY_ORDINAL32(pOTD32[j].u1.Ordinal))
                  pfName = PCHAR(IMAGE_ORDINAL32(pOTD32[j].u1.Ordinal));
                else
                  pfName = PCHAR(PIMAGE_IMPORT_BY_NAME(UINT(hMod) + pOTD32[j].u1.AddressOfData)->Name);

                oAddr = GetFuncAddr(INT_PTR(pFile), pfName);
                rAddr = UINT(oAddr) - UINT(pFile);    //函数RVA
                mBase = (INT_PTR)GetModuleHandleA(PCHAR(UINT(hMod) + pImpDir[i].Name));   //导入模块基址

                if (rAddr != INT_PTR(pTD32[j].u1.Function) - mBase)
                {
                  mbstowcs_s(&bval, stmp1, 128, LPCSTR(UINT(hMod) + pImpDir[i].Name), _TRUNCATE);
                  if (IMAGE_SNAP_BY_ORDINAL32(pOTD32[j].u1.Ordinal))
                    swprintf_s(stmp0, MAX_PATH, L"%s#>%04hX", stmp1, DWORD(IMAGE_ORDINAL32(pOTD32[j].u1.Ordinal)));
                  else
                  {
                    mbstowcs_s(&bval, stmp2, 128, LPCSTR(PIMAGE_IMPORT_BY_NAME(UINT(hMod) + pOTD32[j].u1.AddressOfData)->Name), _TRUNCATE);
                    swprintf_s(stmp0, MAX_PATH, L"%s->%s", stmp1, stmp2);
                  }

                  wcscpy_s(stmp1, 128, L"IAT");
                  swprintf_s(stmp2, 128, HEXFORMAT, &pTD32[j].u1.Function);
                  swprintf_s(stmp3, 128, L"Addr: %08X", mBase + rAddr);
                  swprintf_s(stmp4, 128, L"Addr: %08X", pTD32[j].u1.Function);
                  alt.LV.Item = iidx;
                  alt.LV.BackColor = RGB(255, 255, 255);
                  alt.LV.ForeColor = RGB(0, 0, 0);
                  alt.hType = 1;
                  alt.cAddr = pTD32[j].u1.Function;
                  alt.oAddr = mBase + rAddr;
                  alt.fOffset = 0;
                  AddItem(lpme->szModule, stmp0, stmp1, stmp2, stmp3, stmp4, &alt);
                }
                j++;
              }

              i++;
              UnmapViewOfFile(pFile);
              CloseHandle(hFileMap);
              CloseHandle(hFile);
              continue;
            }
          }
        }
        if (hFileMap) CloseHandle(hFileMap);
        CloseHandle(hFile);
      }

      j = 0;
      pOTD32 = PIMAGE_THUNK_DATA32(UINT(hMod) + pImpDir[i].OriginalFirstThunk);
      pTD32 = PIMAGE_THUNK_DATA32(UINT(hMod) + pImpDir[i].FirstThunk);

      /* 映射文件失败,循环每一个函数 */
      while (pOTD32[j].u1.AddressOfData)
      {
        CheckMessageQueue();
        if (blStop) return;

        if (IMAGE_SNAP_BY_ORDINAL32(pOTD32[j].u1.Ordinal))
          pfName = PCHAR(IMAGE_ORDINAL32(pOTD32[j].u1.Ordinal));
        else
          pfName = PCHAR(PIMAGE_IMPORT_BY_NAME(UINT(hMod) + pOTD32[j].u1.AddressOfData)->Name);

        oAddr = GetFuncAddr(INT_PTR(GetModuleHandleA(LPCSTR(UINT(hMod) + pImpDir[i].Name))), pfName);

        if (oAddr != INT_PTR(pTD32[j].u1.Function))
        {
          mbstowcs_s(&bval, stmp1, 128, LPCSTR(UINT(hMod) + pImpDir[i].Name), _TRUNCATE);
          if (IMAGE_SNAP_BY_ORDINAL32(pOTD32[j].u1.Ordinal))
            swprintf_s(stmp0, MAX_PATH, L"%s#>%04hX", stmp1, DWORD(IMAGE_ORDINAL32(pOTD32[j].u1.Ordinal)));
          else
          {
            mbstowcs_s(&bval, stmp2, 128, LPCSTR(PIMAGE_IMPORT_BY_NAME(UINT(hMod) + pOTD32[j].u1.AddressOfData)->Name), _TRUNCATE);
            swprintf_s(stmp0, MAX_PATH, L"%s->%s",  stmp1, stmp2);
          }

          wcscpy_s(stmp1, 128, L"IAT");
          swprintf_s(stmp2, 128, HEXFORMAT, &pTD32[j].u1.Function);
          swprintf_s(stmp3, 128, L"Addr:%08X", oAddr);
          swprintf_s(stmp4, 128, L"Addr:%08X", pTD32[j].u1.Function);
          alt.LV.Item = iidx;
          alt.LV.BackColor = RGB(255, 255, 255);
          alt.LV.ForeColor = RGB(255, 168, 0);
          alt.hType = 1;
          alt.cAddr = pTD32[j].u1.Function;
          alt.oAddr = oAddr;
          alt.fOffset = 0;
          AddItem(lpme->szModule, stmp0, stmp1, stmp2, stmp3, stmp4, &alt);
        }
        j++;
      }
      i++;
    }

    return;
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    SetStateText(L"检查IAT Hook时出现非法访问!");
    return;
  }
}

void MC_ApiHook::CheckEAT(HMODULE hMod, LPMODULEENTRY32 lpme)
{
  __try
  {
    if (!hMod) return;
    PIMAGE_DOS_HEADER pDos = (PIMAGE_DOS_HEADER)hMod;
    PIMAGE_NT_HEADERS pNtHeader = PIMAGE_NT_HEADERS(UINT(hMod) + pDos->e_lfanew);
    PIMAGE_OPTIONAL_HEADER32 pOptHead = &pNtHeader->OptionalHeader;
    PIMAGE_EXPORT_DIRECTORY pExpDir = PIMAGE_EXPORT_DIRECTORY(UINT(hMod) + pOptHead->DataDirectory[IMAGE_DIRECTORY_ENTRY_EXPORT].VirtualAddress);
    DWORD fMax;
    DWORD fBase;
    DWORD fIdx;
    PWORD pOrdin;
    PDWORD pNames;
    PDWORD pFuncs;
    DWORD rAddr;
    DWORD trAddr;
    HANDLE hFile;
    HANDLE hFileMap;
    DWORD hSize;
    DWORD lSize;
    PBYTE pFile;
    DWORD dwtmp;
    DWORD bval;
    APIHOOK_LV_TYPE alt;
    WCHAR stmp0[MAX_PATH];
    WCHAR stmp1[128];
    WCHAR stmp2[128];
    WCHAR stmp3[128];
    WCHAR stmp4[128];

    if (pNtHeader->FileHeader.Machine != IMAGE_FILE_MACHINE_I386 || pNtHeader->FileHeader.SizeOfOptionalHeader < 0x0068)
    {
      swprintf_s(stmp0, MAX_PATH, L"模块[%s]不合法!", lpme->szModule);
      SetStateText(stmp0);
      return;
    }

    if (!pExpDir)
    {
      swprintf_s(stmp0, MAX_PATH, L"模块[%s]不包含导出表.", lpme->szModule);
      SetStateText(stmp0);
      return;
    }
    
    hFile = CreateFile(lpme->szExePath, GENERIC_READ, FILE_SHARE_READ, NULL, OPEN_EXISTING, FILE_ATTRIBUTE_NORMAL, NULL);
    if (hFile == INVALID_HANDLE_VALUE)
    {
      SetStateText(L"打开文件失败,检查EAT Hook失败!");
      return;
    }

    lSize = GetFileSize(hFile, &hSize);
    SetFilePointer(hFile, 0x3CU, 0, FILE_BEGIN);
    ReadFile(hFile, &dwtmp, 4, (PDWORD)&bval, NULL);
    SetFilePointer(hFile, dwtmp, 0, FILE_BEGIN);
    ReadFile(hFile, &dwtmp, 4, (PDWORD)&bval, NULL);

    if (dwtmp != IMAGE_NT_SIGNATURE || hSize || lSize > 0x20000000)
    {
      CloseHandle(hFile);
      SetStateText(L"文件过大或者格式不正确,检查EAT Hook失败!");
      return;
    }

    hFileMap = CreateFileMapping(hFile, NULL,
      (NtVer >= _WIN32_WINNT_WIN8) ? SEC_IMAGE_NO_EXECUTE | PAGE_READONLY : SEC_IMAGE | PAGE_READONLY,
      0, lSize, NULL);

    if (!hFileMap)
    {
      dwtmp = GetLastError();
      CloseHandle(hFile);
      SetStateText(L"创建文件映射失败,检查EAT Hook失败!");
      return;
    }

    pFile = (PBYTE)MapViewOfFile(hFileMap, FILE_MAP_READ, 0, 0, 0);

    if (!pFile)
    {
      CloseHandle(hFileMap);
      CloseHandle(hFile);
      SetStateText(L"映射文件失败,检查EAT Hook失败!");
      return;
    }

    fMax = pExpDir->NumberOfNames;
    pOrdin = PWORD(DWORD(hMod) + pExpDir->AddressOfNameOrdinals);
    pNames = PDWORD(DWORD(hMod) + pExpDir->AddressOfNames);
    pFuncs = PDWORD(DWORD(hMod) + pExpDir->AddressOfFunctions);

    for (fIdx = 0; fIdx < fMax;fIdx++)
    {
      CheckMessageQueue();
      if (blStop)
      {
        SetProgress(0, 0);
        UnmapViewOfFile(pFile);
        CloseHandle(hFileMap);
        CloseHandle(hFile);
        return;
      }

      SetProgress(fIdx, fMax);
      rAddr = pFuncs[fIdx];
      trAddr = GetFuncAddr(INT_PTR(pFile), (const PCHAR)(pExpDir->Base + fIdx));
      if (!trAddr) continue;
      trAddr -= DWORD(pFile);

      if (rAddr != trAddr)
      {
        if (pNames[fIdx])
        {
          mbstowcs_s((size_t *)&bval, stmp1, 128, LPCCH(DWORD(hMod) + pNames[fIdx]), _TRUNCATE);
          swprintf_s(stmp0, MAX_PATH, L"%s->%s",lpme->szModule,stmp1);
        }
        else
          swprintf_s(stmp0, MAX_PATH, L"%s#>%04hX", lpme->szModule, pExpDir->Base + fIdx);

        wcscpy_s(stmp1, 128, L"EAT");
        swprintf_s(stmp2, 128, HEXFORMAT, &pFuncs[fIdx]);
        swprintf_s(stmp3, 128, L"Addr: %08X", DWORD(hMod) + trAddr);
        swprintf_s(stmp4, 128, L"Addr: %08X", pFuncs[fIdx] + DWORD(hMod));
        alt.LV.Item = iidx;
        alt.LV.BackColor = RGB(255, 255, 255);
        alt.LV.ForeColor = RGB(0, 0, 0);
        alt.hType = 1;
        alt.cAddr = pFuncs[fIdx] + DWORD(hMod);
        alt.oAddr = DWORD(hMod) + trAddr;
        alt.fOffset = 0;
        AddItem(lpme->szModule, stmp0, stmp1, stmp2, stmp3, stmp4, &alt);
      }
    }

    SetProgress(0, 0);
    UnmapViewOfFile(pFile);
    CloseHandle(hFileMap);
    CloseHandle(hFile);
    return;
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    SetStateText(L"检查EAT Hook时出现非法访问!");
    return;
  }
}

void MC_ApiHook::CheckHook()
{
  BOOL c_iat = SendMessage(chk_iat, BM_GETCHECK, 0, 0);
  BOOL c_eat = SendMessage(chk_eat, BM_GETCHECK, 0, 0);
  BOOL c_inline = SendMessage(chk_inline, BM_GETCHECK, 0, 0);
  int icur = SendMessage(cb_mod, CB_GETCURSEL, 0, 0);
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPMODULE, GetCurrentProcessId());
  MODULEENTRY32 me;
  WCHAR dmod[MAX_PATH];

  if (!c_iat && !c_eat && !c_inline)
  {
    SetStateText(L"未选择任何要检测的Hook类型!");
    return;
  }

  if (icur < 0)
  {
    SetStateText(L"未选择模块!");
    return;
  }

  IsCheck = TRUE;
  GetWindowText(cb_mod, dmod, MAX_PATH);
  SetWindowText(btn_chk, L"停止检测");
  ListView_DeleteAllItems(lv_hook);
  iidx = 0;

  me.dwSize = sizeof(MODULEENTRY32);

  if (hSnapshot == INVALID_HANDLE_VALUE)
  {
    SetStateText(L"枚举模块失败!");
    return;
  }

  if (Module32First(hSnapshot, &me))
  {
    do
    {
      if (blStop)
      {
        blStop = FALSE;
        IsCheck = FALSE;
        CloseHandle(hSnapshot);
        SetWindowText(btn_chk, L"检测钩子");
        EnableWindow(btn_chk, TRUE);
        IsCheck = FALSE;
        return;
      }

      CheckMessageQueue();

      if (icur == 0)
      {
        if (c_iat) CheckIAT(me.hModule, &me);
        if (c_eat) CheckEAT(me.hModule, &me);
      }
      else
      {
        if (_tcsicmp(dmod, me.szModule) == 0)
        {
          if (c_iat) CheckIAT(me.hModule, &me);
          if (c_eat) CheckEAT(me.hModule, &me);
          break;
        }
      }

    } while (Module32Next(hSnapshot, &me));
  }
  CloseHandle(hSnapshot);
  SetWindowText(btn_chk, L"检测钩子");
  EnableWindow(btn_chk, TRUE);
  IsCheck = FALSE;
}

void MC_ApiHook::StopCheck()
{
  EnableWindow(btn_chk, FALSE);
  SetWindowText(btn_chk, L"等待...");

  blStop = TRUE;
}

LRESULT CALLBACK MC_ApiHook::ahWndProc(HWND phWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
  case WM_CREATE:
  {
    SendMessage(phWnd, WM_SETFONT, (WPARAM)mf->hf_tal, TRUE);

    This->chk_iat = CreateWindowEx(WS_EX_TRANSPARENT, WC_BUTTON, L"IAT Hook", DEF_VCHILD | BS_AUTOCHECKBOX,
      80, 6, 88, 18, phWnd, NULL, DllBaseAddr, 0);

    This->chk_eat = CreateWindowEx(WS_EX_TRANSPARENT, WC_BUTTON, L"EAT Hook", DEF_VCHILD | BS_AUTOCHECKBOX,
      170, 6, 88, 18, phWnd, NULL, DllBaseAddr, 0);

    This->chk_inline = CreateWindowEx(WS_EX_TRANSPARENT, WC_BUTTON, L"Inline Hook", DEF_VCHILD | BS_AUTOCHECKBOX,
      260, 6, 110, 18, phWnd, NULL, DllBaseAddr, 0);

    This->cb_mod = CreateWindow(WC_COMBOBOX, NULL, CBS_DROPDOWNLIST | WS_VSCROLL | DEF_VCHILD,
      420, 4, 188, 20, phWnd, 0, DllBaseAddr, 0);

    This->btn_upmod = CreateWindow(WC_BUTTON, L"刷新模块", DEF_VCHILD,
      612, 3, 80, 26, phWnd, NULL, DllBaseAddr, 0);

    This->btn_chk = CreateWindow(WC_BUTTON, L"检测钩子", DEF_VCHILD,
      696, 3, 84, 26, phWnd, NULL, DllBaseAddr, 0);

    This->lv_hook = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL, LV_STYLE, 0, 0, 0, 0, phWnd, NULL, DllBaseAddr, 0);

    ListView_SetExtendedListViewStyle(This->lv_hook, LVS_EX_FULLROWSELECT);

    if (This->chk_iat)
      SendMessage(This->chk_iat, WM_SETFONT, (WPARAM)mf->hf_fix, TRUE);

    if (This->chk_eat)
      SendMessage(This->chk_eat, WM_SETFONT, (WPARAM)mf->hf_fix, TRUE);

    if (This->chk_inline)
      SendMessage(This->chk_inline, WM_SETFONT, (WPARAM)mf->hf_fix, TRUE);

    if (This->cb_mod)
      SendMessage(This->cb_mod, WM_SETFONT, (WPARAM)mf->hf_fix, TRUE);

    if (This->btn_upmod)
      SendMessage(This->btn_upmod, WM_SETFONT, (WPARAM)mf->hf_tal, TRUE);

    if (This->btn_chk)
      SendMessage(This->btn_chk, WM_SETFONT, (WPARAM)mf->hf_tal, TRUE);

    This->StateText[0] = 0;

    This->InsertColumn();
    This->UpdateModule();
  }
    return 0;
  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    LONG bkr;
    HDC hDC = BeginPaint(phWnd, &ps);
    GetClientRect(phWnd, &ps.rcPaint);
    SelectObject(hDC, mf->hf_tal);
    FrameRect(hDC, &ps.rcPaint, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
    TextOut(hDC, 4, 8, L"检测类型:", wcslen(L"检测类型:"));
    TextOut(hDC, 376, 8, L"模块:", wcslen(L"模块:"));
    ps.rcPaint.left++;
    ps.rcPaint.top = ps.rcPaint.bottom - 26;
    ps.rcPaint.right--;
    ps.rcPaint.bottom--;
    FrameRect(hDC, &ps.rcPaint, (HBRUSH)GetStockObject(BLACK_BRUSH));

    ps.rcPaint.left++;
    ps.rcPaint.top++;
    ps.rcPaint.right--;
    ps.rcPaint.bottom--;

    bkr = ps.rcPaint.right;
    if (This->drpl)
    {
      if (ps.rcPaint.left + This->drpl < ps.rcPaint.right)
        ps.rcPaint.right = ps.rcPaint.left + This->drpl;
      FillRect(hDC, &ps.rcPaint, (HBRUSH)GetStockObject(DKGRAY_BRUSH));
    }
    ps.rcPaint.right = bkr;

    ps.rcPaint.left += 4;
    ps.rcPaint.top += 4;
    ps.rcPaint.right -= 4;
    ps.rcPaint.bottom -= 2;

    SetTextColor(hDC, RGB(64,64,255));
    DrawText(hDC, This->StateText, -1, &ps.rcPaint, DT_CENTER | DT_SINGLELINE);
    EndPaint(phWnd, &ps);
  }
    break;
  case WM_CTLCOLORSTATIC:
    if (lParam && This)
    {
      if ((HWND)lParam == This->chk_iat || (HWND)lParam == This->chk_eat ||
        (HWND)lParam == This->chk_inline)
      {
        SetBkMode(HDC(wParam), TRANSPARENT);
        return (LRESULT)GetStockObject(WHITE_BRUSH);
      }
    }
    break;
  case WM_DESTROY:
    This->m_hWnd = NULL;
    return 0;
  case WM_SIZE:
    if (wParam != SIZE_MINIMIZED)
    {
      WORD Width = LOWORD(lParam);
      WORD Height = HIWORD(lParam);

      MoveWindow(This->lv_hook, 1, 34, LOWORD(lParam) - 2, HIWORD(lParam) - 60, TRUE);

      if (This->drMax)
        This->drpl = (Width - 4) / This->drMax * This->drCur;
      else
        This->drpl = 0;
    }
    break;
  case WM_COMMAND:
  {
    WORD wid = LOWORD(wParam);
    WORD wnc = HIWORD(wParam);

    if ((HWND)lParam == This->btn_upmod && wnc == BN_CLICKED)
    {
      This->UpdateModule();
      return 0;
    }
    else if ((HWND)lParam == This->btn_chk && wnc == BN_CLICKED)
    {
      EnableWindow(This->chk_iat, FALSE);
      EnableWindow(This->chk_eat, FALSE);
      EnableWindow(This->chk_inline, FALSE);
      EnableWindow(This->cb_mod, FALSE);
      EnableWindow(This->btn_upmod, FALSE);

      This->SetStateText(T_NULL);

      if (This->IsCheck)
        This->StopCheck();
      else
        This->CheckHook();

      EnableWindow(This->chk_iat, TRUE);
      EnableWindow(This->chk_eat, TRUE);
      EnableWindow(This->chk_inline, TRUE);
      EnableWindow(This->cb_mod, TRUE);
      EnableWindow(This->btn_upmod, TRUE);
    }
  }
    break;
  case WM_NOTIFY:
  {
    if (lParam == 0) break;
    LPNMHDR pNm = (LPNMHDR)lParam;

    switch (pNm->code)
    {
    case LVN_DELETEALLITEMS:
      return 0;
    case LVN_DELETEITEM:
    {
      LPNMLISTVIEW pnl = (LPNMLISTVIEW)lParam;
      free((void *)pnl->lParam);
    }
      break;
    }

  }
    break;
  case WM_SIZING:
  {
    if (wParam >= WMSZ_LEFT && wParam <= WMSZ_BOTTOMRIGHT)
    {
      LPRECT lprc = (LPRECT)lParam;
      long Width = lprc->right - lprc->left;
      long Height = lprc->bottom - lprc->top;

      if (Width < 800) lprc->right = lprc->left + 800;
      if (Height < 200) lprc->bottom = lprc->top + 200;
    }
  }
    return 1;
  }

  return DefMDIChildProc(phWnd, Msg, wParam, lParam);
}