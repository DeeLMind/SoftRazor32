#include "../sr_pch.h"

MC_Heap * MC_Heap::This = NULL;

MC_Heap::~MC_Heap()
{
  if (this->m_hWnd)
  {
    DestroyWindow(this->m_hWnd);
    this->m_hWnd = NULL;
  }
  This = NULL;
}

HWND MC_Heap::CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style)
{
  if (this->m_hWnd)
  {
    this->SetTop();
    return this->m_hWnd;
  }

  MDICREATESTRUCT MCS;

  MCS.szClass = (PWCHAR)mf->rcls_mc_heap;
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

LRESULT CALLBACK MC_Heap::heapWndProc(HWND phWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
  case WM_CREATE:
  {
    This->hw_heaplv = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, LN_HEAP, LV_STYLE, 0, 0, 0, 0, phWnd, NULL, DllBaseAddr, 0);
    ListView_SetExtendedListViewStyle(This->hw_heaplv, LVS_EX_FULLROWSELECT);

    This->heapInsertColumn();
    This->UpdateHeapList();
  }
    return 0;
  case WM_DESTROY:
    This->m_hWnd = NULL;
    This->hw_heaplv = NULL;
    return 0;
  case WM_SIZE:
    if (wParam != SIZE_MINIMIZED)
    {
      MoveWindow(This->hw_heaplv, 0, 0, LOWORD(lParam), HIWORD(lParam), TRUE);
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
      if (Height < 400) lprc->bottom = lprc->top + 400;
    }
  }
    return 1;
  }

  return DefMDIChildProc(phWnd, Msg, wParam, lParam);
}

void MC_Heap::heapInsertColumn()
{
  LV_COLUMN lvc;
  ZeroMemory(&lvc, sizeof(LV_COLUMN));
  lvc.mask = LVCF_TEXT | LVCF_WIDTH;

  lvc.pszText = L"堆块基址";
  lvc.cx = DefWidth;
  ListView_InsertColumn(hw_heaplv, 0, &lvc);

  lvc.pszText = L"堆块句柄";
  lvc.cx = DefWidth;
  ListView_InsertColumn(hw_heaplv, 1, &lvc);

  lvc.pszText = L"已用大小";
  lvc.cx = DefWidth;
  ListView_InsertColumn(hw_heaplv, 2, &lvc);

  lvc.pszText = L"空闲大小";
  lvc.cx = DefWidth;
  ListView_InsertColumn(hw_heaplv, 3, &lvc);

  lvc.pszText = L"堆块类别";
  lvc.cx = DefWidth;
  ListView_InsertColumn(hw_heaplv, 4, &lvc);
}

void MC_Heap::UpdateHeapList()
{
  LV_ITEM lvi;
  DWORD i = 0;
  HANDLE hSnapshot = CreateToolhelp32Snapshot(TH32CS_SNAPHEAPLIST, GetCurrentProcessId());
  HEAPLIST32 hl;
  WCHAR stmp[20];

  ListView_DeleteAllItems(hw_heaplv);
  if (hSnapshot == INVALID_HANDLE_VALUE) return;

  hl.dwSize = sizeof(HEAPLIST32);
  lvi.mask = LVIF_TEXT;
  lvi.cchTextMax = 20;
  lvi.pszText = stmp;

  mf->SetDbgState(DS_Busy);

  if (Heap32ListFirst(hSnapshot, &hl))
  {
    HEAPENTRY32 he;

    ZeroMemory(&he, sizeof(HEAPENTRY32));
    he.dwSize = sizeof(HEAPENTRY32);

    do
    {
      ULONG_PTR HeapAddr;
      SIZE_T UsedSize;
      SIZE_T FreeSize;
      HANDLE hHeap;

      UsedSize = 0;
      FreeSize = 0;

      if (Heap32First(&he, GetCurrentProcessId(), hl.th32HeapID))
      {
        HeapAddr = he.dwAddress;
        hHeap = he.hHandle;

        do
        {
          if (GetTickCount() % 100 <= 9)
            CheckMessageQueue();
          if (he.dwFlags == LF32_FREE)
            FreeSize += he.dwBlockSize;
          else
            UsedSize += he.dwBlockSize;
        } while (Heap32Next(&he));

        lvi.iItem = i;
        lvi.iSubItem = 0;
        swprintf_s(stmp, 10, HEXFORMAT, HeapAddr);
        ListView_InsertItem(hw_heaplv, &lvi);

        lvi.iSubItem = 1;
        swprintf_s(stmp, 10, HEXFORMAT, hHeap);
        ListView_SetItem(hw_heaplv, &lvi);

        lvi.iSubItem = 2;
        swprintf_s(stmp, 10, HEXFORMAT, UsedSize);
        ListView_SetItem(hw_heaplv, &lvi);

        lvi.iSubItem = 3;
        swprintf_s(stmp, 10, HEXFORMAT, FreeSize);
        ListView_SetItem(hw_heaplv, &lvi);

        lvi.iSubItem = 4;
        if (hl.dwFlags == HF32_DEFAULT)
          wcscpy_s(stmp, 20, T_H_DEFAULT);
        else if (hl.dwFlags == HF32_SHARED)
          wcscpy_s(stmp, 20, T_H_SHARED);
        else
          wcscpy_s(stmp, 20, T_OTHER);
        ListView_SetItem(hw_heaplv, &lvi);

        i++;
      }
      hl.dwSize = sizeof(HEAPLIST32);
    } while (Heap32ListNext(hSnapshot, &hl));
  }

  CloseHandle(hSnapshot);
  mf->SetDbgState(DS_Idle);
}