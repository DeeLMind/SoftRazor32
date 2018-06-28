#pragma once

#include <locale.h>
#include <stdio.h>
#include <stdlib.h>
#include <process.h>
#include <tchar.h>
#include <windows.h>
#include <commctrl.h>
#include <tlhelp32.h>
#include <Psapi.h>

#include "resource.h"

#pragma comment(linker,"/ALIGN:0x1000")
#pragma comment(linker,"/SECTION:.shcode,ERW")
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"Psapi.lib")

#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32'   name='Microsoft.Windows.Common-Controls'   version='6.0.0.0'   processorArchitecture='x86'   publicKeyToken='6595b64144ccf1df'   language='*'\"")
#elif defined _M_IA64
#pragma comment(linker,"/manifestdependency:\"type='win32'   name='Microsoft.Windows.Common-Controls'   version='6.0.0.0'   processorArchitecture='ia64'   publicKeyToken='6595b64144ccf1df'   language='*'\"")
#elif defined _M_X64
#pragma comment(linker,"/manifestdependency:\"type='win32'   name='Microsoft.Windows.Common-Controls'   version='6.0.0.0'   processorArchitecture='amd64'   publicKeyToken='6595b64144ccf1df'   language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32'   name='Microsoft.Windows.Common-Controls'   version='6.0.0.0'   processorArchitecture='*'   publicKeyToken='6595b64144ccf1df'   language='*'\"")
#endif

#ifdef _INC_COMMCTRL

#define ListView_DlgSetExtendedListViewStyle(hwnd, dlgitem, dw)\
  (DWORD)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dw)

#define ListView_DlgInsertColumn(hwnd, dlgitem, iCol, pcol) \
  (int)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_INSERTCOLUMN, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMN *)(pcol))

#define ListView_DlgDeleteAllItems(hwnd, dlgitem) \
  (BOOL)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_DELETEALLITEMS, 0, 0L)

#define ListView_DlgInsertItem(hwnd, dlgitem, pitem)   \
  (int)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_INSERTITEM, 0, (LPARAM)(const LV_ITEM *)(pitem))

#define ListView_DlgSetItem(hwnd, dlgitem, pitem) \
  (BOOL)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_SETITEM, 0, (LPARAM)(const LV_ITEM *)(pitem))

#define ListView_DlgGetItem(hwnd, dlgitem, pitem) \
  (BOOL)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_GETITEM, 0, (LPARAM)(LV_ITEM *)(pitem))

#define ListView_DlgGetNextItem(hwnd, dlgitem, i, flags) \
  (int)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_GETNEXTITEM, (WPARAM)(int)(i), MAKELPARAM((flags), 0))

#else

#error "未能成功包含commctrl.h"

#endif

#define ASM                           __declspec(naked)
#define MAKE_UINTPTR(addr, offset)    (LPVOID)((UINT_PTR)(addr) + (UINT_PTR)(offset))

extern HINSTANCE      MeInstance;
extern DWORD          PAA;

DWORD WINAPI LoadSoftRazor(LPVOID lpParam);
DWORD WINAPI LoadSoftRazor2(LPVOID lpParam);