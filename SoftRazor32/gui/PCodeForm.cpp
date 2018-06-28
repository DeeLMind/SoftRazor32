#include "../sr_pch.h"

MC_PCodeProc * MC_PCodeProc::This = NULL;

MC_PCodeProc::~MC_PCodeProc()
{
  if (this->m_hWnd)
  {
    DestroyWindow(this->m_hWnd);
    this->m_hWnd = NULL;
  }
  This = NULL;
}

HWND MC_PCodeProc::CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style)
{
  if (this->m_hWnd)
  {
    this->SetTop();
    return this->m_hWnd;
  }

  MDICREATESTRUCT MCS;

  MCS.szClass = (PWCHAR)mf->rcls_mc_pcodeproc;
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

void MC_PCodeProc::AddProc(PWCHAR pProcName, PVOID pAddr, int cb_idx)
{
  if (!pProcName || !pAddr) return;
  PPPLST ptmp = (PPPLST)malloc(sizeof(PPLST));
  if (!ptmp) return;

  ptmp->pAddr = pAddr;
  ptmp->cbidx = cb_idx;
  ptmp->Next = NULL;
  wcscpy_s(ptmp->ProcName, 256, pProcName);

  if (ProcListLast)
  {
    ProcListLast->Next = ptmp;
    ProcListLast = ptmp;
  }
  else
  {
    ProcListHead = ptmp;
    ProcListLast = ptmp;
  }
}

PVOID MC_PCodeProc::GetProcAddrByName(PWCHAR pProcName)
{
  PPPLST pl = ProcListHead;

  while (pl)
  {
    if (_tcscmp(pProcName, pl->ProcName) == 0)
      return pl->pAddr;
    pl = pl->Next;
  }
  return 0;
}

PVOID MC_PCodeProc::GetProcAddrByIndex(int cbIdx)
{
  PPPLST pl = ProcListHead;

  if (cbIdx < 0) return 0;
  while (pl)
  {
    if (cbIdx == pl->cbidx)
      return pl->pAddr;
    pl = pl->Next;
  }
  return 0;
}

void MC_PCodeProc::DelAllProc()
{
  PPPLST pl = ProcListHead;
  PPPLST pd;

  while (pl)
  {
    pd = pl;
    pl = pl->Next;
    free(pd);
  }

  ProcListHead = NULL;
  ProcListLast = NULL;
}

void MC_PCodeProc::UpdateProc(PWCHAR pObjName)
{
  PVBOBJLST pobjlst = mf->GetObjectListByName(pObjName);    //控件名取指针
  PVBOOI poptinfo;    //VB对象可选信息头
  PVBCTRL pctl;      //VB控件
  PPVBEP ppep;    //事件指针
  DWORD j;
  PDWORD pdw = NULL; //检测重复
  DWORD dcount = 0;
  int cbidx;
  UINT mbase;
  UINT msize;
  WORD i;
  WCHAR ptmp0[64];
  WCHAR ptmp1[64];
  WCHAR ptmp2[64];

  ComboBox_DeleteAll(hEvtCbx);
  DelAllProc();

  if (!pobjlst) return;
  if (!HitModule((PVOID)(pobjlst->VbHead), &mbase, &msize)) return;

  //被点击的对象
  poptinfo = (PVBOOI)((DWORD)pobjlst->VbObj->lpObjectInfo + sizeof(VBOI));
  pctl = poptinfo->lpControls;

  if (pctl)
  {
    //循环控件事件
    for (i = 0; i < poptinfo->wControlCount; i++)
    {
      ppep = (PPVBEP)((DWORD)pctl[i].lpEventHandlerTable + sizeof(VB_EVENT_TABLE));
      mf->SetDbgState(DS_Busy);

      for (j = 0; j < pctl[i].wEventHandlerCount; j++)
      {
        //如果该事件地址和描述地址不为NULL
        if (ppep[j] == NULL || ppep[j]->lpDescInfo == NULL) continue;

        PVBPL vbprclst;
        size_t nocc;

        pdw = (PDWORD)_recalloc(pdw, ++dcount, sizeof(DWORD));
        if (!pdw) return;

        if (vbprclst = mf->GetProcListByAddress(pobjlst->ObjName, ppep[j]->lpDescInfo))
          wcscpy_s(ptmp0, 64, vbprclst->EvtName);
        else
        {
          if (!GetMemberNameByIndex(*pctl[i].lpGuid, (DWORD)j, ptmp1, 64, 0))
            swprintf_s(ptmp0, 64, L"Event_%08X", ppep[j]->lpDescInfo);
          else
          {
            mbstowcs_s(&nocc, ptmp2, (size_t)64, pctl[i].lpszName, _TRUNCATE);
            swprintf_s(ptmp0, 64, L"%s_%s", ptmp2, ptmp1);
          }
        }
        pdw[dcount - 1] = (DWORD)ppep[j]->lpDescInfo;
        cbidx = SendMessage(hEvtCbx, CB_ADDSTRING, 0, (LPARAM)ptmp0);
        if (cbidx >= 0)
          AddProc(ptmp0, (PBYTE)ppep[j]->lpDescInfo, cbidx);
      }

      mf->SetDbgState(DS_Idle);
    }
  }

  mf->SetDbgState(DS_Busy);

  /* 如果存在模块过程 */
  if (pobjlst->VbObj->lpObjectInfo->wMethodCount || pobjlst->VbObj->lpObjectInfo->lpMethods)
  {
    /* 循环模块过程 */
    for (j = 0; j < pobjlst->VbObj->lpObjectInfo->wMethodCount; j++)
    {
      DWORD pAddr = pobjlst->VbObj->lpObjectInfo->lpMethods[j];

      if (pAddr == 0 || pAddr == (DWORD)-1 || pAddr < mbase || (pAddr >= mbase + msize)) continue;

      if (pdw && dcount)
      {
        DWORD di;
        BOOL ble = FALSE;

        for (di = 0; di < dcount; di++)
        {
          if (pAddr == pdw[di])
          {
            ble = TRUE;
            break;
          }
        }

        if (ble) break;
      }

      swprintf_s(ptmp0, 64, L"Proc_%08X", pAddr);
      cbidx = SendMessage(hEvtCbx, CB_ADDSTRING, 0, (LPARAM)ptmp0);
      if (cbidx >= 0) AddProc(ptmp0, (PVOID)pAddr, cbidx);
    }
  }
  mf->SetDbgState(DS_Idle);
  if (pdw) free(pdw);
}

BOOL MC_PCodeProc::ParseHeader(PVBHDR pVbHeader)
{
  if (!pVbHeader)
  {
    mf->SetStateText(L"无效的VB头部指针!", RGB(255, 0, 0));
    return FALSE;
  }

  if (pVbHeader->Sign.dwSign != VB_MAGIC)
  {
    mf->SetStateText(L"VB结构标志不正确!", RGB(255, 0, 0));
    return FALSE;
  }

  if (m_hWnd) SetTop();
  else CreateForm(CW_USEDEFAULT, CW_USEDEFAULT, 820, 420, WN_PCPROC, MDI_STYLE);

  PVBOBJLST pVbObjL;
  PVBPUBOBJ pVbObject;
  SIZE_T BVal;
  WORD ObjCount;
  WORD wi;

  mf->SetDbgState(DS_Busy);
  pVbObject = pVbHeader->lpProjInfo->lpObjectTable->lpObjectArray;
  ObjCount = pVbHeader->lpProjInfo->lpObjectTable->dwTotalObjects;

  for (wi = 0; wi < ObjCount; wi++)
  {
    PVBOBJLST pVbObjL;

    if (!(pVbObjL = mf->GetObjectListByObject(&pVbObject[wi])))
    {
      VBOBJLST VbObjL;
      PWCHAR pstmp;
      size_t slen = strlen(pVbObject[wi].lpSZObjectName);

      memset(&VbObjL, 0, sizeof(VbObjL));

      if (!slen)
      {
        pstmp = (PWCHAR)mf->m_hmm->hcalloc(16, sizeof(WCHAR));
        swprintf_s(pstmp, 16, L"_obj_%hu", pVbObject[wi].lpObjectInfo->wObjectIndex);
      }
      else
      {
        pstmp = (PWCHAR)mf->m_hmm->hcalloc(slen + 1);
        mbstowcs_s((size_t *)&BVal, pstmp, slen + 1, pVbObject[wi].lpSZObjectName, _TRUNCATE);
      }

      VbObjL.VbHead = pVbHeader;
      VbObjL.VbObj = &pVbObject[wi];
      VbObjL.iIndex = pVbObject[wi].lpObjectInfo->wObjectIndex;
      VbObjL.ObjName = pstmp;

      pVbObjL = mf->AddObjectList(&VbObjL);
    }

    SendMessage(hObjCbx, CB_ADDSTRING, 0, (LPARAM)pVbObjL->ObjName);
  }

  mf->SetDbgState(DS_Idle);
  return TRUE;
}

/*
void aaa()
{
PVBOOI poptinfo;    //VB对象可选信息头
PVBC pctl;
PPVBEP ppep;
WORD wj;
WORD wk;
poptinfo = (PVBOOI)((DWORD)pVbObject[wi].lpObjectInfo + sizeof(VBOI));
pctl = poptinfo->pControlArray;

for (wj = 0; wj < poptinfo->ControlCount; wj++)
{
ppep = (PPVBEP)((DWORD)pctl[wj].pEventTable + sizeof(VB_EVENT_TABLE));
for (wk = 0; wk < pctl[wj].EventCount; wk++)
{
if (ppep[wk] == NULL || ppep[wk]->lpDescInfo == NULL) continue;
PVBPL pVbProcL;

if (!(pVbProcL = mf->GetProcListByAddress(pVbObjL->ObjName, ppep[wk]->lpDescInfo)))
{
PWCHAR pstmp;
PVOID TT;
DWORD LE;
VBPL VbProcL;
WCHAR entmp[64];

memset(&VbProcL, 0, sizeof(VBPL));
pstmp = (PWCHAR)mf->mhmm->hcalloc(64, sizeof(WCHAR));

if (!GetMemberNameByIndex(*pctl[wj].pGUID, (DWORD)wk, entmp, 64, 0))
swprintf_s(entmp, 64, L"Event_%hu", wk);
swprintf_s(pstmp, 64, L"%s_%s", pVbObjL->ObjName, entmp);
VbProcL.pVbCtl = &pctl[wj];
VbProcL.iIndex = wk;
VbProcL.ProcDesc = ppep[wk]->lpDescInfo;
VbProcL.EvtName = pstmp;
pVbProcL = mf->AddProcListByObject(pVbObjL, &VbProcL);
}
SendMessage(hEvtCbx, CB_ADDSTRING, 0, (LPARAM)pVbProcL->EvtName);
}
}
}
*/

void MC_PCodeProc::InsertColumn()
{
  LV_COLUMN lvc;
  ZeroMemory(&lvc, sizeof(LV_COLUMN));
  lvc.mask = LVCF_TEXT | LVCF_WIDTH;

  lvc.pszText = L"标志";
  lvc.cx = 48;
  ListView_InsertColumn(hList, 0, &lvc);

  lvc.pszText = L"地址";
  lvc.cx = DefWidth;
  ListView_InsertColumn(hList, 1, &lvc);

  lvc.pszText = L"助记符";
  lvc.cx = 256;
  ListView_InsertColumn(hList, 2, &lvc);

  lvc.pszText = L"相关引用";
  lvc.cx = 256;
  ListView_InsertColumn(hList, 3, &lvc);
}

void MC_PCodeProc::ClearList()
{
  ListView_DeleteAllItems(hList);
}

void MC_PCodeProc::UpdateList(PVBPDI ppdi /* 过程描述 */)
{
#define PRIV_BUFLEN       256
  UINT mb;
  UINT ms;
  BOOL blInit = TRUE;
  int bval;
  UINT lIdx = 0;
  PDO pcdeobj;
  LV_ITEM lvi;
  WCHAR stmp[PRIV_BUFLEN];
  WCHAR snote[PRIV_BUFLEN];
  WCHAR haddr[10];

  if (!HitModule(ppdi, &mb, &ms)) return;
  //清除全部列表项
  ClearList();

  pcf_init_decode_object_basic(&pcdeobj, ppdi, stmp, PRIV_BUFLEN, mb, ms);
  pcdeobj.sz_note = snote;
  pcdeobj.sl_note = PRIV_BUFLEN;

  if (!pcf_alloc_mapping(&pcdeobj))
  {
    MessageBeep(MB_ICONHAND);
    mf->SetStateText(L"申请字节码映射失败!", RGB(255, 0, 0));
    return;
  }

  while ((bval = pcf_disasm_proc(&pcdeobj)) == 0)
  {
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.iItem = lIdx;

    lvi.iSubItem = 0;
    lvi.pszText = T_NULL;
    lvi.lParam = (LPARAM)((UINT)pcdeobj.inp_opbuf + pcdeobj.bk_idx);
    ListView_InsertItem(hList, &lvi);

    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT;
    lvi.pszText = haddr;
    swprintf_s(haddr, 10, HEXFORMAT, lvi.lParam);
    ListView_SetItem(hList, &lvi);

    lvi.iSubItem = 2;
    lvi.pszText = pcdeobj.sz_mnem;
    ListView_SetItem(hList, &lvi);

    lvi.iSubItem = 3;
    lvi.pszText = pcdeobj.sz_note;
    ListView_SetItem(hList, &lvi);

    lIdx++;
  }

  pcf_free_mapping(&pcdeobj);

  if (bval < 1)
  {
    swprintf_s(stmp, PRIV_BUFLEN, L"解析P-Code错误! ErrorCode:[%d]", bval);
    MessageBeep(MB_ICONHAND);
    mf->SetStateText(stmp, RGB(255, 0, 0));
  }
  else
  {
    lvi.mask = LVIF_TEXT | LVIF_PARAM;
    lvi.iItem = lIdx;

    lvi.iSubItem = 0;
    lvi.pszText = T_NULL;
    lvi.lParam = (LPARAM)((UINT)pcdeobj.inp_opbuf + pcdeobj.bk_idx);
    ListView_InsertItem(hList, &lvi);

    lvi.iSubItem = 1;
    lvi.mask = LVIF_TEXT;
    lvi.pszText = haddr;
    swprintf_s(haddr, 10, HEXFORMAT, lvi.lParam);
    ListView_SetItem(hList, &lvi);

    lvi.iSubItem = 2;
    lvi.pszText = pcdeobj.sz_mnem;
    ListView_SetItem(hList, &lvi);

    lvi.iSubItem = 3;
    lvi.pszText = pcdeobj.sz_note;
    ListView_SetItem(hList, &lvi);
  }

  return;
#undef PRIV_BUFLEN
}

LRESULT CALLBACK MC_PCodeProc::pcpWndProc(HWND phWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
  case WM_CREATE:
  {
    LONG ExtStyle = GetWindowLong(phWnd, GWL_EXSTYLE);

    ExtStyle |= WS_EX_CLIENTEDGE;
    SetWindowLong(phWnd, GWL_EXSTYLE, ExtStyle);

    This->hObjCbx = CreateWindow(WC_COMBOBOX, NULL, CBS_DROPDOWNLIST | WS_VSCROLL | DEF_VCHILD,
      54, 3, 160, 20, phWnd, 0, DllBaseAddr, 0);
    This->hEvtCbx = CreateWindow(WC_COMBOBOX, NULL, CBS_DROPDOWNLIST | WS_VSCROLL | DEF_VCHILD,
      274, 3, 320, 20, phWnd, 0, DllBaseAddr, 0);

    This->hList = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, NULL, LV_STYLE, 0, 0, 0, 0, phWnd, NULL, DllBaseAddr, 0);

    ListView_SetExtendedListViewStyle(This->hList, LVS_EX_FULLROWSELECT);

    //SendMessage(phWnd, WM_SETFONT, (WPARAM)mf->hf_fix, 1);

    if (This->hObjCbx)
    {
      SendMessage(This->hObjCbx, WM_SETFONT, (WPARAM)mf->hf_fix, 1);
      SendMessage(This->hObjCbx, CB_SETITEMHEIGHT, -1, 20);
    }

    if (This->hEvtCbx)
    {
      SendMessage(This->hEvtCbx, WM_SETFONT, (WPARAM)mf->hf_fix, 1);
      SendMessage(This->hEvtCbx, CB_SETITEMHEIGHT, -1, 20);
    }

    if (This->hList)
      SendMessage(This->hList, WM_SETFONT, (WPARAM)mf->hf_tal, 1);

    This->InsertColumn();
  }
    return 0;
  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(phWnd, &ps);

    if (!hDC) break;
    SelectObject(hDC, MainForm::hf_fix);
    SetTextColor(hDC, RGB(0, 0, 63));
    TextOut(hDC, 4, 8, L"对象:", 3);
    TextOut(hDC, 226, 8, L"过程:", 3);
    EndPaint(phWnd, &ps);
  }
    return 0;
  case WM_COMMAND:
  {
    WORD wid = LOWORD(wParam);
    WORD wnc = HIWORD(wParam);

    switch (wnc)
    {
    case CBN_SELCHANGE:
    {
      if (This->hObjCbx == (HWND)lParam)
      {
        This->ClearList();

        int sLen = SendMessage(This->hObjCbx, WM_GETTEXTLENGTH, 0, 0);
        PWCHAR pstmp;

        if (sLen <= 0) return 0;
        pstmp = (PWCHAR)calloc(sLen + 2, sizeof(WCHAR));
        if (!pstmp) return 0;

        GetWindowText(This->hObjCbx, pstmp, sLen + 2);
        This->UpdateProc(pstmp);

        free(pstmp);
        return 0;
      }
      else if (This->hEvtCbx == (HWND)lParam)
      {
        int cIdx = SendMessage(This->hEvtCbx, CB_GETCURSEL, 0, 0);
        if (cIdx < 0) return 0;
        PVOID pAddr = This->GetProcAddrByIndex(cIdx);
        if (pAddr) This->UpdateList((PVBPDI)pAddr);
        return 0;
      }
    }
      break;
    }
  }
    break;
  case WM_SIZE:
    if (wParam != SIZE_MINIMIZED)
    {
      WORD Width = LOWORD(lParam);
      WORD Height = HIWORD(lParam);
      MoveWindow(This->hList, 1, 36, LOWORD(lParam) - 2, HIWORD(lParam) - 36, TRUE);
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
  case WM_DESTROY:
    This->DelAllProc();
    This->m_hWnd = NULL;
    return 0;
  }

  return DefMDIChildProc(phWnd, Msg, wParam, lParam);
}