#include "stdafx.h"

INT_PTR CALLBACK MainProc(HWND, UINT, WPARAM, LPARAM);
void RefreshProcess(HWND hWnd);
void InjectionSoftRazor(DWORD Pid);

int APIENTRY _tWinMain(_In_ HINSTANCE hInstance,
  _In_opt_ HINSTANCE hPrevInstance,
  _In_ LPTSTR    lpCmdLine,
  _In_ int       nCmdShow)
{
  WORD NtVer;

  MeInstance = hInstance;

  __asm call dword ptr[GetVersion]
  __asm xchg al, ah
  __asm mov NtVer, ax

  PAA |= (NtVer >= _WIN32_WINNT_VISTA) ? 0xFFFF : 0x0FFF;
  setlocale(LC_ALL, NULL);
  InitCommonControls();
  DialogBox(hInstance, MAKEINTRESOURCE(IDD_MAINDLG), 0, MainProc);
  return 1;
}

INT_PTR CALLBACK MainProc(HWND hWnd, UINT msg, WPARAM wParam, LPARAM lParam)
{
  switch (msg)
  {
  case WM_INITDIALOG:
  {
    LV_COLUMN lvc;
    ZeroMemory(&lvc, sizeof(LV_COLUMN));
    lvc.mask = LVCF_TEXT | LVCF_WIDTH;

    lvc.pszText = L"PID";
    lvc.cx = 64;
    ListView_DlgInsertColumn(hWnd, IDC_PROCLIST, 0, &lvc);

    lvc.pszText = L"进程名称";
    lvc.cx = 128;
    ListView_DlgInsertColumn(hWnd, IDC_PROCLIST, 1, &lvc);

    lvc.pszText = L"进程路径";
    lvc.cx = 384;
    ListView_DlgInsertColumn(hWnd, IDC_PROCLIST, 2, &lvc);

    ListView_DlgSetExtendedListViewStyle(hWnd, IDC_PROCLIST, LVS_EX_FULLROWSELECT);
    SendDlgItemMessage(hWnd, IDC_PIDEDIT, EM_SETLIMITTEXT, 10, 0);

    RefreshProcess(hWnd);
  }
  return 1;
  case WM_COMMAND:
  {
    switch (LOWORD(wParam))
    {
    case IDC_Refresh:
      RefreshProcess(hWnd);
      break;
    case IDC_IBTN:
    {
      DWORD iPid;
      TCHAR stmp[16];

      if (GetWindowTextLength(GetDlgItem(hWnd, IDC_PIDEDIT)) <= 0) break;
      GetWindowText(GetDlgItem(hWnd, IDC_PIDEDIT), stmp, 16);
      if (_stscanf_s(stmp, L"%u", &iPid) != 1) break;
      InjectionSoftRazor(iPid);
    }
    break;
    case IDC_EBTN:
      EndDialog(hWnd, 0);
      break;
    }
  }
  break;
  case WM_NOTIFY:
  {
    if (lParam == 0) break;
    LPNMHDR pNm = (LPNMHDR)lParam;
    switch (pNm->code)
    {
    case LVN_ITEMCHANGED:
    {
      if (pNm->idFrom == IDC_PROCLIST)
      {
        int Idx = ListView_DlgGetNextItem(hWnd, IDC_PROCLIST, -1, LVNI_FOCUSED | LVNI_SELECTED);

        if (Idx != -1)
        {
          LV_ITEM lvi;
          TCHAR tPid[12];

          lvi.iItem = Idx;
          lvi.mask = LVIF_PARAM;

          ListView_DlgGetItem(hWnd, IDC_PROCLIST, &lvi);
          tPid[0] = 0;
          _stprintf_s(tPid, 12, L"%u", lvi.lParam);
          SetWindowText(GetDlgItem(hWnd, IDC_PIDEDIT), tPid);
        }
      }
    }
    break;
    }
  }
  break;
  case WM_CLOSE:
    EndDialog(hWnd, 1);
    break;
  }
  return 0;
}

void RefreshProcess(HWND hWnd)
{
  if (!hWnd) return;
  HANDLE hSnap = CreateToolhelp32Snapshot(TH32CS_SNAPPROCESS, 0);

  if (hSnap == INVALID_HANDLE_VALUE)
  {
    MessageBox(hWnd, L"创建进程快照失败!", L"错误", 0);
    return;
  }

  UINT Idx = 0;
  HANDLE hProcess;
  LV_ITEM lvi;
  PROCESSENTRY32 pe32;
  TCHAR stmp[MAX_PATH];

  ZeroMemory(&lvi, sizeof(LV_ITEM));
  pe32.dwSize = sizeof(PROCESSENTRY32);

  ListView_DlgDeleteAllItems(hWnd, IDC_PROCLIST);

  if (Process32First(hSnap, &pe32))
  {
    do
    {
      lvi.iItem = Idx;
      lvi.mask = LVIF_TEXT | LVIF_PARAM;
      lvi.cchTextMax = MAX_PATH;
      lvi.pszText = stmp;
      lvi.lParam = (LPARAM)pe32.th32ProcessID;

      lvi.iSubItem = 0;
      _stprintf_s(stmp, MAX_PATH, L"%u", pe32.th32ProcessID);
      ListView_DlgInsertItem(hWnd, IDC_PROCLIST, &lvi);

      lvi.mask = LVIF_TEXT;
      lvi.iSubItem = 1;
      _stprintf_s(stmp, MAX_PATH, L"%s", pe32.szExeFile);
      ListView_DlgSetItem(hWnd, IDC_PROCLIST, &lvi);

      hProcess = OpenProcess(PAA, 0, pe32.th32ProcessID);

      if (!hProcess)
      {
        Idx++;
        continue;
      }

      GetModuleFileNameEx(hProcess, NULL, stmp, MAX_PATH);

      lvi.iSubItem = 2;
      ListView_DlgSetItem(hWnd, IDC_PROCLIST, &lvi);
      CloseHandle(hProcess);
      Idx++;

    } while (Process32Next(hSnap, &pe32));
  }
  else
    MessageBox(hWnd, L"枚举进程失败!", L"错误", 0);

  CloseHandle(hSnap);
  return;
}

void InjectionSoftRazor(DWORD Pid)
{
  HANDLE hProcess = OpenProcess(PAA, 0, Pid);
  HANDLE hThread;
  PVOID lpCode, lpData;
  DWORD i, dwTemp;
  USHORT wStrLen;
  WCHAR stmp[MAX_PATH];

  if (!hProcess)
  {
    swprintf_s(stmp, MAX_PATH, L"打开进程[%u]失败!", Pid);
    MessageBox(0, stmp, L"错误", 0);
    return;
  }

  lpCode = VirtualAllocEx(hProcess, 0, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_EXECUTE_READWRITE);
  if (!lpCode)
  {
    CloseHandle(hProcess);
    MessageBox(0, L"申请内存失败[0]!", L"错误", 0);
    return;
  }

  lpData = VirtualAllocEx(hProcess, 0, 4096, MEM_COMMIT | MEM_RESERVE, PAGE_READWRITE);
  if (!lpData)
  {
    VirtualFreeEx(hProcess, lpCode, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    MessageBox(0, L"申请内存失败[1]!", L"错误", 0);
    return;
  }

  if (!WriteProcessMemory(hProcess, lpCode, LoadSoftRazor, 4096, &i))
  {
    VirtualFreeEx(hProcess, lpCode, 0, MEM_RELEASE);
    VirtualFreeEx(hProcess, lpData, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    MessageBox(0, L"写入内存失败[0]!", L"错误", 0);
    return;
  }

  GetModuleFileNameW(NULL, stmp, MAX_PATH);
  (wcsrchr(stmp, L'\\'))[1] = 0;
  wcscat_s(stmp, MAX_PATH, L"srdbg32.dll\0");

  wStrLen = wcslen(stmp) * sizeof(WCHAR);
  dwTemp = wStrLen + sizeof(WCHAR);

  WriteProcessMemory(hProcess, lpData, &wStrLen, sizeof(USHORT), &i);   //Write Length

  WriteProcessMemory(hProcess, MAKE_UINTPTR(lpData, 2), &dwTemp, sizeof(USHORT), &i);     //Write MaximumLength

  dwTemp = (DWORD)MAKE_UINTPTR(lpData, 8);
  WriteProcessMemory(hProcess, MAKE_UINTPTR(lpData, 4), &dwTemp, sizeof(LPVOID), &i);     //Write Buffer Address

  WriteProcessMemory(hProcess, MAKE_UINTPTR(lpData, 8), stmp, MAX_PATH * sizeof(WCHAR), &i);     //Write String
  hThread = CreateRemoteThread(hProcess, 0, 0, (LPTHREAD_START_ROUTINE)lpCode, lpData, 0, &i);

  if (!hThread)
  {
    VirtualFreeEx(hProcess, lpCode, 0, MEM_RELEASE);
    VirtualFreeEx(hProcess, lpData, 0, MEM_RELEASE);
    CloseHandle(hProcess);
    MessageBox(0, L"创建远线程失败!", L"错误", 0);
    return;
  }

  WaitForSingleObject(hThread, INFINITE);
  VirtualFreeEx(hProcess, lpCode, 0, MEM_RELEASE);
  VirtualFreeEx(hProcess, lpData, 0, MEM_RELEASE);
  CloseHandle(hThread);
  CloseHandle(hProcess);
  MessageBox(0, L"创建成功!", L"提示", 0);
}