#include "../sr_pch.h"

MC_Memory * MC_Memory::This = NULL;

MC_Memory::~MC_Memory()
{
  if (this->m_hWnd)
  {
    DestroyWindow(this->m_hWnd);
    this->m_hWnd = NULL;
  }

  if (this->PM_0)
  {
    DestroyMenu(PM_0);
    PM_0 = NULL;
  }
  This = NULL;
}

HWND MC_Memory::CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style)
{
  if (this->m_hWnd)
  {
    this->SetTop();
    return this->m_hWnd;
  }

  MDICREATESTRUCT MCS;

  MCS.szClass = (PWCHAR)mf->rcls_mc_memory;
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

LRESULT CALLBACK MC_Memory::memWndProc(HWND phWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
  case WM_CREATE:
  {
    This->memlv = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, LN_MEMORY, LV_STYLE, 0, 0, 0, 0, phWnd, NULL, DllBaseAddr, 0);
    ListView_SetExtendedListViewStyle(This->memlv, LVS_EX_FULLROWSELECT);

    This->PM_0 = GetSubMenu(MainForm::hm_pop, 0);
    This->OnlyCommit = FALSE;
    CheckMenuItem(This->PM_0, PM0_SHOWCOMMIT, MF_UNCHECKED);

    This->memInsertColumn();
    This->UpdateMemoryList();
  }
    return 0;
  case WM_DESTROY:
    DestroyMenu(This->PM_0);
    This->m_hWnd = NULL;
    This->memlv = NULL;
    return 0;
  case WM_SIZE:
    if (wParam != SIZE_MINIMIZED)
    {
      MoveWindow(This->memlv, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
    }
    break;
  case WM_COMMAND:
  {
    switch (LOWORD(wParam))
    {
    case PM0_REFRESH:
      This->UpdateMemoryList();
      break;
    case PM0_SHOWCOMMIT:
      if (This->OnlyCommit)
      {
        This->OnlyCommit = FALSE;
        CheckMenuItem(This->PM_0, PM0_SHOWCOMMIT, MF_UNCHECKED);
      }
      else
      {
        This->OnlyCommit = TRUE;
        CheckMenuItem(This->PM_0, PM0_SHOWCOMMIT, MF_CHECKED);
      }
      This->UpdateMemoryList();
      break;
    /*case PM0_DISASM:
    {
      int isi;
      LVITEM lvi;
      PMEMORY_LV_TYPE pmlt;

      isi = ListView_GetNextItem(This->memlv, -1, LVNI_FOCUSED | LVNI_SELECTED);
      if (isi == -1) break;
      lvi.mask = LVIF_PARAM;
      SendMessage(This->memlv, LVM_GETITEM, isi, (LPARAM)&lvi);
      if (!lvi.lParam) break;
      pmlt = (PMEMORY_LV_TYPE)lvi.lParam;
      mf->mc_disasm->GotoAddress((PBYTE)pmlt->BlockAddress,0);
    }
      break;*/
    }
  }
    break;
  case WM_NOTIFY:
  {
    if (lParam == 0) break;

    LPNMHDR pNm = (LPNMHDR)lParam;

    switch (pNm->code)
    {
    case NM_RCLICK:
    {
      if (pNm->hwndFrom == This->memlv)
      {
        This->iSelect = ListView_GetNextItem(This->memlv, -1, LVNI_FOCUSED | LVNI_SELECTED);

        if (This->iSelect != -1)
        {
          POINT point;
          GetCursorPos(&point);
          TrackPopupMenu(This->PM_0, TPM_LEFTALIGN, point.x, point.y, 0, phWnd, NULL);
        }
      }
    }
      break;
    case NM_CUSTOMDRAW:
    {
      LPNMLVCUSTOMDRAW lpnmcd = (LPNMLVCUSTOMDRAW)lParam;

      switch (lpnmcd->nmcd.dwDrawStage)
      {
      case CDDS_PREPAINT:
        if (pNm->hwndFrom == This->memlv)
          return CDRF_NOTIFYITEMDRAW;
        break;
      case CDDS_ITEMPREPAINT:
      {
        if (pNm->hwndFrom == This->memlv && lpnmcd->nmcd.lItemlParam != 0)
        {
          PLV_TYPE plv = (PLV_TYPE)lpnmcd->nmcd.lItemlParam;
          lpnmcd->clrText = plv->ForeColor;
          lpnmcd->clrTextBk = plv->BackColor;
        }
      }
        break;
      }
    }
      return CDRF_DODEFAULT;
    case LVN_DELETEALLITEMS:
      return FALSE;
    case LVN_DELETEITEM:
    {
      LPNMLISTVIEW pnl = (LPNMLISTVIEW)lParam;
      delete (PMEMORY_LV_TYPE)pnl->lParam;
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

      if (Width < 1000) lprc->right = lprc->left + 1000;
      if (Height < 200) lprc->bottom = lprc->top + 200;
    }
  }
    return 1;
  }

  return DefMDIChildProc(phWnd, Msg, wParam, lParam);
}

void MC_Memory::memInsertColumn()
{
  LV_COLUMN lvc;
  ZeroMemory(&lvc, sizeof(LV_COLUMN));
  lvc.mask = LVCF_TEXT | LVCF_WIDTH;

  lvc.pszText = L"起始地址";
  lvc.cx = DefWidth;
  ListView_InsertColumn(memlv, 0, &lvc);

  lvc.pszText = L"大小";
  lvc.cx = DefWidth;
  ListView_InsertColumn(memlv, 1, &lvc);

  lvc.pszText = L"状态";
  lvc.cx = 50;
  ListView_InsertColumn(memlv, 2, &lvc);

  lvc.pszText = L"初始保护";
  lvc.cx = DefWidth;
  ListView_InsertColumn(memlv, 3, &lvc);

  lvc.pszText = L"当前保护";
  lvc.cx = DefWidth;
  ListView_InsertColumn(memlv, 4, &lvc);

  lvc.pszText = L"属主";
  lvc.cx = 120;
  ListView_InsertColumn(memlv, 5, &lvc);

  lvc.pszText = L"区段";
  lvc.cx = 70;
  ListView_InsertColumn(memlv, 6, &lvc);

  lvc.pszText = L"类型";
  lvc.cx = 48;
  ListView_InsertColumn(memlv, 7, &lvc);

  lvc.pszText = L"保护属性摘要";
  lvc.cx = 90;
  ListView_InsertColumn(memlv, 8, &lvc);

  lvc.pszText = L"文件映射";
  lvc.cx = 250;
  ListView_InsertColumn(memlv, 9, &lvc);
}

void MC_Memory::UpdateMemoryList()
{
  PVOID bAddress = NULL;
  LV_ITEM lvi;
  MEMORY_BASIC_INFORMATION MBI;
  DWORD iItem = 0;
  PMEMORY_LV_TYPE pmlv;
  DWORD ret = VirtualQuery(bAddress, &MBI, sizeof(MEMORY_BASIC_INFORMATION));
  WCHAR stmp[MAX_PATH];

  //ZeroMemory(&lvi, sizeof(LV_ITEM));
  ListView_DeleteAllItems(memlv);

  //lvi.cchTextMax = MAX_PATH;
  lvi.pszText = stmp;

  while (ret == sizeof(MEMORY_BASIC_INFORMATION))
  {
    if (MBI.State != MEM_FREE)
    {
      if (this->OnlyCommit)
        if (MBI.State == MEM_RESERVE)
          goto NextBlock;

      pmlv = new MEMORY_LV_TYPE;
      lvi.mask = LVIF_TEXT | LVIF_PARAM;
      lvi.iItem = iItem;
      lvi.lParam = (LPARAM)pmlv;
      pmlv->LV.Item = iItem;
      pmlv->LV.ForeColor = RGB(0, 0, 0);

      lvi.iSubItem = 0;
      pmlv->BlockAddress = MBI.BaseAddress;
      swprintf_s(stmp, MAX_PATH, HEXFORMAT, MBI.BaseAddress);
      ListView_InsertItem(memlv, &lvi);

      lvi.mask = LVIF_TEXT;
      lvi.iSubItem = 1;
      swprintf_s(stmp, MAX_PATH, HEXFORMAT, MBI.RegionSize);
      ListView_SetItem(memlv, &lvi);

      lvi.iSubItem = 2;
      switch (MBI.State)
      {
      case MEM_COMMIT:
        wcscpy_s(stmp, MAX_PATH, T_COMMIT);
        break;
      case MEM_RESERVE:
        wcscpy_s(stmp, MAX_PATH, T_RESERVE);
        pmlv->LV.ForeColor = RGB(160, 85, 20);
        break;
      default:
        wcscpy_s(stmp, MAX_PATH, T_UNKNOWN);
      }
      ListView_SetItem(memlv, &lvi);

      lvi.iSubItem = 3;
      swprintf_s(stmp, MAX_PATH, HEXFORMAT, MBI.AllocationProtect);
      ListView_SetItem(memlv, &lvi);

      lvi.iSubItem = 4;
      swprintf_s(stmp, MAX_PATH, HEXFORMAT, MBI.Protect);
      ListView_SetItem(memlv, &lvi);

      lvi.iSubItem = 5;
      wcscpy_s(stmp, MAX_PATH, T_NULL);
      if (MBI.Type == MEM_IMAGE)
      {
        if (GetModuleFileName((HMODULE)MBI.AllocationBase, stmp, MAX_PATH))
        {
          lvi.iSubItem = 9;
          ListView_SetItem(memlv, &lvi);
          lvi.iSubItem = 5;
          GetFileName(stmp);
        }
        ListView_SetItem(memlv, &lvi);

        lvi.iSubItem = 6;
        if (MBI.AllocationBase == MBI.BaseAddress)
        {
          wcscpy_s(stmp, MAX_PATH, T_FILEHEADER);
          ListView_SetItem(memlv, &lvi);
        }
        else
        {
          wcscpy_s(stmp, MAX_PATH, T_NULL);
          GetSectionNameA((HMODULE)MBI.AllocationBase, MBI.BaseAddress, (PCHAR)stmp, MAX_PATH);
          ListView_SetItemA(memlv, &lvi);
        }
      }
      else
      {
        ListView_SetItem(memlv, &lvi);
        lvi.iSubItem = 6;
        ListView_SetItem(memlv, &lvi);
      }

      lvi.iSubItem = 7;
      switch (MBI.Type)
      {
      case MEM_PRIVATE:
        wcscpy_s(stmp, MAX_PATH, T_M_PRIVATE);
        pmlv->LV.BackColor = RGB(190, 255, 190);
        break;
      case MEM_IMAGE:
        wcscpy_s(stmp, MAX_PATH, T_M_IMAGE);
        pmlv->LV.BackColor = RGB(190, 190, 255);
        break;
      case MEM_MAPPED:
        wcscpy_s(stmp, MAX_PATH, T_M_MAPPED);
        pmlv->LV.BackColor = RGB(255, 190, 190);
        break;
      default:
        wcscpy_s(stmp, MAX_PATH, T_UNKNOWN);
        pmlv->LV.BackColor = RGB(255, 255, 255);
      }
      ListView_SetItem(memlv, &lvi);

      lvi.iSubItem = 8;
      GetPageProtectString(MBI.Protect, stmp, MAX_PATH);
      ListView_SetItem(memlv, &lvi);

      if (MBI.Type == MEM_MAPPED)
      {
        lvi.iSubItem = 9;
        if (GetMappedFileName(INVALID_HANDLE_VALUE, MBI.AllocationBase, stmp, MAX_PATH))
          ListView_SetItem(memlv, &lvi);
      }

      iItem++;
    }

  NextBlock:
    bAddress = (PVOID)((DWORD)bAddress + MBI.RegionSize);
    ret = VirtualQuery(bAddress, &MBI, sizeof(MEMORY_BASIC_INFORMATION));
  }
}