#include "../sr_pch.h"

#define RDD_DISASM            0x00000001U
#define RDD_DATADUMP          0x00000002U
#define RDD_REGISTERS         0x00000004U
#define RDD_STACK             0x00000008U

/* DisasmDrawSubRect */
#define DDSR_CIRCUIT          0x00000001U
#define DDSR_ADDRESS          0x00000002U
#define DDSR_BYTES            0x00000004U
#define DDSR_MNEMONIC         0x00000008U
#define DDSR_INFO             0x00000010U
#define DDSR_NOTE             0x00000020U

#define DASM_SPACE            4


/* Data Dump Style */
#define DDS_ADDRESS           0
#define DDS_BYTE              1
#define DDS_WORD              2
#define DDS_DWORD             3
#define DDS_QWORD             4
#define DDS_SINGLE            5
#define DDS_DOUBLE            6
#define DDS_ASCII             7
#define DDS_UTF8              8
#define DDS_UTF16             9


MC_Disasm * MC_Disasm::This = NULL;

MC_Disasm::~MC_Disasm()
{
  if (this->m_hWnd)
  {
    DestroyWindow(this->m_hWnd);
    this->m_hWnd = NULL;
  }

  delete hmm;
  delete smm;
  This = NULL;
}

PDAI MC_Disasm::GetItemByIndex(UINT Idx)
{
  if (Idx >= diCount) return NULL;
  return ItemArray[Idx];
}

UINT MC_Disasm::GetItemIndexByAddress(PVOID Address)
{
  UINT i;

  for (i = 0; i < diCount; i++)
  {
    if ((UINT)Address >= (UINT)ItemArray[i]->lpVirtAddr &&
      (UINT)Address < (UINT)ItemArray[i]->lpVirtAddr + (UINT)ItemArray[i]->uiInstSize)
      return i;
  }
  return (UINT)-1;
}

PDAI MC_Disasm::GetItemByAddress(PVOID Address)
{
  UINT i;
  UINT itemCount = diCount;
  if (!ItemArray) return NULL;
  if ((UINT)Address < (UINT)aBaseAddr || (UINT)Address >= (UINT)aBaseAddr + aBlockSize) return NULL;

  for (i = 0; i < itemCount; i++)
  {
    if (ItemArray[i]->lpVirtAddr == Address)
      return ItemArray[i];
    else if (ItemArray[i]->lpVirtAddr == 0)
      break;
  }
  return NULL;
}

void MC_Disasm::DelItemArray()
{
  hmm->hreset(1, 524288);
  smm->hreset(1, 524288);
}


void MC_Disasm::SetScrollBar(UINT CurrPos, UINT MaxPos)
{
  if (MaxPos)
    SendMessage(hw_vsb, SBM_SETRANGE, 0, MaxPos - 1);
  SendMessage(hw_vsb, SBM_SETPOS, CurrPos, TRUE);
}

HWND MC_Disasm::CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style)
{
  if (this->m_hWnd)
  {
    this->SetTop();
    return this->m_hWnd;
  }

  MDICREATESTRUCT MCS;

  MCS.szClass = (PWCHAR)mf->rcls_mc_disasm;
  MCS.szTitle = wName;
  MCS.hOwner = mf->MdlBase;
  MCS.style = Style;
  MCS.cx = cx;
  MCS.cy = cy;
  MCS.x = x;
  MCS.y = y;
  MCS.lParam = (LPARAM)this;

  this->m_hWnd = (HWND)::SendMessage(hw_MDI, WM_MDICREATE, 0, (LPARAM)&MCS);
  if (m_hWnd && !fw_reg) fw_reg = new FW_RegWnd(m_hWnd);
  return this->m_hWnd;
}

int MC_Disasm::AssembleOfAddress(PVOID Addr, XEDPARSE * XEDP)
{
  if (!Addr || !XEDP || !XEDP->dest_size || XEDP->dest_size > 16) return -1;
  UINT oItem = GetSelLineIndex();
  if (oItem == (UINT)-1) return -2;

  PPDAI pItem = ItemArray;
  PDAI tmpItem;
  UINT ocl = 0;   //Original Code Length
  UINT oin = 1;   //Original Item Number
  UINT nin;
  UINT i;
  UINT j;

  PBYTE nCode;

  while (true)                        //
  {
    if ((oItem + oin - 1) >= diCount)
      return -3;

    ocl += pItem[oItem + oin - 1]->uiInstSize;
    if (ocl >= XEDP->dest_size) break;
    oin++;
  }

  nCode = (PBYTE)malloc(ocl);
  if (!nCode) return -4;
  memcpy_s(nCode, ocl, XEDP->dest, XEDP->dest_size);
  i = ocl - XEDP->dest_size;

  if (i)    //如果有多余长度
    memset(&nCode[XEDP->dest_size], 0x90, i);    //填充为nop

  mf->m_mer->TryModifyMemory((PBYTE)Addr, nCode, ocl);

  ud_t dt;
  /*初始化ud*/
  ud_init(&dt);
  ud_set_mode(&dt, 32);
  ud_set_syntax(&dt, UD_SYN_INTEL);
  ud_set_input_buffer(&dt, nCode, ocl);
  ud_set_pc(&dt, (uint64_t)Addr);

  mf->SetDbgState(DS_Busy);
  nin = ud_count(&dt);

  if (!nin)
  {
    free(nCode);
    mf->SetDbgState(DS_Idle);
    return -5;
  }

  ud_set_input_buffer(&dt, nCode, ocl);
  ud_set_pc(&dt, (uint64_t)Addr);

  blDraw = FALSE;

  for (i = 0; i < oin; i++)
  {
    if (pItem[oItem + i]->lpszInstText) smm->hfree(pItem[oItem + i]->lpszInstText);
    if (pItem[oItem + i]->lpNote) smm->hfree(pItem[oItem + i]->lpNote);
    if (pItem[oItem + i]) hmm->hfree(pItem[oItem + i]);
  }

  if (nin != oin)   //如果新数量不等于原条目数量 (重分配数组)
  {
    if (nin > oin)    //新大于原
    {
      //先重新分配数组,再移动指针
      ItemArray = (PPDAI)hmm->hrecalloc(ItemArray, diCount + (nin - oin), sizeof(PDAI));
      pItem = ItemArray;
      memmove(&pItem[oItem + nin], &pItem[oItem + oin], (diCount - (oItem + oin)) * sizeof(PDAI));
      diCount += nin - oin;
    }
    else              //原大于新
    {
      //先移动指针,再重新分配数组
      memmove(&pItem[oItem + nin], &pItem[oItem + oin], (diCount - (oItem + oin)) * sizeof(PDAI));
      ItemArray = (PPDAI)hmm->hrecalloc(ItemArray, diCount - (oin - nin), sizeof(PDAI));
      pItem = ItemArray;
      diCount -= oin - nin;
    }
  }

  for (i = 0; i < nin; i++)
  {
    WCHAR stmp[10];

    /*处理消息*/
    CheckMessageQueue();

    while (ud_disassemble(&dt) == 0)
    {
      if (dt.inp_end) break;
      dt.inp_buf_index++;
    }

    if (dt.inp_end) break;

    tmpItem = (PDAI)hmm->hcalloc(sizeof(DISASM_ITEM));
    if (!tmpItem)
    {
      free(nCode);
      return -6;
    }

    tmpItem->lpVirtAddr = (PVOID)dt.insn_offset;
    tmpItem->uiInstSize = dt.inp_ctr;

    swprintf_s(stmp, 10, HEXFORMAT, tmpItem->lpVirtAddr);
    memcpy(tmpItem->szAddrText, stmp, 8 * sizeof(WCHAR));
    j = casmlen(dt.casm_buf, 128);
    tmpItem->lpszInstText = (PCHAR)smm->hmalloc((j + 1) * sizeof(CHAR));
    if (!tmpItem->lpszInstText)
    {
      free(nCode);
      return -7;
    }
    tmpItem->uiCount = j + 1;
    casmcpy(tmpItem->lpszInstText, dt.casm_buf, j + 1);
    pItem[oItem + i] = tmpItem;
  }

  aSelAddrA = Addr;
  aSelAddrB = Addr;
  mf->SetDbgState(DS_Idle);
  blDraw = TRUE;
  OuterRedrawDisasmRect();
  return 0;
}

BOOL MC_Disasm::DisasmOfAddress(LPCVOID dsAddress, DWORD Flag)
{
  PBYTE cAddr;      //反汇编地址所在内存块基址
  PBYTE pData;      //原内存数据
  PWCHAR pText;
  PDAI pItem;
  UINT i;
  UINT ui0;
  UINT ui1;
  ud_t dt;
  MEMORY_BASIC_INFORMATION MBI;
  WCHAR stmp[10];
  WCHAR stmp1[64];

  /*验证地址的有效性*/
  if (!VirtualQuery(dsAddress, &MBI, sizeof(MEMORY_BASIC_INFORMATION))) return FALSE;
  if (MBI.State != MEM_COMMIT || MBI.RegionSize == 0) return FALSE;
  if (!GetSectionBaseAddr((DWORD)dsAddress, (PDWORD)&cAddr, (PDWORD)&ui1)) return FALSE;

  /*设置为忙碌状态*/
  mf->SetDbgState(DS_Busy);

  /*初始化数据*/
  aBaseAddr = cAddr;
  aBlockSize = ui1;
  aDrawIndex = (UINT)-1;
  aDrawAddr = NULL;
  aSelAddrA = NULL;
  aSelAddrB = NULL;

  if (!GetSectionNameW((HMODULE)MBI.AllocationBase, MBI.BaseAddress, stmp, 10))
  {
    if (MBI.Type == MEM_MAPPED)
      wcscpy_s(aOwnerName, 64, L"映射");
    else
      wcscpy_s(aOwnerName, 64, L"其它");
  }
  else
  {
    if (!GetModuleName((PVOID)dsAddress, stmp1, 64))
      wcscpy_s(stmp1, 64, L"未知模块");
    swprintf_s(aOwnerName, 64, L"%s=>\"%s\"", stmp1, stmp);
  }

  if (!GetDataSection((HMODULE)MBI.AllocationBase, (PDWORD)&ui0, (PDWORD)&dBlockSize))
  {
    dBaseAddr = aBaseAddr;
    dBlockSize = dBlockSize;
  }
  else
  {
    dBaseAddr = (PVOID)ui0;
  }

  dDrawAddr = dBaseAddr;

  if (!GetSectionNameW((HMODULE)MBI.AllocationBase, dBaseAddr, stmp, 10))
  {
    if (MBI.Type == MEM_MAPPED)
      wcscpy_s(dOwnerName, 64, L"映射");
    else
      wcscpy_s(dOwnerName, 64, L"其它");
  }
  else
  {
    if (!GetModuleName((PVOID)dsAddress, stmp1, 64))
      wcscpy_s(stmp1, 64, L"未知模块");
    swprintf_s(dOwnerName, 64, L"%s=>\"%s\"", stmp1, stmp);
  }

  /*申请内存,以存放原始内存块数据*/
  pData = (PBYTE)malloc(ui1);
  if (!pData)
  {
    mf->SetDbgState(DS_Idle);
    return FALSE;
  }

  if (!mf->m_mer->GetOriginalMemory(cAddr, pData, ui1))
  {
    free(pData);
    mf->SetDbgState(DS_Idle);
    return FALSE;
  }

  /*初始化ud*/
  ud_init(&dt);
  ud_set_mode(&dt, 32);
  ud_set_syntax(&dt, UD_SYN_INTEL);
  ud_set_input_buffer(&dt, pData, ui1);
  ud_set_pc(&dt, (uint64_t)cAddr);

  /*清空之前的反汇编条目*/
  hmm->hreset(1, 524288);
  smm->hreset(1, 524288);
  diCount = ud_count(&dt);          //取条目数量
  ItemArray = (PPDAI)hmm->hcalloc(diCount, sizeof(PDAI));

  ud_set_input_buffer(&dt, pData, ui1);
  ud_set_pc(&dt, (uint64_t)cAddr);      //设置内存块的基地址 (EIP)

  for (i = 0; i < diCount; i++)
  {
    /*处理消息*/
    CheckMessageQueue();

    while (ud_disassemble(&dt) == 0)
    {
      if (dt.inp_end) break;
      dt.inp_buf_index++;
    }

    if (dt.inp_end) break;

    pItem = (PDAI)hmm->hcalloc(sizeof(DISASM_ITEM));
    if (!pItem)
    {
      free(pData);
      mf->SetDbgState(DS_Idle);
      return FALSE;
    }

    pItem->lpVirtAddr = (PVOID)dt.insn_offset;
    pItem->uiInstSize = dt.inp_ctr;

    swprintf_s(stmp, 10, HEXFORMAT, pItem->lpVirtAddr);
    memcpy(pItem->szAddrText, stmp, 8 * sizeof(WCHAR));
    ui0 = casmlen(dt.casm_buf, 128);
    pItem->lpszInstText = (PCHAR)smm->hmalloc((ui0 + 1) * sizeof(CHAR));
    if (!pItem->lpszInstText)
    {
      free(pData);
      mf->SetDbgState(DS_Idle);
      return FALSE;
    }
    pItem->uiCount = ui0 + 1;
    casmcpy(pItem->lpszInstText, dt.casm_buf, ui0 + 1);
    ItemArray[i] = pItem;
  }

  free(pData);
  aDrawIndex = GetItemIndexByAddress((PVOID)dsAddress);
  aDrawAddr = (PVOID)dsAddress;
  aSelAddrA = (PVOID)dsAddress;
  aSelAddrB = (PVOID)dsAddress;

  if (this->m_hWnd)
    this->SetTop();
  else
    this->CreateForm(CW_USEDEFAULT, CW_USEDEFAULT, 800, 600, WN_DISASM, MDI_STYLE);

  OuterRedrawDisasmRect();
  OuterRedrawDataRect();
  mf->SetDbgState(DS_Idle);
  return TRUE;
}

BOOL MC_Disasm::GotoAddress(PBYTE dsAddress, DWORD Flag)
{
  if (((UINT)dsAddress >= (UINT)aBaseAddr) && ((UINT)dsAddress < (UINT)aBaseAddr + aBlockSize))
  {
    SetTop();
    aDrawIndex = GetItemIndexByAddress(dsAddress);
    aDrawAddr = dsAddress;
    OuterRedrawDisasmRect();
    return TRUE;
  }
  return DisasmOfAddress(dsAddress, Flag);
}

BOOL MC_Disasm::DumpOfAddress(PBYTE ddAddress, DWORD Flag)
{
  MEMORY_BASIC_INFORMATION MBI;
  HMODULE hMod;
  WCHAR stmp[10];
  WCHAR stmp1[64];

  if (!VirtualQuery(ddAddress, &MBI, sizeof(MEMORY_BASIC_INFORMATION))) return FALSE;

  dBaseAddr = MBI.BaseAddress;
  dBlockSize = MBI.RegionSize;
  dDrawAddr = dBaseAddr;
  dSelAddrA = NULL;
  dSelAddrB = NULL;

  if (!GetSectionNameW((HMODULE)MBI.AllocationBase, MBI.BaseAddress, stmp, 10))
  {
    if (MBI.Type == MEM_MAPPED)
      wcscpy_s(dOwnerName, 64, L"映射");
    else
      wcscpy_s(dOwnerName, 64, L"其它");
    OuterRedrawDataRect();
    return TRUE;
  }

  if (!GetModuleName(ddAddress, stmp1, 64))
    wcscpy_s(stmp1,64,L"未知模块");
  swprintf_s(dOwnerName, 64, L"%s=>[%s]", stmp1,stmp);
  return TRUE;
}

PVOID MC_Disasm::GetSelLineAddress()
{
  return aSelAddrA;
}

UINT MC_Disasm::GetSelLineIndex()
{
  UINT i;
  UINT BaseIdx = aDrawIndex;

  if (aSelAddrA && (aSelAddrA == aSelAddrB))
  {
    if (BaseIdx == UINT(-1))
      return GetItemIndexByAddress(aSelAddrA);

    for (i = BaseIdx; i < (BaseIdx + aMaxItem); i++)
    {
      if (ItemArray[i]->lpVirtAddr == aSelAddrA)
        return i;
    }
  }
  return (UINT)-1;
}

UINT MC_Disasm::GetAsmTextByAddress(PVOID lpAddr, PWCHAR lpszAsmText, UINT max)
{
  if (lpAddr == NULL || lpszAsmText == NULL || max == 0) return 0;
  lpszAsmText[0] = 0;
  PDAI pItem = GetItemByAddress(lpAddr);
  if (pItem == NULL) return 0;

  size_t i, len, csize, count = 0;
  WCHAR stmp[MAX_PATH];

  for (i = 0; (i < pItem->uiCount) && (count < max); i++)
  {
    if (pItem->lpszInstText[i] == CD_NULL) break;

    i++;
    len = strnlen(&pItem->lpszInstText[i], pItem->uiCount - i);

    if (count + len >= max) break;

    mbstowcs_s(&csize, stmp, MAX_PATH, &pItem->lpszInstText[i], len);
    wcscat_s(lpszAsmText, max, stmp);
    i += len;
    count += csize;
  }
  return count;
}

UINT MC_Disasm::GetOpLenByAddress(PVOID Addr)
{
  if (!Addr) return 0;
  PDAI pItem = GetItemByAddress(Addr);
  if (!pItem) return 0;
  return pItem->uiInstSize;
}

void MC_Disasm::InvalidateDisasmRect()
{
  RECT rect;

  rect.left = asmLeft;
  rect.top = asmregTop;
  rect.right = asmLeft + asmWidth;
  rect.bottom = asmregTop + asmregHeight;

  InvalidateRect(this->m_hWnd, &rect, TRUE);
  UpdateWindow(this->m_hWnd);
}

void MC_Disasm::InvalidateDataRect()
{
  RECT rect;

  rect.left = datLeft;
  rect.top = datstkTop;
  rect.right = datLeft + datWidth;
  rect.bottom = datstkTop + datstkHeight;

  InvalidateRect(this->m_hWnd, &rect, TRUE);
  UpdateWindow(this->m_hWnd);
}

void MC_Disasm::InvalidateRegistersRect()
{
  RECT rect;

  rect.left = regLeft;
  rect.top = asmregTop;
  rect.right = regLeft + regWidth;
  rect.bottom = asmregTop + asmregHeight;

  InvalidateRect(this->m_hWnd, &rect, TRUE);
  UpdateWindow(this->m_hWnd);
}

void MC_Disasm::InvalidateStackRect()
{
  RECT rect;

  rect.left = stkLeft;
  rect.top = datstkTop;
  rect.right = stkLeft + stkWidth;
  rect.bottom = datstkTop + datstkHeight;

  InvalidateRect(this->m_hWnd, &rect, TRUE);
  UpdateWindow(this->m_hWnd);
}

void MC_Disasm::InvalidateDisasmAndRegistersRect()
{
  RECT rect;

  rect.left = asmLeft;
  rect.top = asmregTop;
  rect.right = regLeft + regWidth;
  rect.bottom = asmregTop + asmregHeight;

  InvalidateRect(this->m_hWnd, &rect, TRUE);
  UpdateWindow(this->m_hWnd);
}

void MC_Disasm::InvalidateDataAndStackRect()
{
  RECT rect;

  rect.left = datLeft;
  rect.top = datstkTop;
  rect.right = stkLeft + stkWidth;
  rect.bottom = datstkTop + datstkHeight;

  InvalidateRect(this->m_hWnd, &rect, TRUE);
  UpdateWindow(this->m_hWnd);
}

void MC_Disasm::InvalidateAllRect()
{
  RECT rect;

  rect.left = drLeft;
  rect.top = drTop;
  rect.right = drLeft + drWidth;
  rect.bottom = drTop + drHeight;

  InvalidateRect(this->m_hWnd, &rect, TRUE);
  UpdateWindow(this->m_hWnd);
}

void MC_Disasm::InvalidateHeightRect(WORD Top, WORD Bottom)
{
  RECT rect;

  if (Top > 12) Top -= 12;
  if (Bottom < (drTop + drHeight) - 12) Bottom += 12;

  rect.left = drLeft;
  rect.top = Top;
  rect.right = drLeft + drWidth;
  rect.bottom = Bottom;

  InvalidateRect(this->m_hWnd, &rect, TRUE);
  UpdateWindow(this->m_hWnd);
}

void MC_Disasm::InvalidateRectByRect(WORD Left, WORD Top, WORD Right, WORD Bottom)
{
  RECT rect;

  rect.left = Left;
  rect.top = Top;
  rect.right = Right;
  rect.bottom = Bottom;

  InvalidateRect(this->m_hWnd, &rect, TRUE);
  UpdateWindow(this->m_hWnd);
}

void MC_Disasm::OuterRedrawDisasmRect()
{
  if (!m_hWnd) return;

  HDC hDC = GetDC(m_hWnd);
  if (!hDC) return;
  HDC hCDC = CreateCompatibleDC(hDC);
  HBITMAP hBM = CreateCompatibleBitmap(hDC, int(cWidth), int(cHeight));

  if (hCDC)
  {
    if (hBM)
    {
      SelectObject(hCDC, hBM);
      DeleteObject(hBM);

      SelectObject(hCDC, MainForm::hf_fix);

      SelectObject(hCDC, GetStockObject(WHITE_PEN));

      SelectObject(hCDC, MainForm::hb_white);

      DrawDisasmRect(hCDC);
      BitBlt(hDC, asmLeft, asmregTop, asmWidth, asmregHeight, hCDC, asmLeft, asmregTop, SRCCOPY);

      DeleteDC(hCDC);
      ReleaseDC(m_hWnd, hDC);

      return;
    }
    DeleteDC(hCDC);
  }

  /* 设置字体 */
  SelectObject(hDC, MainForm::hf_fix);
  /* 设置白色画笔 */
  SelectObject(hDC, GetStockObject(WHITE_PEN));
  /* 设置白色画刷 */
  SelectObject(hDC, MainForm::hb_white);

  DrawDisasmRect(hDC);

  ReleaseDC(m_hWnd, hDC);
}

void MC_Disasm::OuterRedrawDataRect()
{
  if (!m_hWnd) return;
  HDC hDC = GetDC(m_hWnd);

  if (!hDC) return;
  HDC hCDC = CreateCompatibleDC(hDC);
  HBITMAP hBM = CreateCompatibleBitmap(hDC, int(cWidth), int(cHeight));

  if (hCDC)
  {
    if (hBM)
    {
      SelectObject(hCDC, hBM);
      DeleteObject(hBM);

      SelectObject(hCDC, MainForm::hf_fix);

      SelectObject(hCDC, GetStockObject(WHITE_PEN));

      SelectObject(hCDC, MainForm::hb_white);

      DrawDataRect(hCDC);
      BitBlt(hDC, datLeft, datstkTop, datWidth, datstkHeight, hCDC, datLeft, datstkTop, SRCCOPY);

      DeleteDC(hCDC);
      ReleaseDC(m_hWnd, hDC);

      return;
    }
    DeleteDC(hCDC);
  }

  /* 设置字体 */
  SelectObject(hDC, MainForm::hf_fix);
  /* 设置白色画笔 */
  SelectObject(hDC, GetStockObject(WHITE_PEN));
  /* 设置白色画刷 */
  SelectObject(hDC, MainForm::hb_white);

  DrawDataRect(hDC);
  ReleaseDC(m_hWnd, hDC);
}

void MC_Disasm::OuterRedrawRegistersRect()
{
  if (!m_hWnd) return;
  HDC hDC = GetDC(m_hWnd);

  if (!hDC) return;
  HDC hCDC = CreateCompatibleDC(hDC);
  HBITMAP hBM = CreateCompatibleBitmap(hDC, int(cWidth), int(cHeight));

  if (hCDC)
  {
    if (hBM)
    {
      SelectObject(hCDC, hBM);
      DeleteObject(hBM);

      SelectObject(hCDC, MainForm::hf_fix);

      SelectObject(hCDC, GetStockObject(WHITE_PEN));

      SelectObject(hCDC, MainForm::hb_white);

      DrawRegistersRect(hCDC);
      BitBlt(hDC, regLeft, asmregTop, regWidth, asmregHeight, hCDC, regLeft, asmregTop, SRCCOPY);

      DeleteDC(hCDC);
      ReleaseDC(m_hWnd, hDC);

      return;
    }
    DeleteDC(hCDC);
  }

  /* 设置字体 */
  SelectObject(hDC, MainForm::hf_fix);
  /* 设置白色画笔 */
  SelectObject(hDC, GetStockObject(WHITE_PEN));
  /* 设置白色画刷 */
  SelectObject(hDC, MainForm::hb_white);

  DrawRegistersRect(hDC);
  ReleaseDC(m_hWnd, hDC);
}

void MC_Disasm::OuterRedrawStackRect()
{
  if (!m_hWnd) return;
  HDC hDC = GetDC(m_hWnd);

  if (!hDC) return;
  HDC hCDC = CreateCompatibleDC(hDC);
  HBITMAP hBM = CreateCompatibleBitmap(hDC, int(cWidth), int(cHeight));

  if (hCDC)
  {
    if (hBM)
    {
      SelectObject(hCDC, hBM);
      DeleteObject(hBM);

      SelectObject(hCDC, MainForm::hf_fix);

      SelectObject(hCDC, GetStockObject(WHITE_PEN));

      SelectObject(hCDC, MainForm::hb_white);

      DrawStackRect(hCDC);
      BitBlt(hDC, stkLeft, datstkTop, stkWidth, datstkHeight, hCDC, stkLeft, datstkTop, SRCCOPY);

      DeleteDC(hCDC);
      ReleaseDC(m_hWnd, hDC);

      return;
    }
    DeleteDC(hCDC);
  }

  /* 设置字体 */
  SelectObject(hDC, MainForm::hf_fix);
  /* 设置白色画笔 */
  SelectObject(hDC, GetStockObject(WHITE_PEN));
  /* 设置白色画刷 */
  SelectObject(hDC, MainForm::hb_white);

  DrawStackRect(hDC);
  ReleaseDC(m_hWnd, hDC);
}

void MC_Disasm::OuterRedrawDisasmAndRegistersRect()
{
  if (!m_hWnd) return;
  HDC hDC = GetDC(m_hWnd);

  if (!hDC) return;
  HDC hCDC = CreateCompatibleDC(hDC);
  HBITMAP hBM = CreateCompatibleBitmap(hDC, int(cWidth), int(cHeight));

  if (hCDC)
  {
    if (hBM)
    {
      SelectObject(hCDC, hBM);
      DeleteObject(hBM);

      SelectObject(hCDC, MainForm::hf_fix);
      SelectObject(hCDC, GetStockObject(WHITE_PEN));
      SelectObject(hCDC, MainForm::hb_white);

      DrawDisasmRect(hCDC);
      DrawRegistersRect(hCDC);

      BitBlt(hDC, asmLeft, asmregTop, drWidth, asmregHeight, hCDC, asmLeft, asmregTop, SRCCOPY);

      DeleteDC(hCDC);
      ReleaseDC(m_hWnd, hDC);

      return;
    }
    DeleteDC(hCDC);
  }

  /* 设置字体 */
  SelectObject(hDC, MainForm::hf_fix);
  /* 设置白色画笔 */
  SelectObject(hDC, GetStockObject(WHITE_PEN));
  /* 设置白色画刷 */
  SelectObject(hDC, MainForm::hb_white);

  DrawDisasmRect(hDC);
  DrawRegistersRect(hDC);
  ReleaseDC(m_hWnd, hDC);
}

void MC_Disasm::OuterRedrawDataAndStackRect()
{
  if (!m_hWnd) return;
  HDC hDC = GetDC(m_hWnd);

  if (!hDC) return;
  HDC hCDC = CreateCompatibleDC(hDC);
  HBITMAP hBM = CreateCompatibleBitmap(hDC, int(cWidth), int(cHeight));

  if (hCDC)
  {
    if (hBM)
    {
      SelectObject(hCDC, hBM);
      DeleteObject(hBM);

      SelectObject(hCDC, MainForm::hf_fix);
      SelectObject(hCDC, GetStockObject(WHITE_PEN));
      SelectObject(hCDC, MainForm::hb_white);

      DrawDataRect(hCDC);
      DrawStackRect(hCDC);

      BitBlt(hDC, datLeft, datstkTop, drWidth, datstkHeight, hCDC, datLeft, datstkTop, SRCCOPY);

      DeleteDC(hCDC);
      ReleaseDC(m_hWnd, hDC);

      return;
    }
    DeleteDC(hCDC);
  }

  /* 设置字体 */
  SelectObject(hDC, MainForm::hf_fix);
  /* 设置白色画笔 */
  SelectObject(hDC, GetStockObject(WHITE_PEN));
  /* 设置白色画刷 */
  SelectObject(hDC, MainForm::hb_white);

  DrawDataRect(hDC);
  DrawStackRect(hDC);
  ReleaseDC(m_hWnd, hDC);
}

void MC_Disasm::OuterRedrawAllRect()
{
  if (!m_hWnd) return;
  HDC hDC = GetDC(m_hWnd);

  if (!hDC) return;

  if (!DoubleBufferDraw(hDC, NULL))
    SingleBufferDraw(hDC, NULL);
}

void MC_Disasm::OuterRedrawDisasmSubRect(UINT dFlag)
{
  if (!m_hWnd) return;
  HDC hDC = GetDC(m_hWnd);

  if (!hDC) return;
  /* 设置字体 */
  SelectObject(hDC, MainForm::hf_fix);
  /* 设置白色画笔 */
  SelectObject(hDC, GetStockObject(WHITE_PEN));
  /* 设置白色画刷 */
  SelectObject(hDC, MainForm::hb_white);

  DrawDisasmSubRect(hDC, dFlag);
  ReleaseDC(m_hWnd, hDC);
}

PDAI MC_Disasm::HitTest(WORD px /* Left */, WORD py /* Top */)
{
  if (!asmWidth || !asmregHeight || aDrawIndex == UINT(-1) || !aDrawAddr) return NULL;
  if (px < asmLeft + 2 || px > asmLeft + asmWidth - 2 || py < asmregTop + 2 || py > asmregTop + asmregHeight - 2) return NULL;

  UINT BaseIdx = aDrawIndex;
  UINT Idx;
  
  Idx = (py - asmregTop - 2) / 20;

  if (BaseIdx >= diCount)
    BaseIdx = GetItemIndexByAddress(aDrawAddr);

  return GetItemByIndex(BaseIdx + Idx);
}

BOOL MC_Disasm::HitRect(LPCRECT rc0, LPCRECT rc1)
{
  if ((rc1->left >= rc0->left && rc1->left <= rc0->right && rc1->top >= rc0->top && rc1->top <= rc0->bottom) ||
    (rc1->right >= rc0->left && rc1->right <= rc0->right && rc1->top >= rc0->top && rc1->top <= rc0->bottom) ||
    (rc1->left >= rc0->left && rc1->left <= rc0->right && rc1->bottom >= rc0->top && rc1->bottom <= rc0->bottom) ||
    (rc1->right >= rc0->left && rc1->right <= rc0->right && rc1->bottom >= rc0->top && rc1->bottom <= rc0->bottom) ||
    (rc0->left >= rc1->left && rc0->left <= rc1->right && rc0->top >= rc1->top && rc0->top <= rc1->bottom) ||
    (rc0->right >= rc1->left && rc0->right <= rc1->right && rc0->top >= rc1->top && rc0->top <= rc1->bottom) ||
    (rc0->left >= rc1->left && rc0->left <= rc1->right && rc0->bottom >= rc1->top && rc0->bottom <= rc1->bottom) ||
    (rc0->right >= rc1->left && rc0->right <= rc1->right && rc0->bottom >= rc1->top && rc0->bottom <= rc1->bottom))
    return TRUE;
  return FALSE;
}

void MC_Disasm::DrawDisasmRect(HDC hDC)
{
  UINT BaseIdx = aDrawIndex;
  PPDAI ppItem = ItemArray;
  int itmp;
  WORD i;           //索引
  WORD OverSize;    //反汇编区域剩余大小
  RECT rect;

  rect.left = asmLeft;
  rect.top = asmregTop;
  rect.right = rect.left + asmWidth;
  rect.bottom = rect.top + asmregHeight;

  if (BaseIdx >= diCount)
    BaseIdx = GetItemIndexByAddress(aDrawAddr);

  /* 设置黑色画笔 */
  SelectObject(hDC, GetStockObject(BLACK_PEN));
  /* 设置黑色画刷 */
  SelectObject(hDC, MainForm::hb_gray);

  if ((cdFlag0 & 3) == 0)
  {
    FrameRect(hDC, &rect, MainForm::hb_black);
    rect.left++;
    rect.top++;
    rect.right--;
    rect.bottom--;
    FrameRect(hDC, &rect, MainForm::hb_dkgray);
  }
  else
  {
    FrameRect(hDC, &rect, MainForm::hb_gray);
    rect.left++;
    rect.top++;
    rect.right--;
    rect.bottom--;
    FrameRect(hDC, &rect, MainForm::hb_gray);
  }

  rect.left++;
  rect.top++;
  rect.right--;
  rect.bottom--;

  FillRect(hDC, &rect, MainForm::hb_white);

  if (BaseIdx == (UINT)-1 || !ItemArray) return;

  for (i = 0; i < aMaxItem; i++)    //绘制每个条目
  {
    if (BaseIdx + i >= diCount) break;
    OverSize = (asmregHeight - DASM_SPACE) - i * 20;

    /* 开始绘制条目背景 */
    rect.left = asmLeft + 2;
    rect.top = (asmregTop + 2) + i * 20;
    rect.right = asmRight;
    rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20));

    /* 判断当前是否shift所选择的条目,并进行绘制条目背景 */
    if (aSelAddrA && aSelAddrB)
    {
      /* 存在被选中的条目 */
      if ((UINT)ppItem[BaseIdx + i]->lpVirtAddr >= (UINT)aSelAddrA &&
        (UINT)ppItem[BaseIdx + i]->lpVirtAddr <= (UINT)aSelAddrB)
        FillRect(hDC, &rect, MainForm::hb_ltgray);
      else
        FillRect(hDC, &rect, MainForm::hb_white);
    }
    else
      FillRect(hDC, &rect, MainForm::hb_white);

    rect.top += 2;
    rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20 - 2));

    /*开始绘制地址文本*/
    SetTextColor(hDC, RGB(0, 0, 0));
    SetBkMode(hDC, TRANSPARENT);//设置背景样式
    rect.left = dlAddr + 2;
    rect.right = dlByte - 1;
    DrawText(hDC, ppItem[BaseIdx + i]->szAddrText, 8, &rect, DT_LEFT | DT_SINGLELINE);

    /*开始绘制汇编指令*/
    rect.left = dlMnem + 2;
    rect.right = dlInfo - 1;

    DrawCasmA(hDC, ppItem[BaseIdx + i]->lpszInstText,
      ppItem[BaseIdx + i]->uiCount, 20, &rect, DT_LEFT | DT_SINGLELINE);
  }

  /*
  if ( ((asmregTop + 2) + i * 20) < (asmregTop + asmregHeight - 4) )
  {
    OverSize = (asmregHeight - DASM_SPACE) - i * 20;

    rect.left = asmLeft + 2;
    rect.top = (asmregTop + 2) + i * 20;
    rect.right = asmRight;
    rect.bottom = asmregTop + OverSize;

    FillRect(hDC, &rect, MainForm::hb_white);
  }*/

  itmp = asmregTop + asmregHeight - 2;

  MoveToEx(hDC, dlAddr, asmregTop + 2, NULL);
  LineTo(hDC, dlAddr, itmp);

  MoveToEx(hDC, dlByte, asmregTop + 2, NULL);
  LineTo(hDC, dlByte, itmp);

  MoveToEx(hDC, dlMnem, asmregTop + 2, NULL);
  LineTo(hDC, dlMnem, itmp);

  MoveToEx(hDC, dlInfo, asmregTop + 2, NULL);
  LineTo(hDC, dlInfo, itmp);

  MoveToEx(hDC, dlNote, asmregTop + 2, NULL);
  LineTo(hDC, dlNote, itmp);
}

void MC_Disasm::DrawDataRect(HDC hDC)
{
  RECT rect;

  rect.left = datLeft;
  rect.top = datstkTop;
  rect.right = datLeft + datWidth;
  rect.bottom = datstkTop + datstkHeight;

  if ((cdFlag0 & 3) == 1)
  {
    FrameRect(hDC, &rect, MainForm::hb_black);
    rect.left++;
    rect.top++;
    rect.right--;
    rect.bottom--;
    FrameRect(hDC, &rect, MainForm::hb_dkgray);
  }
  else
  {
    FrameRect(hDC, &rect, MainForm::hb_gray);
    rect.left++;
    rect.top++;
    rect.right--;
    rect.bottom--;
    FrameRect(hDC, &rect, MainForm::hb_gray);
  }

  rect.left++;
  rect.top++;
  rect.right--;
  rect.bottom--;
  FillRect(hDC, &rect, MainForm::hb_white);
}

void MC_Disasm::DrawRegistersRect(HDC hDC)
{
  RECT rect;

  rect.left = regLeft;
  rect.top = asmregTop;
  rect.right = regLeft + regWidth;
  rect.bottom = asmregTop + asmregHeight;

  if ((cdFlag0 & 3) == 2)
  {
    FrameRect(hDC, &rect, MainForm::hb_black);
    rect.left++;
    rect.top++;
    rect.right--;
    rect.bottom--;
    FrameRect(hDC, &rect, MainForm::hb_dkgray);
  }
  else
  {
    FrameRect(hDC, &rect, MainForm::hb_gray);
    rect.left++;
    rect.top++;
    rect.right--;
    rect.bottom--;
    FrameRect(hDC, &rect, MainForm::hb_gray);
  }

  rect.left++;
  rect.top++;
  rect.right--;
  rect.bottom--;
  FillRect(hDC, &rect, MainForm::hb_white);

}

void MC_Disasm::DrawStackRect(HDC hDC)
{
  RECT rect;

  rect.left = stkLeft;
  rect.top = datstkTop;
  rect.right = stkLeft + stkWidth;
  rect.bottom = datstkTop + datstkHeight;

  if ((cdFlag0 & 3) == 3)
  {
    FrameRect(hDC, &rect, MainForm::hb_black);
    rect.left++;
    rect.top++;
    rect.right--;
    rect.bottom--;
    FrameRect(hDC, &rect, MainForm::hb_dkgray);
  }
  else
  {
    FrameRect(hDC, &rect, MainForm::hb_gray);
    rect.left++;
    rect.top++;
    rect.right--;
    rect.bottom--;
    FrameRect(hDC, &rect, MainForm::hb_gray);
  }

  rect.left++;
  rect.top++;
  rect.right--;
  rect.bottom--;
  FillRect(hDC, &rect, MainForm::hb_white);
}

void MC_Disasm::DrawTextRect(HDC hDC)
{
  RECT rect;
  WCHAR DrawBuffer[256];

  SetTextColor(hDC, RGB(32, 32, 255));
  SetBkMode(hDC, TRANSPARENT);
  SelectObject(hDC, MainForm::hf_tam);

  /* 绘制状态栏边框 */
  rect.left = drLeft;
  rect.top = dtText;
  rect.right = drLeft + dwText;
  rect.bottom = dtText + dhText;
  FrameRect(hDC, &rect, MainForm::hb_dkgray);

  rect.left++;
  rect.top++;
  rect.right--;
  rect.bottom--;
  FillRect(hDC, &rect, MainForm::hb_white);

  rect.left++;
  rect.top++;
  rect.right--;
  rect.bottom = rect.top + 19;

  swprintf_s(DrawBuffer, 256, L"反汇编段基址:%08X 大小:%08X 属主:%s    数据转储基址:%08X 大小:%08X 属主:%s",
    aBaseAddr, aBlockSize, aOwnerName, dBaseAddr, dBlockSize, dOwnerName);
  DrawText(hDC, DrawBuffer, -1, &rect, DT_LEFT | DT_SINGLELINE);
}

void MC_Disasm::DrawDisasmSubRect(HDC hDC, UINT dFlag)
{
  UINT BaseIdx = GetItemIndexByAddress(aDrawAddr);
  PPDAI ppItem = ItemArray;
  RECT rect;
  WORD i;           //索引
  WORD OverSize;    //反汇编区域剩余大小

  /* 设置黑色画笔 */
  SelectObject(hDC, GetStockObject(BLACK_PEN));
  /* 设置黑色画刷 */
  SelectObject(hDC, MainForm::hb_gray);

  if (BaseIdx == (UINT)-1 || !ItemArray) return;

  for (i = 0; i < aMaxItem; i++)    //绘制每个条目
  {
    if (BaseIdx + i >= diCount) break;
    OverSize = (asmregHeight - DASM_SPACE) - i * 20;

    if (dFlag & DDSR_CIRCUIT)     //流程线路
    {
      /* 开始绘制条目背景 */
      rect.left = asmLeft + 2;
      rect.top = (asmregTop + 2) + i * 20;
      rect.right = dlAddr - 1;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20));

      /* 判断当前是否shift所选择的条目,并进行绘制条目背景 */
      if (aSelAddrA && aSelAddrB)
      {
        /* 存在被选中的条目 */
        if ((UINT)ppItem[BaseIdx + i]->lpVirtAddr >= (UINT)aSelAddrA &&
          (UINT)ppItem[BaseIdx + i]->lpVirtAddr <= (UINT)aSelAddrB)
          FillRect(hDC, &rect, MainForm::hb_ltgray);
        else
          FillRect(hDC, &rect, MainForm::hb_white);
      }
      else
        FillRect(hDC, &rect, MainForm::hb_white);

      rect.top += 2;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20 - 2));
    }


    if (dFlag & DDSR_ADDRESS)     //地址
    {
      /* 开始绘制条目背景 */
      rect.left = dlAddr + 1;
      rect.top = (asmregTop + 2) + i * 20;
      rect.right = dlByte - 1;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20));

      /* 判断当前是否shift所选择的条目,并进行绘制条目背景 */
      if (aSelAddrA && aSelAddrB)
      {
        /* 存在被选中的条目 */
        if ((UINT)ppItem[BaseIdx + i]->lpVirtAddr >= (UINT)aSelAddrA &&
          (UINT)ppItem[BaseIdx + i]->lpVirtAddr <= (UINT)aSelAddrB)
          FillRect(hDC, &rect, MainForm::hb_ltgray);
        else
          FillRect(hDC, &rect, MainForm::hb_white);
      }
      else
        FillRect(hDC, &rect, MainForm::hb_white);

      rect.left++;
      rect.top += 2;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20 - 2));

      /*开始绘制地址文本*/
      SetTextColor(hDC, RGB(0, 0, 0));
      SetBkMode(hDC, TRANSPARENT);//设置背景样式
      rect.left = dlAddr + 2;
      rect.right = dlByte - 1;
      DrawText(hDC, ppItem[BaseIdx + i]->szAddrText, 8, &rect, DT_LEFT | DT_SINGLELINE);
    }

    if (dFlag & DDSR_BYTES)     //字节码
    {
      /* 开始绘制条目背景 */
      rect.left = dlByte + 1;
      rect.top = (asmregTop + 2) + i * 20;
      rect.right = dlMnem - 1;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20));

      /* 判断当前是否shift所选择的条目,并进行绘制条目背景 */
      if (aSelAddrA && aSelAddrB)
      {
        /* 存在被选中的条目 */
        if ((UINT)ppItem[BaseIdx + i]->lpVirtAddr >= (UINT)aSelAddrA &&
          (UINT)ppItem[BaseIdx + i]->lpVirtAddr <= (UINT)aSelAddrB)
          FillRect(hDC, &rect, MainForm::hb_ltgray);
        else
          FillRect(hDC, &rect, MainForm::hb_white);
      }
      else
        FillRect(hDC, &rect, MainForm::hb_white);

      rect.left++;
      rect.top += 2;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20 - 2));
    }

    if (dFlag & DDSR_MNEMONIC)     //助记符
    {
      /* 开始绘制条目背景 */
      rect.left = dlMnem + 1;
      rect.top = (asmregTop + 2) + i * 20;
      rect.right = dlInfo - 1;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20));

      /* 判断当前是否shift所选择的条目,并进行绘制条目背景 */
      if (aSelAddrA && aSelAddrB)
      {
        /* 存在被选中的条目 */
        if ((UINT)ppItem[BaseIdx + i]->lpVirtAddr >= (UINT)aSelAddrA &&
          (UINT)ppItem[BaseIdx + i]->lpVirtAddr <= (UINT)aSelAddrB)
          FillRect(hDC, &rect, MainForm::hb_ltgray);
        else
          FillRect(hDC, &rect, MainForm::hb_white);
      }
      else
        FillRect(hDC, &rect, MainForm::hb_white);

      rect.left++;
      rect.top += 2;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20 - 2));

      DrawCasmA(hDC, ppItem[BaseIdx + i]->lpszInstText,
        ppItem[BaseIdx + i]->uiCount, 20, &rect, DT_LEFT | DT_SINGLELINE);
    }

    if (dFlag & DDSR_INFO)    //引用
    {
      /* 开始绘制条目背景 */
      rect.left = dlInfo + 1;
      rect.top = (asmregTop + 2) + i * 20;
      rect.right = dlNote - 1;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20));

      /* 判断当前是否shift所选择的条目,并进行绘制条目背景 */
      if (aSelAddrA && aSelAddrB)
      {
        /* 存在被选中的条目 */
        if ((UINT)ppItem[BaseIdx + i]->lpVirtAddr >= (UINT)aSelAddrA &&
          (UINT)ppItem[BaseIdx + i]->lpVirtAddr <= (UINT)aSelAddrB)
          FillRect(hDC, &rect, MainForm::hb_ltgray);
        else
          FillRect(hDC, &rect, MainForm::hb_white);
      }
      else
        FillRect(hDC, &rect, MainForm::hb_white);

      rect.left++;
      rect.top += 2;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20 - 2));
    }

    if (dFlag & DDSR_NOTE)    //引用
    {
      /* 开始绘制条目背景 */
      rect.left = dlNote + 1;
      rect.top = (asmregTop + 2) + i * 20;
      rect.right = asmRight;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20));

      /* 判断当前是否shift所选择的条目,并进行绘制条目背景 */
      if (aSelAddrA && aSelAddrB)
      {
        /* 存在被选中的条目 */
        if ((UINT)ppItem[BaseIdx + i]->lpVirtAddr >= (UINT)aSelAddrA &&
          (UINT)ppItem[BaseIdx + i]->lpVirtAddr <= (UINT)aSelAddrB)
          FillRect(hDC, &rect, MainForm::hb_ltgray);
        else
          FillRect(hDC, &rect, MainForm::hb_white);
      }
      else
        FillRect(hDC, &rect, MainForm::hb_white);

      rect.left++;
      rect.top += 2;
      rect.bottom = rect.top + ((OverSize / 20) ? 20 : (OverSize % 20 - 2));
    }
  }

  if (((asmregTop + 2) + i * 20) < (asmregTop + asmregHeight))
  {
    OverSize = (asmregHeight - DASM_SPACE) - i * 20;

    /* 开始绘制条目背景 */
    rect.left = asmLeft + 2;
    rect.top = (asmregTop + 2) + i * 20;
    rect.right = asmRight;
    rect.bottom = asmregTop + asmregHeight - 2;

    FillRect(hDC, &rect, MainForm::hb_white);
  }

  MoveToEx(hDC, dlAddr, asmregTop + 2, NULL);
  LineTo(hDC, dlAddr, asmregTop + asmregHeight - 2);

  MoveToEx(hDC, dlByte, asmregTop + 2, NULL);
  LineTo(hDC, dlByte, asmregTop + asmregHeight - 2);

  MoveToEx(hDC, dlMnem, asmregTop + 2, NULL);
  LineTo(hDC, dlMnem, asmregTop + asmregHeight - 2);

  MoveToEx(hDC, dlNote, asmregTop + 2, NULL);
  LineTo(hDC, dlNote, asmregTop + asmregHeight - 2);
}

void MC_Disasm::UpdateDisasmSubRect()
{
  asmRight = asmLeft + asmWidth - 22;

  if (dlNote + 40 > asmRight)
    dlNote = asmRight - 40;

  if (dlInfo + 40 > dlNote)
    dlInfo = dlNote - 40;

  if (dlMnem + 40 > dlInfo)
    dlMnem = dlInfo - 40;

  if (dlByte + 40 > dlMnem)
    dlByte = dlMnem - 40;

  if (dlAddr + 40 > dlByte)
    dlAddr = dlByte - 40;
}

void MC_Disasm::DeviceContextDraw(HDC hDC, LPPAINTSTRUCT lpps)
{
  RECT rect;

  /* 设置字体 */
  SelectObject(hDC, MainForm::hf_fix);
  /* 设置白色画笔 */
  SelectObject(hDC, GetStockObject(WHITE_PEN));
  /* 设置白色画刷 */
  SelectObject(hDC, MainForm::hb_white);
  /* 设置背景色 */
  SetBkColor(hDC, RGB(255, 255, 255));

  /* 绘制背景 */
  rect.left = drLeft;
  rect.top = drTop;
  rect.right = cWidth;
  rect.bottom = cHeight;
  FillRect(hDC, &rect, MainForm::hb_white);

  if (lpps)
  {
    /* 测试命中反汇编区域 */
    rect.left = asmLeft;
    rect.top = asmregTop;
    rect.right = asmLeft + asmWidth;
    rect.bottom = asmregTop + asmregHeight;

    if (HitRect(&rect, &lpps->rcPaint))
      DrawDisasmRect(hDC);

    /* 测试命中数据转储区域 */
    rect.left = datLeft;
    rect.top = datstkTop;
    rect.right = datLeft + datWidth;
    rect.bottom = datstkTop + datstkHeight;

    if (HitRect(&rect, &lpps->rcPaint))
      DrawDataRect(hDC);

    /* 测试命中寄存器区域 */
    rect.left = regLeft;
    rect.top = asmregTop;
    rect.right = regLeft + regWidth;
    rect.bottom = asmregTop + asmregHeight;

    if (HitRect(&rect, &lpps->rcPaint))
      DrawRegistersRect(hDC);

    /* 测试命中栈区域 */
    rect.left = stkLeft;
    rect.top = datstkTop;
    rect.right = stkLeft + stkWidth;
    rect.bottom = datstkTop + datstkHeight;

    if (HitRect(&rect, &lpps->rcPaint))
      DrawStackRect(hDC);

    /* 测试命中文本区域 */
    rect.left = drLeft;
    rect.top = dtText;
    rect.right = drLeft + dwText;
    rect.bottom = dtText + dhText;

    if (HitRect(&rect, &lpps->rcPaint))
      DrawTextRect(hDC);
  }
  else
  {
    DrawDisasmRect(hDC);
    DrawDataRect(hDC);
    DrawRegistersRect(hDC);
    DrawStackRect(hDC);
    DrawTextRect(hDC);
  }

  SetScrollBar(aDrawIndex, diCount);
}

void MC_Disasm::SingleBufferDraw(HDC hDC, LPPAINTSTRUCT lpps)
{
  if (!hDC) return;
  DeviceContextDraw(hDC, lpps);
  sbdc++;
}

BOOL MC_Disasm::DoubleBufferDraw(HDC hDC, LPPAINTSTRUCT lpps)
{
  BOOL rval;

  if (!hDC) return FALSE;

  HDC hCDC = CreateCompatibleDC(hDC);
  HBITMAP hBM = CreateCompatibleBitmap(hDC, int(cWidth), int(cHeight));

  if (hCDC)
  {
    if (hBM)
    {
      SelectObject(hCDC, hBM);
      DeleteObject(hBM);

      DeviceContextDraw(hCDC, lpps);

      rval = BitBlt(hDC, drLeft, drTop, drWidth, cHeight - drTop, hCDC, drLeft, drTop, SRCCOPY);
      DeleteDC(hCDC);
      return rval;
    }
    DeleteDC(hCDC);
  }

  return FALSE;
}

LRESULT CALLBACK MC_Disasm::disasmWndProc(HWND phWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  static BOOL csns = FALSE;
  static BOOL cswe = FALSE;
  static BOOL bltme = TRUE;
  static UINT lFlag;
  static int16_t bpx = 0;
  static int16_t bpy = 0;

  switch (Msg)
  {
  case WM_CREATE:
  {
    This->hw_vsb = CreateWindowEx(0, WC_SCROLLBAR, NULL, DEF_VCHILD | SBS_VERT, 0, 0, 0, 0, phWnd, NULL, DllBaseAddr, 0);
    This->hw_tb = CreateWindowEx(0, TOOLBARCLASSNAME, NULL,
      DEF_VCHILD | CCS_NODIVIDER | CCS_NORESIZE | TBSTYLE_TOOLTIPS | WS_BORDER, 0, 0, 0, 0, phWnd, NULL, DllBaseAddr, 0);
    SendMessage(phWnd, WM_SETFONT, (WPARAM)MainForm::hf_tam, TRUE);

    This->cdFlag0 = 0;
    This->cdFlag1 = 0;
    This->sbdc = 0;

    if (This->hw_tb)
    {
      TBBUTTON tbb;

      SendMessage(This->hw_tb, TB_SETIMAGELIST, 0, (LPARAM)MainForm::hImageList);

      tbb.iBitmap = mf->bpi_run;
      tbb.idCommand = CDV_RUN;
      tbb.fsState = TBSTATE_ENABLED;
      tbb.fsStyle = BTNS_BUTTON;
      *(PWORD)tbb.bReserved = 0;
      tbb.dwData = 0;
      tbb.iString = NULL;

      SendMessage(This->hw_tb, TB_ADDBUTTONS, 1, (LPARAM)&tbb);
      
      tbb.iBitmap = mf->bpi_pause;
      tbb.idCommand = CDV_PAUSE;
      SendMessage(This->hw_tb, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

      tbb.iBitmap = mf->bpi_stepin;
      tbb.idCommand = CDV_STEPIN;
      SendMessage(This->hw_tb, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

      tbb.iBitmap = mf->bpi_stepover;
      tbb.idCommand = CDV_STEPOVER;
      SendMessage(This->hw_tb, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

      tbb.iBitmap = 20;
      tbb.idCommand = -1;
      tbb.fsStyle = BTNS_SEP;
      SendMessage(This->hw_tb, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

      tbb.iBitmap = mf->bpi_trun;
      tbb.idCommand = CDV_TRUN;
      tbb.fsStyle = BTNS_BUTTON;
      SendMessage(This->hw_tb, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

      tbb.iBitmap = mf->bpi_tpause;
      tbb.idCommand = CDV_TPAUSE;
      SendMessage(This->hw_tb, TB_ADDBUTTONS, 1, (LPARAM)&tbb);

      tbb.iBitmap = mf->bpi_tstop;
      tbb.idCommand = CDV_TSTOP;
      SendMessage(This->hw_tb, TB_ADDBUTTONS, 1, (LPARAM)&tbb);
    }
  }
    return 0;
  case WM_SETCURSOR:
  {
    if (cswe || bpx)
    {
      SetCursor(LoadCursor(NULL, IDC_SIZEWE));
      return 1;
    }
    else if (csns || bpy)
    {
      SetCursor(LoadCursor(NULL, IDC_SIZENS));
      return 1;
    }
  }
    break;
  case WM_MOUSEMOVE:
  {
    int16_t px = LOWORD(lParam);
    int16_t py = HIWORD(lParam);

    if (bltme)
    {
      TRACKMOUSEEVENT tme;

      bltme = FALSE;
      tme.cbSize = sizeof(tme);
      tme.hwndTrack = This->m_hWnd;
      tme.dwFlags = TME_LEAVE;
      _TrackMouseEvent(&tme);
    }

    if (bpx)
    {
      if (px < This->drLeft) break;

      if (lFlag == DLFG_ASMREG)     // asm | reg
      {
        This->asmWidth = px - This->asmLeft;
        if (This->asmWidth < 380) This->asmWidth = 380;
        if ((This->drWidth - This->asmWidth) < 120) This->asmWidth = This->drWidth - 120;

        This->regLeft = This->asmLeft + This->asmWidth;
        This->regWidth = This->drWidth - This->asmWidth;
        This->UpdateDisasmSubRect();
        MoveWindow(This->hw_vsb, This->asmWidth - 22, This->asmregTop + 2, 20, This->asmregHeight - 4, TRUE);
        This->OuterRedrawDisasmAndRegistersRect();
      } 
      else if (lFlag == DLFG_DLADDR)    //dlAddr
      {
        int16_t pos = px - This->dlAddr;

        if (pos > 0)
        {
          if ((This->dlNote + pos) > (This->asmRight - 40))
            pos = (This->asmRight - 40) - This->dlNote;

          This->dlNote += pos;
          This->dlInfo += pos;
          This->dlMnem += pos;
          This->dlByte += pos;
          This->dlAddr += pos;

          //This->OuterRedrawDisasmSubRect(DDSR_ADDRESS | DDSR_BYTES | DDSR_MNEMONIC | DDSR_CITE | DDSR_NOTE);
          This->OuterRedrawDisasmRect();
        }
        else if (pos < 0)
        {
          if ((int(This->dlAddr) + pos) < int(This->asmLeft + 40))
            pos = short(This->asmLeft + 40) - short(This->dlAddr);

          This->dlAddr += pos;
          This->dlByte += pos;
          This->dlMnem += pos;
          This->dlInfo += pos;
          This->dlNote += pos;
          //This->OuterRedrawDisasmSubRect(DDSR_ADDRESS | DDSR_BYTES | DDSR_MNEMONIC | DDSR_CITE | DDSR_NOTE);
          This->OuterRedrawDisasmRect();
        }
      }
      else if (lFlag == DLFG_DLBYTE)    //dlByte
      {
        int16_t pos = px - This->dlByte;

        if (pos > 0)
        {
          if ((This->dlNote + pos) > (This->asmRight - 40))
            pos = (This->asmRight - 40) - This->dlNote;

          This->dlNote += pos;
          This->dlInfo += pos;
          This->dlMnem += pos;
          This->dlByte += pos;

          This->OuterRedrawDisasmRect();
        }
        else if (pos < 0)
        {
          if ((int(This->dlByte) + pos) < int(This->dlAddr + 40))
            pos = short(This->dlAddr + 40) - short(This->dlByte);

          This->dlByte += pos;
          This->dlMnem += pos;
          This->dlInfo += pos;
          This->dlNote += pos;
          This->OuterRedrawDisasmRect();
        }
      }
      else if (lFlag == DLFG_DLMNEM)    //dlMnem
      {
        int16_t pos = px - This->dlMnem;

        if (pos > 0)
        {
          if ((This->dlNote + pos) > (This->asmRight - 40))
            pos = (This->asmRight - 40) - This->dlNote;

          This->dlNote += pos;
          This->dlInfo += pos;
          This->dlMnem += pos;

          This->OuterRedrawDisasmRect();
        }
        else if (pos < 0)
        {
          if ((int(This->dlMnem) + pos) < int(This->dlByte + 40))
            pos = short(This->dlByte+ 40) - short(This->dlMnem);

          This->dlMnem += pos;
          This->dlInfo += pos;
          This->dlNote += pos;
          This->OuterRedrawDisasmRect();
        }
      }
      else if (lFlag == DLFG_DLINFO)
      {
        int16_t pos = px - This->dlInfo;

        if (pos > 0)
        {
          if ((This->dlNote + pos) > (This->asmRight - 40))
            pos = (This->asmRight - 40) - This->dlNote;

          This->dlNote += pos;
          This->dlInfo += pos;

          This->OuterRedrawDisasmRect();
        }
        else if (pos < 0)
        {
          if ((int(This->dlInfo) + pos) < int(This->dlMnem + 40))
            pos = short(This->dlMnem + 40) - short(This->dlInfo);

          This->dlInfo += pos;
          This->dlNote += pos;
          This->OuterRedrawDisasmRect();
        }
      }
      else if (lFlag == DLFG_DLNOTE)    //dlNote
      {
        int16_t pos = px - This->dlNote;

        if (pos > 0)
        {
          if ((This->dlNote + pos) > (This->asmRight - 40))
            pos = (This->asmRight - 40) - This->dlNote;

          This->dlNote += pos;

          This->OuterRedrawDisasmRect();
        }
        else if (pos < 0)
        {
          if ((int(This->dlNote) + pos) < int(This->dlInfo + 40))
            pos = short(This->dlInfo + 40) - short(This->dlNote);

          This->dlNote += pos;
          This->OuterRedrawDisasmRect();
        }
      }
      else   //dat | stk
      {
        This->datWidth = px - This->datLeft;
        if (This->datWidth < 220) This->datWidth = 220;
        if ((This->drWidth - This->datWidth) < 180) This->datWidth = This->drWidth - 180;
        This->stkLeft = This->datLeft + This->datWidth;
        This->stkWidth = This->drWidth - This->datWidth;
        This->OuterRedrawDataAndStackRect();
      }
      return 0;
    }
    else if (bpy)
    {
      if (py < This->drTop) break;

      This->asmregHeight = py - This->asmregTop;
      if (This->asmregHeight < 140) This->asmregHeight = 140;
      if ((This->drHeight - This->asmregHeight) < 120) This->asmregHeight = This->drHeight - 120;
      This->datstkTop = This->asmregTop + This->asmregHeight;
      This->datstkHeight = This->drHeight - This->asmregHeight;

      This->aMaxItem = (This->asmregHeight - DASM_SPACE) / 20;
      if ((This->asmregHeight - DASM_SPACE) % 20) This->aMaxItem++;
      MoveWindow(This->hw_vsb, This->asmWidth - 22, This->asmregTop + 2, 20, This->asmregHeight - 4, TRUE);
      This->OuterRedrawAllRect();
      //This->InvalidateHeightRect(VALMIN(bpy,py),VALMAX(bpy,py));
      bpy = py;
      return 0;
    }
    else if ((py >= This->asmregTop + 2) && (py <= This->datstkTop - 2))     //asm | reg
    {
      if ( ((px >= This->regLeft - 1) && (px <= This->regLeft + 2)) ||
        ((px >= This->dlAddr - 1) && (px <= This->dlAddr + 1)) ||
        ((px >= This->dlByte - 1) && (px <= This->dlByte + 1)) ||
        ((px >= This->dlMnem - 1) && (px <= This->dlMnem + 1)) ||
        ((px >= This->dlInfo - 1) && (px <= This->dlInfo + 1)) ||
        ((px >= This->dlNote - 1) && (px <= This->dlNote + 1)) )
      {
        csns = FALSE;
        cswe = TRUE;
        return 0;
      }
    }
    else if ((px >= This->stkLeft -1) && (px <= This->stkLeft + 2) &&
      (py >= This->datstkTop + 2) && (py <= (This->datstkTop + This->datstkHeight - 2)))   //dat | stk
    {
      csns = FALSE;
      cswe = TRUE;
      return 0;
    }
    else if((py >= This->datstkTop - 2) && (py <= This->datstkTop + 2)) //asm ----
    {
      csns = TRUE;
      cswe = FALSE;
      return 0;
    }

    csns = FALSE;
    cswe = FALSE;
    bpx = 0;
    bpy = 0;
  }
    break;
  case WM_PAINT:
    if (This->blDraw)
    {
      PAINTSTRUCT ps;
      HDC hDC = BeginPaint(phWnd, &ps);
      
      if (!This->DoubleBufferDraw(hDC, &ps))        //优先使用双缓冲区绘制,如果失败再尝试单缓冲区绘制
        This->SingleBufferDraw(hDC, &ps);

      EndPaint(phWnd, &ps);
    }
    break;
  case WM_ERASEBKGND:
    return TRUE;
  case WM_LBUTTONDOWN:
  {
    PDAI HitItem;
    int16_t px = LOWORD(lParam);
    int16_t py = HIWORD(lParam);
    
    if (px < 0 || py < 0) break;

    SetFocus(phWnd);

    if ((py >= This->asmregTop + 2) && (py <= This->datstkTop - 2))
    {
      if ((px >= This->regLeft - 1) && (px <= This->regLeft + 2))
      {
        bpx = px;
        lFlag = DLFG_ASMREG;
        SetCapture(phWnd);
        return 0;
      }
      else if ((px >= This->dlAddr - 1) && (px <= This->dlAddr + 1))
      {
        bpx = px;
        lFlag = DLFG_DLADDR;
        SetCapture(phWnd);
        return 0;
      }
      else if ((px >= This->dlByte - 1) && (px <= This->dlByte + 1))
      {
        bpx = px;
        lFlag = DLFG_DLBYTE;
        SetCapture(phWnd);
        return 0;
      }
      else if ((px >= This->dlMnem - 1) && (px <= This->dlMnem + 1))
      {
        bpx = px;
        lFlag = DLFG_DLMNEM;
        SetCapture(phWnd);
        return 0;
      }
      else if ((px >= This->dlInfo - 1) && (px <= This->dlInfo + 1))
      {
        bpx = px;
        lFlag = DLFG_DLINFO;
        SetCapture(phWnd);
        return 0;
      }
      else if ((px >= This->dlNote - 1) && (px <= This->dlNote + 1))
      {
        bpx = px;
        lFlag = DLFG_DLNOTE;
        SetCapture(phWnd);
        return 0;
      }
    }
    else if ((px >= This->stkLeft - 1) && (px <= This->stkLeft + 2) &&
      (py >= This->datstkTop + 2) && (py <= (This->datstkTop + This->datstkHeight - 2)))
    {
      bpx = px;
      lFlag = DLFG_DATSTK;
      SetCapture(phWnd);
      return 0;
    }
    else if ((py >= This->datstkTop - 2) && (py <= This->datstkTop + 2))
    {
      bpy = py;
      SetCapture(phWnd);
      return 0;
    }

    HitItem = This->HitTest(px, py);

    if (!This->aSelAddrA)
    {
      if (!HitItem)
        break;
      else
        This->aSelAddrA = HitItem->lpVirtAddr;
    }

    if (wParam & MK_SHIFT)    //Shift被按下
    {
      if (HitItem)            //有项目被击中
      {
        This->aSelAddrA = VALMIN(This->aSelAddrA, HitItem->lpVirtAddr);
        This->aSelAddrB = VALMAX(This->aSelAddrA, HitItem->lpVirtAddr);
      }
      else
        This->aSelAddrB = This->aSelAddrA;
    }
    else
    {
      if (HitItem)            //有项目被击中
      {
        This->aSelAddrA = HitItem->lpVirtAddr;
        This->aSelAddrB = HitItem->lpVirtAddr;
      }
    }
    This->OuterRedrawDisasmRect();
  }
    break;
  case WM_LBUTTONUP:
    if (bpx || bpy)
    {
      csns = FALSE;
      cswe = FALSE;
      if (bpx || bpy)
      {
        bpx = 0;
        bpy = 0;
        ReleaseCapture();
      }
      return 0;
    }
    break;
  case WM_MOUSELEAVE:
    csns = FALSE;
    cswe = FALSE;
    bltme = TRUE;
    if (bpx || bpy)
    {
      bpx = 0;
      bpy = 0;
      ReleaseCapture();
    }
    return 0;
  case WM_SIZE:
    if (wParam != SIZE_MINIMIZED)
    {
      WORD Width = LOWORD(lParam);
      WORD Height = HIWORD(lParam);

      This->cWidth = Width;
      This->cHeight = Height;

      //计算自绘区域大小
      This->drWidth = Width;
      This->drHeight = Height - 24 - This->dhText;

      //计算反汇编区域
      This->asmLeft = This->drLeft;
      This->asmregTop = This->drTop;

      if (This->asmWidth + 240 > This->drWidth)
        This->asmWidth = This->drWidth - 240;
      if (This->asmregHeight + 120 > This->drHeight)
        This->asmregHeight = This->drHeight - 120;

      This->UpdateDisasmSubRect();

      //计算数据dump区域
      This->datLeft = This->drLeft ;
      This->datstkTop = This->asmregTop + This->asmregHeight;
      if (This->datWidth + 180 > This->drWidth)
        This->datWidth = This->drWidth - 180;
      This->datstkHeight = This->drHeight - This->asmregHeight;

      //计算寄存器区域
      This->regLeft = This->asmLeft + This->asmWidth;
      This->regWidth = This->drWidth - This->asmWidth;
      if (This->asmregHeight + 120 > This->drHeight)
        This->asmregHeight = This->drHeight - 120;

      //计算栈区域
      This->stkLeft = This->datLeft + This->datWidth;
      This->stkWidth = This->drWidth - This->datWidth;

      //计算文本区域
      This->dwText = Width;
      This->dtText = Height - This->dhText;

      //计算反汇编条目数量
      This->aMaxItem = (This->asmregHeight - DASM_SPACE) / 20;
      
      if ((This->asmregHeight - DASM_SPACE) % 20) This->aMaxItem++;

      MoveWindow(This->hw_tb, 0, 0, Width, 24, TRUE);
      MoveWindow(This->hw_vsb, This->asmWidth - 22, This->asmregTop + 2, 20, This->asmregHeight - 4, TRUE);
    }
    break;
  case WM_SIZING:
    if (wParam >= WMSZ_LEFT && wParam <= WMSZ_BOTTOMRIGHT)
    {
      LPRECT lprc = (LPRECT)lParam;
      long Width = lprc->right - lprc->left;
      long Height = lprc->bottom - lprc->top;

      if (Width < 680) lprc->right = lprc->left + 680;
      if (Height < 480) lprc->bottom = lprc->top + 480;
    }
    return 1;
  case WM_MOUSEWHEEL:
  {
    SCROLLINFO SI;
    int16_t mwu = HIWORD(wParam);

    if (mwu == 0) break;
    ZeroMemory(&SI, sizeof(SCROLLINFO));
    SI.cbSize = sizeof(SCROLLINFO);
    SI.fMask = SIF_RANGE | SIF_POS;
    GetScrollInfo((HWND)This->hw_vsb, SB_CTL, &SI);

    if (mwu < 0)
    {
      SI.nPos += 2;
      if (SI.nPos > SI.nMax) SI.nPos = SI.nMax;
    }

    if (mwu > 0)
    {
      SI.nPos -= 2;
      if (SI.nPos < SI.nMin) SI.nPos = SI.nMin;
    }

    This->aDrawIndex = SI.nPos;
    This->aDrawAddr = This->ItemArray[SI.nPos]->lpVirtAddr;
    SetScrollPos((HWND)This->hw_vsb, SB_CTL, SI.nPos, TRUE);
    This->OuterRedrawDisasmRect();
  }
    return 0;
  case WM_RBUTTONDOWN:
  {
    SetFocus(phWnd);

    PDAI HitItem = This->HitTest(LOWORD(lParam), HIWORD(lParam));
    if (HitItem)
    {
      This->aSelAddrA = HitItem->lpVirtAddr;
      This->aSelAddrB = HitItem->lpVirtAddr;
      This->OuterRedrawDisasmRect();
    }
    return 0;
  }
  case WM_LBUTTONDBLCLK:
  {
    PDAI HitItem = This->HitTest(LOWORD(lParam), HIWORD(lParam));
    if (HitItem)
    {
      This->aSelAddrA = HitItem->lpVirtAddr;
      This->aSelAddrB = HitItem->lpVirtAddr;
      DialogBox(DllBaseAddr, MAKEINTRESOURCE(ASM_DLG), phWnd, AsmProc);
    }
  }
    return 0;
  case WM_KEYDOWN:
  {
    if (GetKeyState(VK_CONTROL) < 0)
    {
      if (wParam == 'G')
      {
        DialogBox(DllBaseAddr, MAKEINTRESOURCE(GOTOADDR_DLG), phWnd, GotoAddrProc);
        return 0;
      }
      else if (wParam == 'R')
      {
        This->fw_reg->CreateForm();
      }
    }
  }
    break;
  case WM_VSCROLL:
  {
    if (lParam != (LPARAM)This->hw_vsb) break;

    int snPos;
    SCROLLINFO SI;

    ZeroMemory(&SI, sizeof(SCROLLINFO));
    SI.cbSize = sizeof(SCROLLINFO);
    SI.fMask = SIF_RANGE | SIF_POS | SIF_TRACKPOS;
    GetScrollInfo((HWND)lParam, SB_CTL, &SI);
    if (SI.nMax == 0 || SI.nMax < SI.nMin) return 0;

    switch (LOWORD(wParam))
    {
    case SB_LINEUP:
      snPos = SI.nPos;
      snPos--;
      break;
    case SB_LINEDOWN:
      snPos = SI.nPos;
      snPos++;
      break;
    case SB_PAGEUP:
      snPos = SI.nPos;
      snPos -= This->aMaxItem;
      break;
    case SB_PAGEDOWN:
      snPos = SI.nPos;
      snPos += This->aMaxItem;
      break;
    case SB_THUMBTRACK:
      snPos = SI.nTrackPos;
      break;
    default:
      return 0;
    }
    if (snPos < SI.nMin) snPos = SI.nMin;
    if (snPos > SI.nMax) snPos = SI.nMax;

    This->aDrawIndex = snPos;
    This->aDrawAddr = This->ItemArray[snPos]->lpVirtAddr;
    SetScrollPos((HWND)lParam, SB_CTL, snPos, TRUE);
    This->OuterRedrawDisasmRect();
  }
    return 0;
  case WM_DESTROY:
    mf->SetDbgState(DS_Busy);

    if (This->fw_reg)
    {
      This->fw_reg->DestroyForm();
      This->fw_reg = NULL;
    }

    This->DelItemArray();
    This->ItemArray = NULL;
    This->aBaseAddr = NULL;
    This->aBlockSize = NULL;
    This->aDrawAddr = NULL;
    This->aSelAddrA = NULL;
    This->drWidth = NULL;
    This->drHeight = NULL;
    This->aMaxItem = NULL;

    This->m_hWnd = NULL;
    mf->SetDbgState(DS_Idle);
    return 0;
  }

  return DefMDIChildProc(phWnd, Msg, wParam, lParam);
}