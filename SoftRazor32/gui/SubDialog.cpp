#include "../sr_pch.h"

static WORD iCount = 0;
static HRSRC hRsrc = NULL;
static HGLOBAL hGlobal = NULL;
static LPVOID midBuffer = NULL;
static WCHAR AsmText[128];

void CALLBACK TimerProc(HWND hWnd, UINT nMsg, UINT nTimerid, DWORD dwTime)
{
  iCount++;
  if (iCount >= 60) iCount = 0;
  //SendDlgItemMessage(hWnd, REM_STATIC, WM_SETTEXT, 0, (LPARAM)lrct[iCount]);
}

INT_PTR CALLBACK AboutProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
  case WM_INITDIALOG:
  {
    DWORD dwSize;

    hRsrc = FindResource(DllBaseAddr, MAKEINTRESOURCE(IDR_MIDI), L"MIDI");
    if (hRsrc == NULL) return 1;
    dwSize = SizeofResource(DllBaseAddr, hRsrc);
    if (dwSize == 0) return 1;
    hGlobal = LoadResource(DllBaseAddr, hRsrc);
    if (hGlobal == NULL) return 1;
    midBuffer = LockResource(hGlobal);
    if (midBuffer == NULL) return 1;
    PlayMIDI((PBYTE)midBuffer, dwSize);
  }
    return 1;
  case WM_CTLCOLORSTATIC:
    if ((HWND)lParam == GetDlgItem(hDlg, IDC_DONATEME))
    {
      SetBkMode((HDC)wParam, TRANSPARENT);
      SetTextColor((HDC)wParam, RGB(0, 0, 250));//设置字体颜色
      return (INT_PTR)GetStockObject(NULL_BRUSH);
    }
    break;
  case WM_COMMAND:
  {
    if (HIWORD(wParam) == STN_CLICKED)
    {
      if (LOWORD(wParam) == IDC_DONATEME)
      {
        ShellExecute(hDlg, L"open", L"https://me.alipay.com/csersoft", T_NULL, T_NULL, SW_SHOW);
        return 0;
      }
    }
    if (LOWORD(wParam) == ABOUT_OK)
    {
      EndDialog(hDlg, 1);
      return 0;
    }
  }
    break;
  case WM_CLOSE:
    EndDialog(hDlg, 1);
    return 0;
  case WM_DESTROY:
    StopMIDI();
    if (midBuffer) UnlockResource(midBuffer);
    if (hGlobal) FreeResource(hGlobal);
    midBuffer = NULL;
    hGlobal = NULL;
    KillTimer(hDlg, 1);
    return 0;
  }
  return 0;
}

INT_PTR CALLBACK GotoAddrProc(HWND hDlg,UINT Msg,WPARAM wParam,LPARAM lParam)
{
  switch (Msg)
  {
  case WM_INITDIALOG:
    SendDlgItemMessage(hDlg,IDC_EXPEDIT,EM_SETLIMITTEXT,512,0);
    SetFocus(GetDlgItem(hDlg,IDC_EXPEDIT));
    return 1;
  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDC_GOTOBTN:
    {
      int tlen = GetWindowTextLength(GetDlgItem(hDlg, IDC_EXPEDIT));
      if (tlen <= 0) break;
      PWCHAR stmp = (PWCHAR)malloc((tlen + 1) * sizeof(WCHAR));
      if (!tlen) break;

      GetWindowText(GetDlgItem(hDlg, IDC_EXPEDIT), stmp, tlen + 1);
      WCharTrim(stmp);
      if (tlen = wcslen(stmp) <= 8)
      {
        if (WCharIsHex(stmp))     //是16进制
        {
          int addr;
          MEMORY_BASIC_INFORMATION mbi;

          if (_stscanf_s(stmp, L"%x", &addr) != 1)
          {
            SetWindowText(GetDlgItem(hDlg, IDC_GTAD_STATUS), L"无效参数!");
            free(stmp);
            break;
          }
          VirtualQuery((LPVOID)addr, &mbi, sizeof(MEMORY_BASIC_INFORMATION));
          if (mbi.State != MEM_COMMIT)
          {
            SetWindowText(GetDlgItem(hDlg, IDC_GTAD_STATUS), L"无效的地址!");
            free(stmp);
            break;
          }
          mf->mc_disasm->GotoAddress((PBYTE)addr, 0);
          free(stmp);
          EndDialog(hDlg, 1);
        }
      }
    }
      break;
    case IDC_GOBACKBTN:
      EndDialog(hDlg,0);
      break;
    }
    break;
  case WM_CTLCOLORSTATIC:
    if ((HWND)lParam == GetDlgItem(hDlg, IDC_GTAD_STATUS))
    {
      SetBkMode((HDC)wParam, TRANSPARENT);
      SetTextColor((HDC)wParam, RGB(250, 32, 32));//设置字体颜色
      return (INT_PTR)GetStockObject(NULL_BRUSH);
    }
    break;
  case WM_CLOSE:
    EndDialog(hDlg,1);
    break;
  }
  return 0;
}

INT_PTR CALLBACK AsmProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
  case WM_INITDIALOG:
  {
    PVOID cAddr;

    SetFocus(GetDlgItem(hDlg, IDC_EDIT_ASM));
    cAddr = mf->mc_disasm->GetSelLineAddress();
    if (!cAddr) EndDialog(hDlg, 0);
    SendDlgItemMessage(hDlg, IDC_EDIT_ASM, EM_SETLIMITTEXT, 126,0);
    //SendDlgItemMessage(hDlg, IDC_EDIT_ASM, WM_SETFONT, (WPARAM)mf->hf_lar, 1);
    SendDlgItemMessage(hDlg, IDC_ASM_MSG, WM_SETFONT, (WPARAM)mf->hf_tam, 1);
    swprintf_s(AsmText, 128, L"在0x%08X处汇编以下代码(原指令长度:%u):", cAddr, mf->mc_disasm->GetOpLenByAddress(cAddr));
    SendDlgItemMessage(hDlg, IDC_ASM_TITLE, WM_SETTEXT, 0, (LPARAM)AsmText);
    mf->mc_disasm->GetAsmTextByAddress(cAddr, AsmText, 128);
    SendDlgItemMessage(hDlg, IDC_EDIT_ASM, WM_SETTEXT, 0, (LPARAM)AsmText);

  }
    return 1;
  case WM_COMMAND:
    switch (LOWORD(wParam))
    {
    case IDOK:
    {
      int asml;
      PVOID cAddr = mf->mc_disasm->GetSelLineAddress();
      PWCHAR WAsmText;
      PCHAR MAsmText;
      XEDPARSE XEDP;
      
      asml = GetWindowTextLength(GetDlgItem(hDlg, IDC_EDIT_ASM));
      if (!asml || !cAddr)
      {
        EndDialog(hDlg, IDCANCEL);
        break;
      }
        
      WAsmText = (PWCHAR)malloc((asml + 2) * sizeof(WCHAR));
      MAsmText = (PCHAR)malloc(asml + 32);

      if (!WAsmText || !MAsmText)
      {
        if (WAsmText) free(WAsmText);
        if (MAsmText) free(MAsmText);
        EndDialog(hDlg, 1);
        break;
      }

      GetWindowText(GetDlgItem(hDlg, IDC_EDIT_ASM), WAsmText, asml + 2);
      if (wcscmp(AsmText, WAsmText) == 0) /* 未改变汇编代码 */
      {
        free(WAsmText);
        free(MAsmText);
        EndDialog(hDlg, IDOK);
        break;
      }

      free(WAsmText);
      GetWindowTextA(GetDlgItem(hDlg, IDC_EDIT_ASM), MAsmText, asml + 32);

      XEDP.x64 = false;
      XEDP.cip = ULONGLONG(cAddr);
      XEDP.cbUnknown = NULL;
      strcpy_s(XEDP.instr, XEDPARSE_MAXBUFSIZE, MAsmText);

      if (XEDP_Assemble(&XEDP) == 0)     //汇编到结构中失败
      {
        if (strlen(XEDP.error))
          SetWindowTextA(GetDlgItem(hDlg, IDC_ASM_MSG), XEDP.error);
        else
          SetWindowText(GetDlgItem(hDlg, IDC_ASM_MSG), L"汇编字节码失败!");
        break;
      }

      free(MAsmText);
      mf->mc_disasm->AssembleOfAddress(cAddr, &XEDP);
      EndDialog(hDlg, IDOK);
    }
      break;
    case IDCANCEL:
      EndDialog(hDlg, 0);
      break;
    }
    break;
  case WM_CLOSE:
    EndDialog(hDlg, 0);
    break;
  }
  return 0;
}

INT_PTR CALLBACK SetDEProc(HWND hDlg, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  static LPARAM mlParam;

  switch (Msg)
  {
  case WM_INITDIALOG:
    SendDlgItemMessage(hDlg, IDC_CB_EAV, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EAV, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EAV, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 1) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EABE, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EABE, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EABE, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 2) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EBP, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EBP, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EBP, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 4) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EDM, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EDM, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EDM, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 8) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EFDO, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EFDO, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EFDO, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 16) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EFDBZ, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EFDBZ, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EFDBZ, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 32) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EFIR, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EFIR, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EFIR, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 64) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EFIO, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EFIO, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EFIO, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 128) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EFO, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EFO, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EFO, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 256) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EFSC, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EFSC, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EFSC, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 512) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EFU, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EFU, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EFU, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 1024) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EII, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EII, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EII, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 2048) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EIPE, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EIPE, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EIPE, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 4096) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EIDBZ, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EIDBZ, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EIDBZ, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 8192) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EIO, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EIO, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EIO, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 16384) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EID, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EID, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EID, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 32768) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_ENE, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_ENE, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_ENE, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 65536) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_EPI, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_EPI, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_EPI, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 131072) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_ESS, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_ESS, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_ESS, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 262144) ? 0 : 1, 0);

    SendDlgItemMessage(hDlg, IDC_CB_ESO, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_ASK);
    SendDlgItemMessage(hDlg, IDC_CB_ESO, CB_ADDSTRING, 0, (LPARAM)(const PWCHAR)T_IGNORE);
    SendDlgItemMessage(hDlg, IDC_CB_ESO, CB_SETCURSEL, (((lParam) ? GC.ldeval : GC.fdeval) & 524288) ? 0 : 1, 0);

    if (lParam)
      SendDlgItemMessage(hDlg, IDC_STATE, WM_SETTEXT, 0, (LPARAM)L"设置最后异常的处理方式.");
    else
      SendDlgItemMessage(hDlg, IDC_STATE, WM_SETTEXT, 0, (LPARAM)L"设置最先异常的处理方式.");

    mlParam = lParam;

    return 1;
  case WM_COMMAND:
  {
    DWORD ltmp;
    WORD wid = LOWORD(wParam);
    WORD wnc = HIWORD(wParam);
    
    if (wid == IDOK)
    {
      ltmp = 0;

      if (SendDlgItemMessage(hDlg, IDC_CB_EAV, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 1;
      if (SendDlgItemMessage(hDlg, IDC_CB_EABE, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 2;
      if (SendDlgItemMessage(hDlg, IDC_CB_EBP, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 4;
      if (SendDlgItemMessage(hDlg, IDC_CB_EDM, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 8;
      if (SendDlgItemMessage(hDlg, IDC_CB_EFDO, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 16;
      if (SendDlgItemMessage(hDlg, IDC_CB_EFDBZ, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 32;
      if (SendDlgItemMessage(hDlg, IDC_CB_EFIR, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 64;
      if (SendDlgItemMessage(hDlg, IDC_CB_EFIO, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 128;
      if (SendDlgItemMessage(hDlg, IDC_CB_EFO, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 256;
      if (SendDlgItemMessage(hDlg, IDC_CB_EFSC, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 512;
      if (SendDlgItemMessage(hDlg, IDC_CB_EFU, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 1024;
      if (SendDlgItemMessage(hDlg, IDC_CB_EII, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 2048;
      if (SendDlgItemMessage(hDlg, IDC_CB_EIPE, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 4096;
      if (SendDlgItemMessage(hDlg, IDC_CB_EIDBZ, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 8192;
      if (SendDlgItemMessage(hDlg, IDC_CB_EIO, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 16384;
      if (SendDlgItemMessage(hDlg, IDC_CB_EID, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 32768;
      if (SendDlgItemMessage(hDlg, IDC_CB_ENE, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 65536;
      if (SendDlgItemMessage(hDlg, IDC_CB_EPI, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 131072;
      if (SendDlgItemMessage(hDlg, IDC_CB_ESS, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 262144;
      if (SendDlgItemMessage(hDlg, IDC_CB_ESO, CB_GETCURSEL, 0, 0) == 0)
        ltmp |= 524288;

      if (mlParam)
        GC.ldeval = ltmp;
      else
        GC.fdeval = ltmp;

      GC.SaveAllConfig();
      EndDialog(hDlg, IDOK);
    }
    else if (wid == IDCANCEL)
    {
      EndDialog(hDlg, IDCANCEL);
    }
  }
    break;
  case WM_CLOSE:
    return EndDialog(hDlg, IDCLOSE);
  }
  return 0;
}