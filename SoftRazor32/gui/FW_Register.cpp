#include "../sr_pch.h"

/*
  Float Window - Register Window
*/

FW_RegWnd::~FW_RegWnd()
{
  if (this->m_hWnd)
  {
    DestroyWindow(this->m_hWnd);
    this->m_hWnd = NULL;
  }
}

HWND FW_RegWnd::CreateForm()
{
  if (!m_hOwner) return NULL;

  if (m_hWnd)
  {
    SetTop();
    return m_hWnd;
  }

  m_hWnd = CreateDialogParam(mf->MdlBase, MAKEINTRESOURCE(FWND_REG), m_hOwner, this->fw_regWnd, LPARAM(this));
  if (m_hWnd) ShowWindow(m_hWnd, SW_SHOW);
  return m_hWnd;
}

void FW_RegWnd::UpdateThreadInfo()
{
  if (!m_hWnd) return;
  ComboBox_DlgDeleteAll(m_hWnd, IDC_TCB);

  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  THREADENTRY32 te;
  WCHAR stmp[32];

  if (hSnapshot == INVALID_HANDLE_VALUE) return;

  te.dwSize = sizeof(THREADENTRY32);

  if (Thread32First(hSnapshot, &te))
  {
    do 
    {
      if (te.th32OwnerProcessID == GetCurrentProcessId())
      {
        if (GetThreadState(te.th32OwnerProcessID, te.th32ThreadID) == TS_Suspended)
        {
          swprintf_s(stmp, 32, L"%u [%Xh]", te.th32ThreadID, te.th32ThreadID);
          SendDlgItemMessage(m_hWnd, IDC_TCB, CB_ADDSTRING, 0, (LPARAM)stmp);
        }
      }
    } while (Thread32Next(hSnapshot,&te));
  }

  CloseHandle(hSnapshot);
}

void FW_RegWnd::UpdateRegistersByHandle(HANDLE hThread)
{
  if (!m_hWnd || !hThread) return;
  UINT i;
  WCHAR stmp[64];
  DWORD dBase;
  DWORD dLimit;
  BOOL Is4KB;
  CONTEXT ct;

  ct.ContextFlags = CONTEXT_FULL;

  if (!GetThreadContext(hThread, &ct)) return;

  /* ax */
  swprintf_s(stmp, 64, HEXFORMAT, ct.Eax);
  SendDlgItemMessage(m_hWnd, SE_AX, WM_SETTEXT, 0, (LPARAM)stmp);

  swprintf_s(stmp, 64, HEXFORMAT, ct.Ecx);
  SendDlgItemMessage(m_hWnd, SE_CX, WM_SETTEXT, 0, (LPARAM)stmp);

  swprintf_s(stmp, 64, HEXFORMAT, ct.Edx);
  SendDlgItemMessage(m_hWnd, SE_DX, WM_SETTEXT, 0, (LPARAM)stmp);

  swprintf_s(stmp, 64, HEXFORMAT, ct.Ebx);
  SendDlgItemMessage(m_hWnd, SE_BX, WM_SETTEXT, 0, (LPARAM)stmp);

  /* sp */
  swprintf_s(stmp, 64, HEXFORMAT, ct.Esp);
  SendDlgItemMessage(m_hWnd, SE_SP, WM_SETTEXT, 0, (LPARAM)stmp);

  swprintf_s(stmp, 64, HEXFORMAT, ct.Ebp);
  SendDlgItemMessage(m_hWnd, SE_BP, WM_SETTEXT, 0, (LPARAM)stmp);

  swprintf_s(stmp, 64, HEXFORMAT, ct.Esi);
  SendDlgItemMessage(m_hWnd, SE_SI, WM_SETTEXT, 0, (LPARAM)stmp);

  swprintf_s(stmp, 64, HEXFORMAT, ct.Edi);
  SendDlgItemMessage(m_hWnd, SE_DI, WM_SETTEXT, 0, (LPARAM)stmp);

  /* ip */
  swprintf_s(stmp, 64, HEXFORMAT, ct.Eip);
  SendDlgItemMessage(m_hWnd, SE_IP, WM_SETTEXT, 0, (LPARAM)stmp);

  /* sr */
  swprintf_s(stmp, 64, L"%04X", ct.SegCs);
  SendDlgItemMessage(m_hWnd, SSS_CS, WM_SETTEXT, 0, (LPARAM)stmp);

  if (GetThreadDescriptor(hThread, ct.SegCs, &dBase, &dLimit, &Is4KB))
  {
    if (Is4KB)
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,4K", dBase, dLimit);
    else
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,1B", dBase, dLimit);

    SendDlgItemMessage(m_hWnd, SSV_CS, WM_SETTEXT, 0, (LPARAM)stmp);
  }
  else
    SendDlgItemMessage(m_hWnd, SSV_CS, WM_SETTEXT, 0, (LPARAM)T_UNKN8);

  swprintf_s(stmp, 64, L"%04X", ct.SegDs);
  SendDlgItemMessage(m_hWnd, SSS_DS, WM_SETTEXT, 0, (LPARAM)stmp);

  if (GetThreadDescriptor(hThread, ct.SegDs, &dBase, &dLimit, &Is4KB))
  {
    if (Is4KB)
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,4K", dBase, dLimit);
    else
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,1B", dBase, dLimit);

    SendDlgItemMessage(m_hWnd, SSV_DS, WM_SETTEXT, 0, (LPARAM)stmp);
  }
  else
    SendDlgItemMessage(m_hWnd, SSV_DS, WM_SETTEXT, 0, (LPARAM)T_UNKN8);

  swprintf_s(stmp, 64, L"%04X", ct.SegSs);
  SendDlgItemMessage(m_hWnd, SSS_SS, WM_SETTEXT, 0, (LPARAM)stmp);

  if (GetThreadDescriptor(hThread, ct.SegSs, &dBase, &dLimit, &Is4KB))
  {
    if (Is4KB)
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,4K", dBase, dLimit);
    else
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,1B", dBase, dLimit);

    SendDlgItemMessage(m_hWnd, SSV_SS, WM_SETTEXT, 0, (LPARAM)stmp);
  }
  else
    SendDlgItemMessage(m_hWnd, SSV_SS, WM_SETTEXT, 0, (LPARAM)T_UNKN8);

  swprintf_s(stmp, 64, L"%04X", ct.SegEs);
  SendDlgItemMessage(m_hWnd, SSS_ES, WM_SETTEXT, 0, (LPARAM)stmp);

  if (GetThreadDescriptor(hThread, ct.SegEs, &dBase, &dLimit, &Is4KB))
  {
    if (Is4KB)
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,4K", dBase, dLimit);
    else
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,1B", dBase, dLimit);

    SendDlgItemMessage(m_hWnd, SSV_ES, WM_SETTEXT, 0, (LPARAM)stmp);
  }
  else
    SendDlgItemMessage(m_hWnd, SSV_ES, WM_SETTEXT, 0, (LPARAM)T_UNKN8);

  swprintf_s(stmp, 64, L"%04X", ct.SegFs);
  SendDlgItemMessage(m_hWnd, SSS_FS, WM_SETTEXT, 0, (LPARAM)stmp);

  if (GetThreadDescriptor(hThread, ct.SegFs, &dBase, &dLimit, &Is4KB))
  {
    if (Is4KB)
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,4K", dBase, dLimit);
    else
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,1B", dBase, dLimit);

    SendDlgItemMessage(m_hWnd, SSV_FS, WM_SETTEXT, 0, (LPARAM)stmp);
  }
  else
    SendDlgItemMessage(m_hWnd, SSV_FS, WM_SETTEXT, 0, (LPARAM)T_UNKN8);

  swprintf_s(stmp, 64, L"%04X", ct.SegGs);
  SendDlgItemMessage(m_hWnd, SSS_GS, WM_SETTEXT, 0, (LPARAM)stmp);

  if (GetThreadDescriptor(hThread, ct.SegGs, &dBase, &dLimit, &Is4KB))
  {
    if (Is4KB)
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,4K", dBase, dLimit);
    else
      swprintf_s(stmp, 64, L"B:%08X,L:%05X,1B", dBase, dLimit);

    SendDlgItemMessage(m_hWnd, SSV_GS, WM_SETTEXT, 0, (LPARAM)stmp);
  }
  else
    SendDlgItemMessage(m_hWnd, SSV_GS, WM_SETTEXT, 0, (LPARAM)T_UNKN8);

  swprintf_s(stmp, 64, HEXFORMAT, ct.EFlags);
  SendDlgItemMessage(m_hWnd, SE_EFL, WM_SETTEXT, 0, (LPARAM)stmp);

  for (i = 0; i < 32; i++)
  {
    if (ct.EFlags & (0x00000001U << i))
      SendDlgItemMessage(m_hWnd, SE_F0 + i, WM_SETTEXT, 0, (LPARAM)L"1");
    else
      SendDlgItemMessage(m_hWnd, SE_F0 + i, WM_SETTEXT, 0, (LPARAM)L"0");
  }
}

void FW_RegWnd::UpdateRegistersById(DWORD TID)
{
  HANDLE hThread = OpenThread(THREAD_GET_CONTEXT | THREAD_QUERY_INFORMATION, FALSE, TID);

  if (!hThread) return;

  UpdateRegistersByHandle(hThread);
  CloseHandle(hThread);
}

void FW_RegWnd::ResetRegistersText()
{
  if (!m_hWnd) return;
  UINT i;

  SendDlgItemMessage(m_hWnd, SE_AX, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SE_CX, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SE_DX, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SE_BX, WM_SETTEXT, 0, (LPARAM)T_UNKN8);

  SendDlgItemMessage(m_hWnd, SE_SP, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SE_BP, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SE_SI, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SE_DI, WM_SETTEXT, 0, (LPARAM)T_UNKN8);


  SendDlgItemMessage(m_hWnd, SI_AX, WM_SETTEXT, 0, (LPARAM)T_NULL);
  SendDlgItemMessage(m_hWnd, SI_CX, WM_SETTEXT, 0, (LPARAM)T_NULL);
  SendDlgItemMessage(m_hWnd, SI_DX, WM_SETTEXT, 0, (LPARAM)T_NULL);
  SendDlgItemMessage(m_hWnd, SI_BX, WM_SETTEXT, 0, (LPARAM)T_NULL);

  SendDlgItemMessage(m_hWnd, SI_SP, WM_SETTEXT, 0, (LPARAM)T_NULL);
  SendDlgItemMessage(m_hWnd, SI_BP, WM_SETTEXT, 0, (LPARAM)T_NULL);
  SendDlgItemMessage(m_hWnd, SI_SI, WM_SETTEXT, 0, (LPARAM)T_NULL);
  SendDlgItemMessage(m_hWnd, SI_DI, WM_SETTEXT, 0, (LPARAM)T_NULL);


  SendDlgItemMessage(m_hWnd, SE_IP, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SI_MODULE, WM_SETTEXT, 0, (LPARAM)T_NULL);

  SendDlgItemMessage(m_hWnd, SSS_CS, WM_SETTEXT, 0, (LPARAM)T_UNKN4);
  SendDlgItemMessage(m_hWnd, SSS_DS, WM_SETTEXT, 0, (LPARAM)T_UNKN4);
  SendDlgItemMessage(m_hWnd, SSS_SS, WM_SETTEXT, 0, (LPARAM)T_UNKN4);
  SendDlgItemMessage(m_hWnd, SSS_ES, WM_SETTEXT, 0, (LPARAM)T_UNKN4);
  SendDlgItemMessage(m_hWnd, SSS_FS, WM_SETTEXT, 0, (LPARAM)T_UNKN4);
  SendDlgItemMessage(m_hWnd, SSS_GS, WM_SETTEXT, 0, (LPARAM)T_UNKN4);

  SendDlgItemMessage(m_hWnd, SSV_CS, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SSV_DS, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SSV_SS, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SSV_ES, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SSV_FS, WM_SETTEXT, 0, (LPARAM)T_UNKN8);
  SendDlgItemMessage(m_hWnd, SSV_GS, WM_SETTEXT, 0, (LPARAM)T_UNKN8);

  SendDlgItemMessage(m_hWnd, SE_EFL, WM_SETTEXT, 0, (LPARAM)T_UNKN8);

  for (i = 0; i < 32; i++)
  {
    SendDlgItemMessage(m_hWnd, SE_F0 + i, WM_SETTEXT, 0, (LPARAM)T_UNKN1);
  }
}

INT_PTR CALLBACK FW_RegWnd::fw_regWnd(HWND hDlg, UINT msg, WPARAM wParam, LPARAM lParam)
{
  FW_RegWnd * This = (FW_RegWnd *)GetClassPointer(hDlg);

  switch (msg)
  {
  case WM_INITDIALOG:
    This = (FW_RegWnd *)lParam;
    AddClassTable(hDlg, This);
    This->m_hWnd = hDlg;
    This->UpdateThreadInfo();
    break;
  case WM_COMMAND:
  {
    WORD wid = LOWORD(wParam);
    WORD wnc = HIWORD(wParam);

    switch (wid)    //控件ID
    {
    case IDC_UTL:   //更新线程组合框
      This->UpdateThreadInfo();
      break;
    case IDC_TCB:   //线程被更改
      if (wnc == CBN_SELCHANGE)
      {
        This->ResetRegistersText();

        int slen = SendDlgItemMessage(hDlg, IDC_TCB, WM_GETTEXTLENGTH, 0, 0);
        PWCHAR pstmp;
        DWORD tid;

        if (slen <= 0) break;
        pstmp = (PWCHAR)calloc(slen + 2, sizeof(WCHAR));
        if (!pstmp) break;

        SendDlgItemMessage(hDlg, IDC_TCB, WM_GETTEXT, WPARAM(slen + 2), (LPARAM)pstmp);

        _stscanf_s(pstmp, L"%u ", &tid);
        free(pstmp);
        if (tid == 0) break;

        if (GetThreadState(GetCurrentProcessId(), tid) != TS_Suspended) break;

        This->UpdateRegistersById(tid);
      }
      break;
    default:
      return FALSE;
    }
  }
    break;
  case WM_CLOSE:
    DestroyWindow(hDlg);
    break;
  case WM_DESTROY:
    DelClassTable(hDlg);
    This->m_hWnd = NULL;
    break;
  default:
    return FALSE;   //未处理的消息
  }

  return TRUE;  //已处理的消息
}