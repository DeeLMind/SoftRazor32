#pragma once

#ifndef _SR_INC_COMMON_H_
#include "src/common.h"
#endif

#ifndef _INC_LOCALE
#include <locale.h>
#endif

#ifndef _INC_STDIO
#include <stdio.h>
#endif

#ifndef _STDINT
#include <stdint.h>
#endif

#ifndef _INC_STDLIB
#include <stdlib.h>
#endif

#ifndef _INC_STRING
#include <string.h>
#endif

#ifndef _INC_PROCESS
#include <process.h>
#endif

#ifndef _INC_TOOLHELP32
#include <tlhelp32.h>
#endif

#ifndef _PSAPI_H_
#include <Psapi.h>
#endif

#ifndef _INC_SHLWAPI
#include <shlwapi.h>
#endif

#ifndef _INC_SHELLAPI
#include <ShellAPI.h>
#endif

#include "resource.h"
#include "../CommHdr/timer.hpp"
#include "src/dmusici.h"
#include "src/g_define.h"
#include "src/g_type.h"
#include "src/comcls.hpp"
#include "src/kernel.h"
#include "src/vbstruct.h"
#include "src/mspcode.h"
#include "src/md5c.h"
#include "../HMM/hmm.h"
#include "../udis86/udis86.h"
#include "../XEDParse/XEDParse.h"
#include "gui/wndcls.hpp"


#pragma comment(linker, "/ENTRY:DllEntry") 
#pragma comment(linker, "/SECTION:.mspcvm,RWE")
#pragma comment(linker, "/SECTION:.ehcode,RWE")

//Static Library
#pragma comment(lib,"user32.lib")
#pragma comment(lib,"gdi32.lib")
#pragma comment(lib,"comctl32.lib")
#pragma comment(lib,"Psapi.lib")
#pragma comment(lib,"shlwapi.lib")
#pragma comment(lib,"shell32.lib")
#pragma comment(lib,"ole32.lib")
#pragma comment(lib,"oleaut32.lib")
#pragma comment(lib,"Advapi32.lib")

//#pragma comment(lib,"librex.lib")
#pragma comment(lib,"dxguid.lib")
//#pragma comment(lib,"lib/ntdll.lib")
#pragma comment(lib,"hmm32.lib")
#pragma comment(lib,"XEDParse_x86.lib")

#pragma warning(disable:4101)
#pragma warning(disable:4996)

//解决VS2015以及以上版本的VS 找不到 __iob_func 符号的问题
#if _MSC_VER >= 1900  
static FILE _iob[] = { *stdin, *stdout, *stderr };

extern "C" FILE * __cdecl __iob_func(void)
{
  return _iob;
}
#endif


#if defined _M_IX86
#pragma comment(linker,"/manifestdependency:\"type='win32'   name='Microsoft.Windows.Common-Controls'   version='6.0.0.0'   processorArchitecture='x86'   publicKeyToken='6595b64144ccf1df'   language='*'\"")
#else
#pragma comment(linker,"/manifestdependency:\"type='win32'   name='Microsoft.Windows.Common-Controls'   version='6.0.0.0'   processorArchitecture='*'   publicKeyToken='6595b64144ccf1df'   language='*'\"")
#endif

/* extern "C" */
#ifdef __cplusplus
extern "C" {
#endif

  BOOL WINAPI DllEntry(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved);
  BOOL WINAPI _DllMainCRTStartup(HANDLE hDllHandle, DWORD dwReason, LPVOID lpreserved);
  BOOL WINAPI DllMain(HMODULE hModule, DWORD ul_reason_for_call, LPVOID lpReserved);

#ifdef __cplusplus
}
#endif

#ifdef _INC_COMMCTRL

#define ListView_InsertItemA(hwnd, pItem) (int)::SendMessageA((hwnd), LVM_INSERTITEMA, 0, (LPARAM)(const LV_ITEMA *)(pItem))

#define ListView_SetItemA(hwnd, pItem) (BOOL)::SendMessageA((hwnd), LVM_SETITEMA, 0, (LPARAM)(const LV_ITEMA *)(pItem))

#define ListView_DlgDeleteColumn(hwnd, dlgitem, iCol) \
  (BOOL)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_DELETECOLUMN, (WPARAM)(int)(iCol), 0)

#define ListView_DlgSetExtendedListViewStyle(hwnd, dlgitem, dw)\
  (DWORD)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_SETEXTENDEDLISTVIEWSTYLE, 0, dw)

#define ListView_DlgInsertColumn(hwnd, dlgitem, iCol, pcol) \
  (int)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_INSERTCOLUMN, (WPARAM)(int)(iCol), (LPARAM)(const LV_COLUMN *)(pcol))

#define ListView_DlgDeleteAllItems(hwnd, dlgitem) \
  (BOOL)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_DELETEALLITEMS, 0, 0L)

#define ListView_DlgInsertItemA(hwnd, dlgitem, pitem)   \
  (int)::SendDlgItemMessageA((hwnd),(int)(dlgitem),LVM_INSERTITEMA, 0, (LPARAM)(const LVITEMA *)(pitem))

#define ListView_DlgInsertItemW(hwnd, dlgitem, pitem)   \
  (int)::SendDlgItemMessageW((hwnd),(int)(dlgitem),LVM_INSERTITEMW, 0, (LPARAM)(const LVITEMW *)(pitem))

#define ListView_DlgInsertItem(hwnd, dlgitem, pitem)   \
  (int)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_INSERTITEM, 0, (LPARAM)(const LV_ITEM *)(pitem))

#define ListView_DlgSetItemA(hwnd, dlgitem,pitem) \
  (BOOL)::SendDlgItemMessageA((hwnd),(int)(dlgitem),LVM_SETITEMA, 0, (LPARAM)(const LVITEMA *)(pitem))

#define ListView_DlgSetItemW(hwnd, dlgitem,pitem) \
  (BOOL)::SendDlgItemMessageW((hwnd),(int)(dlgitem),LVM_SETITEMW, 0, (LPARAM)(const LVITEMW *)(pitem))

#define ListView_DlgSetItem(hwnd, dlgitem,pitem) \
  (BOOL)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_SETITEM, 0, (LPARAM)(const LV_ITEM *)(pitem))

#define ListView_DlgGetItemCount(hwnd, dlgitem) \
  (int)::SendDlgItemMessage((hwnd),(int)(dlgitem),LVM_GETITEMCOUNT, 0, 0L)

#define TreeView_InsertItemA(hwnd, lpis) \
  (HTREEITEM)::SendMessageA((hwnd), TVM_INSERTITEMA, 0, (LPARAM)(LPTV_INSERTSTRUCT)(lpis))

#define ComboBox_DeleteAll(hwnd) \
      while ((int)SNDMSG((HWND)(hwnd), CB_DELETESTRING , 0, 0) > 0) {} 

#define ComboBox_DlgDeleteAll(hwnd, dlgitem) \
      while ((int)::SendDlgItemMessage((HWND)(hwnd), (int)(dlgitem), CB_DELETESTRING , 0, 0) > 0) {} 

#define _LoadIcon_16x16(iIcon, hIcon, bret) \
  hIcon = (HICON)::LoadImage(DllBaseAddr, MAKEINTRESOURCE(iIcon),IMAGE_ICON,16,16,LR_DEFAULTCOLOR | LR_SHARED | LR_CREATEDIBSECTION); \
  if (!hIcon) bret = FALSE 

#define SR_TRANSPARENT              (COLORREF)0xFFFFFFFF

/*
#define ALLOC_PARAM(tvis,ptv,ncode,idx) \
  ptv = (PVB_TV_TYPE)calloc(1,sizeof(VB_TV_TYPE)); \
  if (ptv) { \
  ptv->TV.Sign = MY_MAGIC; \
  ptv->TV.NCode = (WORD)ncode; \
  ptv->TV.Index = (WORD)idx; } \
  tvis.item.lParam = (LPARAM)ptv
  */

#else
#error "未能成功包含commctrl.h"
#endif

#define VALMIN(inta,intb)       ((UINT)(inta) < (UINT)(intb))?(inta):(intb)
#define VALMAX(inta,intb)       ((UINT)(inta) > (UINT)(intb))?(inta):(intb)
#define ALLOC_SEG(sn)           __declspec(allocate(sn)) 
#define TEXPECT_NOACCESS(EC)    (EC == EXCEPTION_ACCESS_VIOLATION)?EXCEPTION_EXECUTE_HANDLER:EXCEPTION_CONTINUE_SEARCH
#define TF_SUCCESS(val)         ((val) == 0)
#define SET_FLAG(val, flag)     ((val) |= (flag))
#define SET_NOFL(val, flag)     ((val) &= ~(flag))
#define CHK_FLAG(val, flag)     (((val) & (flag)) == flag)
#define CHK_NOFL(val, flag)     (((val) & (flag)) != flag)
#define CHK_EMPTY(val, flag)    (((val) & (flag)) == 0)
#define CHK_NOTEMPTY(val, flag) (((val) & (flag)) != 0)

#define MAKE_UINTPTR(p1, p2)    ((UINT_PTR)(p1) + (UINT_PTR)(p2))


#define CHKPE_NOCHKMACHINE      0x00000001U
#define CHKPE_NOCHKOPTHDR       0x00000002U
#define CHKPE_NOCHKSECTION      0x00000004U

#define MFUNC_PRINTGUIDW(_lpwcs, _wcsmax, _guid) \
  swprintf_s(_lpwcs, _wcsmax, L"{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", _guid##.Data1, \
        _guid##.Data2, _guid##.Data3, \
        _guid##.Data4[0], _guid##.Data4[1], \
        _guid##.Data4[2], _guid##.Data4[3], \
        _guid##.Data4[4], _guid##.Data4[5], \
        _guid##.Data4[6], _guid##.Data4[7])

#define MFUNC_PRINTLPGUIDW(_lpwcs, _wcsmax, _lpguid) \
  swprintf_s(_lpwcs, _wcsmax, L"{%08X-%04X-%04x-%02X%02X-%02X%02X%02X%02X%02X%02X}", ((LPGUID)(_lpguid))->Data1, \
        ((LPGUID)(_lpguid))->Data2, ((LPGUID)(_lpguid))->Data3, \
        ((LPGUID)(_lpguid))->Data4[0], ((LPGUID)(_lpguid))->Data4[1], \
        ((LPGUID)(_lpguid))->Data4[2], ((LPGUID)(_lpguid))->Data4[3], \
        ((LPGUID)(_lpguid))->Data4[4], ((LPGUID)(_lpguid))->Data4[5], \
        ((LPGUID)(_lpguid))->Data4[6], ((LPGUID)(_lpguid))->Data4[7])

extern const GUID               vbclsid;
extern GlobalConfig             GC;

extern HMODULE                  DllBaseAddr;
extern DWORD                    PAA;      //Process All Access
extern DWORD                    TAA;      //Thread All Access
extern DWORD                    LastCode;
extern WORD                     NtVer;
extern WCHAR                    DllDir[MAX_PATH];

/*反汇编语法颜色*/
extern COLORREF     crAsmFGC[128];
extern COLORREF     crAsmBGC[128];

//About Thread
extern HANDLE       ht_main;
extern UINT         tid_main;

extern MainForm   * mf;

extern bool         mdlASCEND;

//extern func


extern int ImageList_Add32bpp(HIMAGELIST hImageList, HINSTANCE hInst, UINT nId, UINT uType);
extern BOOL RegisterAllClass();
extern BOOL UnregisterAllClass();
extern BOOL LoadAllMenu();
extern BOOL DestroyAllMenu();
extern BOOL LoadAllImage();
extern BOOL DestroyAllImage();
extern BOOL InitBaseControl();
extern BOOL FreeBaseControl();

extern BOOL WINAPI LaunchDebugger(ULONG LaunchParam);
extern int SetStatusText(PWCHAR pText);
extern bool PlayMIDI(PBYTE data, UINT size);
extern void StopMIDI();
extern DWORD GetThreadInformation(DWORD Tid, THREADINFOCLASS TIC, PVOID pData, DWORD dLen);

extern COLORREF scGetForeColor(DWORD sc);
extern COLORREF scGetBackColor(DWORD sc);
extern int CALLBACK mdl_CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM SubItem);
extern void CommonUpdatelParam(HWND hlv);
extern void ShowCurrentFocus();

extern LRESULT CALLBACK MainProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);

extern BOOL CheckMessageQueue();
extern INT_PTR WINAPI GetFuncAddr(INT_PTR hMod, const PCHAR pName);
extern void TestFunc(DWORD);
extern DWORD TestExpect(DWORD ExpectCode);
extern int If_LCIDToLocaleName(LCID Locale, LPWSTR lpName, int cchName, DWORD dwFlags);
extern BOOL TryCopyWCharFromWChar(PWCHAR psDest, size_t dwMax, PWCHAR psSrc);
extern BOOL TryCopyWCharFromChar(PWCHAR psDest, size_t dwMax, PCHAR psSrc);
extern BOOL TryCatenateWCharFromWChar(PWCHAR psDest, size_t dwMax, PWCHAR psSrc);
extern BOOL TryCatenateWCharFromChar(PWCHAR psDest, size_t dwMax, PCHAR psSrc);
extern BOOL GetModuleName(PVOID dwAddr, PWCHAR oBuffer, UINT sMax);
extern BOOL CheckPEFormatByBuffer(PBYTE pBuffer, DWORD bMax, DWORD cFlag);
extern BOOL HitModule(PVOID pAddr, PUINT pBase, PUINT pSize);
extern INT_PTR GetModuleBaseAddr(PVOID pAddr);
extern BOOL GetThreadDescriptor(HANDLE hThread, DWORD dwSelector, PDWORD pBase, PDWORD pLimit, PBOOL Is4KB);
extern UINT GetModuleSize(HMODULE hMod, BOOL FormImageSize);
extern UINT GetModuleEntry(HMODULE hMod);
extern BOOL GetSectionBaseAddr(DWORD dwAddr, PDWORD opSectAddr, PDWORD opSectSize);
extern void GetFileName(PWCHAR FilePath);
extern BOOL GetSectionNameA(HMODULE hMod, PVOID pSect, PCHAR OutName, rsize_t sMax);
extern BOOL GetSectionNameW(HMODULE hMod, PVOID pSect, PWCHAR OutName, rsize_t sMax);
extern BOOL GetDataSection(HMODULE hMod, PDWORD dsAddr, __in_opt PDWORD dsSize);
extern void GetPageProtectString(DWORD PP, PWCHAR _Out, DWORD _Max);
extern DWORD GetThreadInformation(HANDLE hThread, THREADINFOCLASS TIC, PVOID pData, DWORD dLen);
extern DWORD GetThreadStartAddress(HANDLE hThread);
extern PVOID GetThreadEnvironmentBlockAddress(HANDLE hThread);
extern SRDBG_TS GetThreadState(ULONG ulPID, ULONG ulTID);
extern UINT WINAPI FindMemory(PBYTE destAddr, UINT destSize, PCMP_BYTE pCB, UINT cbCount, BOOL Absolute);
extern BOOL GetMemberNameByIndex(GUID ClsID, DWORD Idx, PWCHAR pText, DWORD tCount, PDWORD pMemberID);
extern BOOL GetEventNameByIndex(GUID ClsID, DWORD Idx, PWCHAR pText, DWORD tCount, PDWORD pMemberID);
extern BOOL GetEventNameByMemberID(GUID ClsID, DWORD MemberID, PWCHAR pText, DWORD tCount);
extern BOOL GetPropertyText(GUID guid, WORD VTOffset, PWCHAR szNote, DWORD slNote);
extern PVBHDR GetVbHeaderByModule(HMODULE hMod);
extern PIMAGE_SECTION_HEADER GetSectionsAddress(HMODULE hMod);
extern WORD GetSectionNumber(HMODULE hMod);
extern PVOID GetSectionAddrByName(HMODULE hMod, PCHAR pSecName, DWORD * pSecSize);
extern PVOID WINAPI MemMem(PBYTE pMem1, SIZE_T MemSize1, PBYTE FindMem, SIZE_T fmSize);
extern BOOL GetDataMD5Sum(PBYTE pBuffer, UINT bMax, PUNIMD5 poMD5);

extern size_t casmlen(char *src, size_t max);
extern size_t casmcpy(char *dest, char *src, size_t max);
extern int DrawCasmA(HDC hDC, PCHAR lpAsmText, size_t bufmax, LONG sspace, LPCRECT lprc, UINT format);
extern int DrawCasmW(HDC hdc, PWCHAR pCasm, size_t bufcount, LONG wspace, LPCRECT lprc, UINT format);

extern BOOL WCharTrim(PWCHAR src);
extern BOOL WCharIsHex(PWCHAR src);

extern INT_PTR CALLBACK GotoAddrProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK AsmProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK AboutProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);
extern INT_PTR CALLBACK SetDEProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam);


extern LONG NTAPI SR_SetHandler();
extern void NTAPI SR_RemoveHandler();
extern LONG NTAPI FirstExceptionHandler(PEXCEPTION_POINTERS ExceptionInfo);
extern LONG NTAPI ContinueHandler(PEXCEPTION_POINTERS ExceptionInfo);

extern inline void CRC64_Init(uint64_t * crc)
{
  *crc = UINT64_C(0xffffffffffffffff);
}

extern inline void CRC64_Final(uint64_t * crc)
{
  *crc ^= UINT64_C(0xffffffffffffffff);
}

extern void CRC64_Calc(uint64_t * crc, const uint8_t * pData, size_t dlen);

extern int pcf_init_decode_object_basic(PPDO pcdo, PVBPDI pdi, PWCHAR pstr, uint32_t slen, uint32_t mb, uint32_t ms);
extern int pcf_decode(PPDO pcdo);
extern BOOL pcf_alloc_mapping(PPDO pcdo);
extern BOOL pcf_free_mapping(PPDO pcdo);
extern int pcf_disassemble(PPDO pcdo);
extern int pcf_disasm_proc(PPDO pcdo);
extern BOOL pcf_decodedcount(PPDO pcdo, uint32_t * dednum, uint32_t * mapmax);


extern UINT XEDP_Assemble(struct XEDPARSE * pXEDP);
