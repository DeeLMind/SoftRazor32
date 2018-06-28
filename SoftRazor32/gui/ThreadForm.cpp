#include "../sr_pch.h"

MC_Thread * MC_Thread::This = NULL;

MC_Thread::~MC_Thread()
{
  if (this->m_hWnd)
  {
    DestroyWindow(this->m_hWnd);
    this->m_hWnd = NULL;
  }
  This = NULL;
}

HWND MC_Thread::CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style)
{
  if (this->m_hWnd)
  {
    this->SetTop();
    return this->m_hWnd;
  }

  MDICREATESTRUCT MCS;

  MCS.szClass = (PWCHAR)mf->rcls_mc_thread;
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

LRESULT CALLBACK MC_Thread::threadWndProc(HWND phWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
  case WM_CREATE:
  {
    This->hw_thlv = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, LN_THREAD, LV_STYLE, 0, 0, 0, 0, phWnd, NULL, DllBaseAddr, 0);
    ListView_SetExtendedListViewStyle(This->hw_thlv, LVS_EX_FULLROWSELECT);

    This->PM_3 = GetSubMenu(MainForm::hm_pop, 3);

    This->threadInsertColumn();
    This->UpdateThreadList();
  }
    return 0;
  case WM_NOTIFY:
  {
    if (lParam == 0) break;

    LPNMHDR pNm = (LPNMHDR)lParam;

    switch (pNm->code)
    {
    case NM_CUSTOMDRAW:
    {
      LPNMLVCUSTOMDRAW lpnmcd = (LPNMLVCUSTOMDRAW)lParam;

      switch (lpnmcd->nmcd.dwDrawStage)
      {
      case CDDS_PREPAINT:
        if (pNm->hwndFrom == This->hw_thlv)
          return CDRF_NOTIFYITEMDRAW;
        break;
      case CDDS_ITEMPREPAINT:
      {
        if (pNm->hwndFrom == This->hw_thlv && lpnmcd->nmcd.lItemlParam != 0)
        {
          PLV_TYPE plv = (PLV_TYPE)lpnmcd->nmcd.lItemlParam;
          lpnmcd->clrText = plv->ForeColor;
          lpnmcd->clrTextBk = plv->BackColor;
        }
        return CDRF_DODEFAULT;
      }
        break;
      }
    }
      break;
    case LVN_DELETEALLITEMS:
      return 0;
    case LVN_DELETEITEM:
    {
      LPNMLISTVIEW pnl = (LPNMLISTVIEW)lParam;
      if (pnl->lParam) free((void *)pnl->lParam);
    }
      break;
    case NM_RCLICK:
      if (pNm->hwndFrom == This->hw_thlv)
      {
        This->iSelect = ListView_GetNextItem(This->hw_thlv, -1, LVNI_FOCUSED | LVNI_SELECTED);

        if (This->iSelect != -1)
        {
          POINT point;
          GetCursorPos(&point);
          TrackPopupMenu(This->PM_3, TPM_LEFTALIGN, point.x, point.y, 0, phWnd, NULL);
        }
      }
      break;
    }
  }
    break;
  case WM_COMMAND:
  {
    WORD wid = LOWORD(wParam);
    WORD wnc = HIWORD(wParam);

    switch (wid)
    {
    case PM3_REFRESH:
      This->UpdateThreadList();
      return 0;
    case PM3_SPTHREAD:
    {
      if (This->iSelect == -1) return 0;

      HANDLE hThread;
      PTHREAD_LV_TYPE ptlv;
      LV_ITEM lvi;

      lvi.mask = LVIF_PARAM;
      lvi.iItem = This->iSelect;
      lvi.iSubItem = 0;

      if (ListView_GetItem(This->hw_thlv, &lvi))
      {
        ptlv = (PTHREAD_LV_TYPE)lvi.lParam;

        if (ptlv->TID == tid_main)
        {
          mf->SetStateText(L"不可对调试线程进行操作!", RGB(255, 0, 0));
          return 0;
        }

        hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, ptlv->TID);

        if (hThread)
        {
          SuspendThread(hThread);
          CloseHandle(hThread);
        }
      }
      This->UpdateThreadList();
    }
      return 0;
    case PM3_RSTHREAD:
    {
      if (This->iSelect == -1) return 0;

      HANDLE hThread;
      PTHREAD_LV_TYPE ptlv;
      LV_ITEM lvi;

      lvi.mask = LVIF_PARAM;
      lvi.iItem = This->iSelect;
      lvi.iSubItem = 0;

      if (ListView_GetItem(This->hw_thlv, &lvi))
      {
        ptlv = (PTHREAD_LV_TYPE)lvi.lParam;

        if (ptlv->TID == tid_main)
        {
          mf->SetStateText(L"不可对调试线程进行操作!", RGB(255, 0, 0));
          return 0;
        }

        hThread = OpenThread(THREAD_SUSPEND_RESUME, FALSE, ptlv->TID);

        if (hThread)
        {
          ResumeThread(hThread);
          CloseHandle(hThread);
        }
      }
      This->UpdateThreadList();
    }
      return 0;
    }
  }
    break;
  case WM_SIZING:
    if (wParam >= WMSZ_LEFT && wParam <= WMSZ_BOTTOMRIGHT)
    {
      LPRECT lprc = (LPRECT)lParam;
      long Width = lprc->right - lprc->left;
      long Height = lprc->bottom - lprc->top;

      if (Width < 800) lprc->right = lprc->left + 800;
      if (Height < 400) lprc->bottom = lprc->top + 400;
    }
    return 1;
  case WM_SIZE:
    if (wParam != SIZE_MINIMIZED)
      MoveWindow(This->hw_thlv, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
    break;
  case WM_DESTROY:
    This->m_hWnd = NULL;
    This->hw_thlv = NULL;
    return 0;
  }

  return DefMDIChildProc(phWnd, Msg, wParam, lParam);
}

void MC_Thread::threadInsertColumn()
{
  LV_COLUMN lvc;
  ZeroMemory(&lvc, sizeof(LV_COLUMN));
  lvc.mask = LVCF_TEXT | LVCF_WIDTH;

  lvc.pszText = L"线程ID";
  lvc.cx = 100;
  ListView_InsertColumn(hw_thlv, 0, &lvc);

  lvc.pszText = L"线程入口";
  lvc.cx = DefWidth;
  ListView_InsertColumn(hw_thlv, 1, &lvc);

  lvc.pszText = L"TEB";
  lvc.cx = DefWidth;
  ListView_InsertColumn(hw_thlv, 2, &lvc);

  lvc.pszText = L"优先级";
  lvc.cx = DefWidth;
  ListView_InsertColumn(hw_thlv, 3, &lvc);

  lvc.pszText = L"线程状态";
  lvc.cx = DefWidth;
  ListView_InsertColumn(hw_thlv, 4, &lvc);
}

void MC_Thread::UpdateThreadList()
{
  LV_ITEM lvi;
  DWORD i = 0;
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPTHREAD, 0);
  HANDLE hThread;
  PTHREAD_LV_TYPE ptlv;
  THREADENTRY32 te;
  WCHAR stmp[20];

  if (hSnapshot == INVALID_HANDLE_VALUE) return;

  te.dwSize = sizeof(THREADENTRY32);
  iSelect = -1;
  ListView_DeleteAllItems(hw_thlv);

  if (Thread32First(hSnapshot, &te))
  {
    lvi.cchTextMax = 20;
    lvi.pszText = stmp;

    do
    {
      if (te.th32OwnerProcessID == GetCurrentProcessId())
      {
        ptlv = (PTHREAD_LV_TYPE)malloc(sizeof(THREAD_LV_TYPE));
        if (!ptlv) continue;

        lvi.iItem = i;
        lvi.mask = LVIF_TEXT | LVIF_PARAM;
        lvi.lParam = (LPARAM)ptlv;

        ptlv->LV.Item = i;
        if (te.th32ThreadID == tid_main) ptlv->LV.ForeColor = RGB(0, 255, 0);
        else ptlv->LV.ForeColor = RGB(0, 0, 0);
        ptlv->LV.BackColor = RGB(255, 255, 255);
        ptlv->TID = te.th32ThreadID;

        hThread = OpenThread(THREAD_QUERY_INFORMATION, 0, te.th32ThreadID);

        lvi.iSubItem = 0;
        swprintf_s(stmp, 20, L"%X  (%u)", te.th32ThreadID, te.th32ThreadID);
        ListView_InsertItem(hw_thlv, &lvi);

        lvi.mask = LVIF_TEXT;
        lvi.iSubItem = 4;

        switch (GetThreadState(te.th32OwnerProcessID, te.th32ThreadID))
        {
        case TS_Terminated:
          ptlv->State = TS_Terminated;
          wcscpy_s(stmp, 20, T_TS_TIM);
          break;
        case TS_Running:
          ptlv->State = TS_Running;
          wcscpy_s(stmp, 20, T_TS_RUN);
          break;
        case TS_Suspended:
          ptlv->State = TS_Suspended;
          wcscpy_s(stmp, 20, T_TS_SP);
          break;
        case TS_Wait:
          ptlv->State = TS_Wait;
          wcscpy_s(stmp, 20, T_TS_WAIT);
          break;
        default:
          ptlv->State = TS_Unknown;
          wcscpy_s(stmp, 20, T_UNKNOWN);
          break;
        }

        ListView_SetItem(hw_thlv, &lvi);

        if (!hThread) goto NextLoop;

        lvi.iSubItem = 1;
        swprintf_s(stmp, 20, HEXFORMAT, GetThreadStartAddress(hThread));
        ListView_SetItem(hw_thlv, &lvi);

        lvi.iSubItem = 2;
        swprintf_s(stmp, 20, HEXFORMAT, GetThreadEnvironmentBlockAddress(hThread));
        ListView_SetItem(hw_thlv, &lvi);

        lvi.iSubItem = 3;
        switch (GetThreadPriority(hThread))
        {
        case THREAD_PRIORITY_ABOVE_NORMAL:
          wcscpy_s(stmp, 20, T_TP_AN);
          break;
        case THREAD_PRIORITY_BELOW_NORMAL:
          wcscpy_s(stmp, 20, T_TP_BN);
          break;
        case THREAD_PRIORITY_HIGHEST:
          wcscpy_s(stmp, 20, T_TP_HE);
          break;
        case THREAD_PRIORITY_IDLE:
          wcscpy_s(stmp, 20, T_TP_IDLE);
          break;
        case THREAD_PRIORITY_LOWEST:
          wcscpy_s(stmp, 20, T_TP_LE);
          break;
        case THREAD_PRIORITY_NORMAL:
          wcscpy_s(stmp, 20, T_TP_NM);
          break;
        case THREAD_PRIORITY_TIME_CRITICAL:
          wcscpy_s(stmp, 20, T_TP_TC);
          break;
        default:
          wcscpy_s(stmp, 20, T_UNKNOWN);
        }
        ListView_SetItem(hw_thlv, &lvi);

        CloseHandle(hThread);
      NextLoop:
        i++;
      }
    } while (Thread32Next(hSnapshot, &te));
  }
  CloseHandle(hSnapshot);
}
