#include "../sr_pch.h"

#ifdef ALLOC_PARAM
#undef ALLOC_PARAM
#endif

#ifdef ALLOC_PARAMEX
#undef ALLOC_PARAMEX
#endif


#define ALLOC_PARAM(tvis, lptv, ncode, idx) \
  lptv = (PVB_TV_TYPE)calloc(1, sizeof(VB_TV_TYPE)); \
  if (lptv != NULL) { \
    lptv->TV.Sign = MY_MAGIC; \
    lptv->TV.NCode = (WORD)ncode; \
    lptv->TV.Index = (WORD)idx; } \
  tvis.item.lParam = (LPARAM)lptv

#define ALLOC_PARAMEX(tvis, lptv, ncode, idx, exparam) \
  lptv = (PVB_TV_TYPE)calloc(1, sizeof(VB_TV_TYPE)); \
  if (lptv != NULL) { \
    lptv->TV.Sign = MY_MAGIC; \
    lptv->TV.NCode = (WORD)ncode; \
    lptv->TV.Index = (WORD)idx; \
    lptv->ExParam = (LPARAM)exparam; } \
  tvis.item.lParam = (LPARAM)lptv

static WNDPROC wpVbTV = NULL;
MC_VBInfo * MC_VBInfo::This = NULL;

MC_VBInfo::~MC_VBInfo()
{
  if (this->m_hWnd)
  {
    DestroyWindow(this->m_hWnd);
    this->m_hWnd = NULL;
  }

  if (this->hMenu_pop_4)
  {
    DestroyMenu(this->hMenu_pop_4);
    hMenu_pop_4 = NULL;
  }
  This = NULL;
}

HWND MC_VBInfo::CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style)
{
  if (this->m_hWnd)
  {
    this->SetTop();
    return this->m_hWnd;
  }

  MDICREATESTRUCT MCS;

  MCS.szClass = (PWCHAR)mf->rcls_mc_vbinfo;
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

//解析VB头部结构
void MC_VBInfo::ParseHeader(PVBHDR lpVbHdr)
{
#define PRIV_BUFLEN         64
  PVBPUBOBJ lpVbObj;
  PVB_TV_TYPE lpVbTV;
  PVBCRI lpvbcri;
  DWORD i, j, ci;
  uint32_t osRegInfo;
  size_t RetVal;
  HTREEITEM hti_LastForm;
  HTREEITEM hti_LastItem;
  WORD frmCount = 0;  //窗体计数
  WORD basCount = 0;  //模块计数
  WORD clsCount = 0;  //类模块计数
  WORD ctlCount = 0;  //用户控件计数
  WORD ObjCount, k;
  TV_INSERTSTRUCT tvis;
  WCHAR sTmp0[MAX_PATH];
  WCHAR sTmp1[PRIV_BUFLEN];
  WCHAR sTmp2[PRIV_BUFLEN];

  __try
  {
    //判断头部结构标志
    if (lpVbHdr->Sign.dwSign != VB_MAGIC)
    {
      mf->SetStateText(L"VB结构标志不正确!", RGB(255, 0, 0));
      return;
    }

    //重置控件内容
    ResetView();

    lp_VbHdr = lpVbHdr;

    /* 打印vbhdr结构地址 */
    swprintf_s(sTmp0, MAX_PATH, HEXFORMAT, lpVbHdr);
    /* 设置文本 */
    SendMessage(hEdit_lphdr, WM_SETTEXT, 0, (LPARAM)sTmp0);

    lpVbObj = lpVbHdr->lpProjInfo->lpObjectTable->lpObjectArray;
    ObjCount = lpVbHdr->lpProjInfo->lpObjectTable->dwTotalObjects;

    tvis.hParent = 0;
    tvis.hInsertAfter = TVI_ROOT;
    tvis.item.mask = TVIF_TEXT | TVIF_IMAGE | TVIF_SELECTEDIMAGE | TVIF_PARAM;
    tvis.item.iImage = MainForm::bpi_root;
    tvis.item.iSelectedImage = MainForm::bpi_root;

    ALLOC_PARAM(tvis, lpVbTV, VBPC_NULLITEM, 0);
    tvis.item.pszText = L"VB结构";
    hti_struct = ctv_vb.InsertItem(&tvis);

    tvis.hParent = hti_struct;
    tvis.item.iImage = MainForm::bpi_vbs;
    tvis.item.iSelectedImage = MainForm::bpi_vbs;

    ALLOC_PARAM(tvis, lpVbTV, VBPC_STRUCT, VBII_VBHDR);
    tvis.item.pszText = L"VB 头部";                    //添加VB基础结构节点
    lpVbTV->TV.hItem = ctv_vb.InsertItem(&tvis);

    ALLOC_PARAM(tvis, lpVbTV, VBPC_STRUCT, VBII_PRJI);
    tvis.item.pszText = L"VB 工程信息";
    lpVbTV->TV.hItem = ctv_vb.InsertItem(&tvis);

    ALLOC_PARAM(tvis, lpVbTV, VBPC_STRUCT, VBII_COMD);
    tvis.item.pszText = L"VB COM注册数据";
    lpVbTV->TV.hItem = ctv_vb.InsertItem(&tvis);

    ALLOC_PARAM(tvis, lpVbTV, VBPC_STRUCT, VBII_COMI);
    tvis.item.pszText = L"VB COM注册信息  (...)";
    lpVbTV->TV.hItem = hti_cominfo = ctv_vb.InsertItem(&tvis);

    ALLOC_PARAM(tvis, lpVbTV, VBPC_STRUCT, VBII_OBJTAB);
    tvis.item.pszText = L"VB 对象描述表";
    lpVbTV->TV.hItem = ctv_vb.InsertItem(&tvis);

    ALLOC_PARAM(tvis, lpVbTV, VBPC_STRUCT, VBII_OBJECT);
    swprintf_s(sTmp0, MAX_PATH, L"VB 对象  (%hu)", ObjCount);
    tvis.item.pszText = sTmp0;
    lpVbTV->TV.hItem = hti_object = ctv_vb.InsertItem(&tvis);


    //
    tvis.hParent = 0;
    tvis.hInsertAfter = TVI_LAST;
    tvis.item.iImage = MainForm::bpi_root;
    tvis.item.iSelectedImage = MainForm::bpi_root;

    tvis.item.pszText = L"窗体  [Form]  (...)";
    ALLOC_PARAM(tvis, lpVbTV, VBPC_NULLITEM, 1);
    hti_frm = ctv_vb.InsertItem(&tvis);
    lpVbTV->TV.hItem = hti_frm;

    tvis.item.pszText = L"模块  [Module]  (...)";
    ALLOC_PARAM(tvis, lpVbTV, VBPC_NULLITEM, 2);
    hti_bas = ctv_vb.InsertItem(&tvis);
    lpVbTV->TV.hItem = hti_bas;

    tvis.item.pszText = L"类模块  [Class]  (...)";
    ALLOC_PARAM(tvis, lpVbTV, VBPC_NULLITEM, 3);
    hti_cls = ctv_vb.InsertItem(&tvis);
    lpVbTV->TV.hItem = hti_cls;

    tvis.item.pszText = L"用户控件  [UserControl]  (...)";
    ALLOC_PARAM(tvis, lpVbTV, VBPC_NULLITEM, 4);
    hti_ctl = ctv_vb.InsertItem(&tvis);
    lpVbTV->TV.hItem = hti_ctl;

    mf->SetDbgState(DS_Busy);

    osRegInfo = lpVbHdr->lpComRegData->oRegInfo;
    ci = 0;

    tvis.hParent = hti_cominfo;
    tvis.item.iImage = MainForm::bpi_info;
    tvis.item.iSelectedImage = MainForm::bpi_info;
    tvis.item.pszText = sTmp0;

    //遍历COM注册信息链表
    while (osRegInfo != 0)
    {
      lpvbcri = (PVBCRI)MAKE_UINTPTR(lpVbHdr->lpComRegData, osRegInfo);
      
      if (!TryCopyWCharFromChar(sTmp0, MAX_PATH, (PCHAR)MAKE_UINTPTR(lpVbHdr->lpComRegData, lpvbcri->oObjectName)))
        swprintf_s(sTmp0, MAX_PATH, L"_ComRegInfo_%u", ci);

      ALLOC_PARAM(tvis, lpVbTV, VBPC_STRUCT, VBII_COMISUB);
      lpVbTV->ExParam = (LPARAM)lpvbcri;
      lpVbTV->TV.hItem = ctv_vb.InsertItem(&tvis);

      ci++;
      osRegInfo = lpvbcri->oNextObject;
    }

    swprintf_s(sTmp0, MAX_PATH, L"VB COM注册信息  (%u)", (DWORD)ci);
    ctv_vb.SetItemText(hti_cominfo, sTmp0);

    //遍历Objects并计数
    for (i = 0; i < ObjCount; i++)    
    {
      PVBOOI lpOptObj;    //VB对象可选信息头
      PVBCTRL lpCtrl;
      PPVBEP lpEP;
      PVBOBJLST lpVbObjLst;

      tvis.hParent = hti_object;
      tvis.item.iImage = MainForm::bpi_object;
      tvis.item.iSelectedImage = MainForm::bpi_object;
      tvis.item.pszText = sTmp0;

      ALLOC_PARAMEX(tvis, lpVbTV, VBPC_OBJECT, i, &lpVbObj[i]);
      
      if (!TryCopyWCharFromChar(sTmp0, MAX_PATH, lpVbObj[i].lpSZObjectName))
        swprintf_s(sTmp0, MAX_PATH, L"_obj%hu", i);

      tvis.hParent = lpVbTV->TV.hItem = ctv_vb.InsertItem(&tvis);
      tvis.item.iImage = MainForm::bpi_info;
      tvis.item.iSelectedImage = MainForm::bpi_info;

      tvis.item.pszText = L"对象信息";
      ALLOC_PARAMEX(tvis, lpVbTV, VBPC_OBJINFO, i, lpVbObj[i].lpObjectInfo);
      lpVbTV->TV.hItem = ctv_vb.InsertItem(&tvis);

      if (CHK_FLAG(lpVbObj[i].fObjectType, 0x02))  //存在可选信息
      {
        tvis.item.pszText = L"可选信息";
        ALLOC_PARAMEX(tvis, lpVbTV, VBPC_OBJOPTINFO, i, MAKE_UINTPTR(lpVbObj[i].lpObjectInfo, sizeof(VB_OBJECT_INFO)));
        lpVbTV->TV.hItem = ctv_vb.InsertItem(&tvis);
      }

      tvis.hParent = hti_object;
      tvis.item.iImage = MainForm::bpi_object;
      tvis.item.iSelectedImage = MainForm::bpi_object;
      tvis.item.pszText = sTmp0;

      //判断窗体
      if (CHK_FLAG(lpVbObj[i].fObjectType, 0x80))
      {
        frmCount++;

        tvis.hParent = hti_frm;
        tvis.item.iImage = MainForm::bpi_frm;
        tvis.item.iSelectedImage = MainForm::bpi_frm;
        tvis.item.pszText = sTmp0;
        ALLOC_PARAM(tvis, lpVbTV, VBPC_ROOTFORM, i);

        if ((lpVbObjLst = mf->GetObjectListByObject(&lpVbObj[i])) == NULL)
        {
          VBOBJLST VbObjL;
          PWCHAR pstmp;
          size_t slen = strlen(lpVbObj[i].lpSZObjectName);

          memset(&VbObjL, 0, sizeof(VbObjL));

          if (!slen)
          {
            pstmp = (PWCHAR)mf->m_hmm->hcalloc(16, sizeof(WCHAR));
            swprintf_s(pstmp, 16, L"_frm_%hu", lpVbObj[i].lpObjectInfo->wObjectIndex);
          }
          else
          {
            pstmp = (PWCHAR)mf->m_hmm->hcalloc(slen + 1);
            mbstowcs_s(&RetVal, pstmp, slen + 1, lpVbObj[i].lpSZObjectName, _TRUNCATE);
          }

          VbObjL.VbHead = lpVbHdr;
          VbObjL.VbObj = &lpVbObj[i];
          VbObjL.iIndex = lpVbObj[i].lpObjectInfo->wObjectIndex;
          VbObjL.ObjName = pstmp;

          lpVbObjLst = mf->AddObjectList(&VbObjL);
        }
        tvis.item.pszText = lpVbObjLst->ObjName;
        hti_LastForm = ctv_vb.InsertItem(&tvis);
        lpVbTV->TV.hItem = hti_LastForm;

        lpOptObj = (PVBOOI)((DWORD)lpVbObj[i].lpObjectInfo + sizeof(VBOI));
        lpCtrl = lpOptObj->lpControls;

        for (j = 0; j < lpOptObj->wControlCount; j++)  //遍历控件
        {
          tvis.hParent = hti_LastForm;
          tvis.item.iImage = MainForm::bpi_ukctl;
          tvis.item.iSelectedImage = MainForm::bpi_ukctl;
          tvis.item.pszText = sTmp0;
          ALLOC_PARAM(tvis, lpVbTV, VBPC_FORM, j);

          if (strlen(lpCtrl[j].lpszName) == 0)
            swprintf_s(sTmp0, MAX_PATH, L"_ctl%u", j + 1);
          else
            mbstowcs_s(&RetVal, sTmp0, (size_t)MAX_PATH, lpCtrl[j].lpszName, _TRUNCATE);

          hti_LastItem = ctv_vb.InsertItem(&tvis);
          lpVbTV->TV.hItem = hti_LastItem;       //保存当前控件
          wcscpy_s(sTmp1, PRIV_BUFLEN, sTmp0);      //备份控件名
          tvis.hParent = hti_LastItem;

          lpEP = (PPVBEP)((DWORD)lpCtrl[j].lpEventHandlerTable + sizeof(VB_EVENT_TABLE));
          for (k = 0; k < lpCtrl[j].wEventHandlerCount; k++)          //遍历控件事件
          {
            if (lpEP[k] == NULL || lpEP[k]->lpDescInfo == NULL) continue;
            PVBPL pVbProcL;
            if (!(pVbProcL = mf->GetProcListByAddress(lpVbObjLst->ObjName, lpEP[k]->lpDescInfo)))
            {
              VBPL VbProcL;
              PWCHAR pstmp = 0;
              PVOID TT;
              DWORD LE;

              memset(&VbProcL, 0, sizeof(VBPL));
              pstmp = (PWCHAR)mf->m_hmm->hcalloc(64, sizeof(WCHAR));

              if (!GetMemberNameByIndex(*lpCtrl[j].lpGuid, (DWORD)k, sTmp2, 64, 0))
                swprintf_s(sTmp2, 64, L"Event_%hu", k);
              swprintf_s(pstmp, 64, L"%s_%s", sTmp1, sTmp2);
              VbProcL.pVbCtl = &lpCtrl[j];
              VbProcL.iIndex = k;
              VbProcL.ProcDesc = lpEP[k]->lpDescInfo;
              VbProcL.EvtName = pstmp;
              pVbProcL = mf->AddProcListByObject(lpVbObjLst, &VbProcL);
            }

            tvis.item.iImage = MainForm::bpi_event;
            tvis.item.iSelectedImage = MainForm::bpi_event;
            ALLOC_PARAM(tvis, lpVbTV, VBPC_FRMCTL, k);
            tvis.item.pszText = pVbProcL->EvtName;
            lpVbTV->TV.hItem = ctv_vb.InsertItem(&tvis);
            tvis.item.pszText = sTmp0;
          }
        }
      }
      else if (CHK_NOFL(lpVbObj[i].fObjectType, 0x02)) //判断模块
      {
        basCount++;

        tvis.hParent = hti_bas;
        tvis.item.iImage = MainForm::bpi_bas;
        tvis.item.iSelectedImage = MainForm::bpi_bas;
        ALLOC_PARAM(tvis, lpVbTV, VBPC_BAS, i);

        if (strlen(lpVbObj[i].lpSZObjectName) == 0)
        {
          swprintf_s(sTmp0, MAX_PATH, L"_bas%hu", i + 1);
          tvis.item.pszText = sTmp0;
          lpVbTV->TV.hItem = TreeView_InsertItem(hTV_vb, &tvis);
        }
        else
        {
          tvis.item.pszText = (PWCHAR)lpVbObj[i].lpSZObjectName;
          lpVbTV->TV.hItem = TreeView_InsertItemA(hTV_vb, &tvis);
        }
      }
      else if (CHK_FLAG(lpVbObj[i].fObjectType, 0x001DA003)) //判断用户控件
      {
        ctlCount++;

        tvis.hParent = hti_ctl;
        tvis.item.iImage = MainForm::bpi_ctl;
        tvis.item.iSelectedImage = MainForm::bpi_ctl;
        ALLOC_PARAM(tvis, lpVbTV, VBPC_UC, i);

        if (strlen(lpVbObj[i].lpSZObjectName) == 0)
        {
          swprintf_s(sTmp0, MAX_PATH, L"_ctl%hu", i + 1);
          tvis.item.pszText = sTmp0;
          lpVbTV->TV.hItem = TreeView_InsertItem(hTV_vb, &tvis);
        }
        else
        {
          tvis.item.pszText = (PWCHAR)lpVbObj[i].lpSZObjectName;
          lpVbTV->TV.hItem = TreeView_InsertItemA(hTV_vb, &tvis);
        }
      }
      else if (CHK_FLAG(lpVbObj[i].fObjectType, 0x00018003)) //判断类模块
      {
        clsCount++;

        tvis.hParent = hti_cls;
        tvis.item.iImage = MainForm::bpi_cls;
        tvis.item.iSelectedImage = MainForm::bpi_cls;
        ALLOC_PARAM(tvis, lpVbTV, VBPC_CLS, i);

        if (strlen(lpVbObj[i].lpSZObjectName) == 0)
        {
          swprintf_s(sTmp0, MAX_PATH, L"_cls%hu", i + 1);
          tvis.item.pszText = sTmp0;
          lpVbTV->TV.hItem = TreeView_InsertItem(hTV_vb, &tvis);
        }
        else
        {
          tvis.item.pszText = (PWCHAR)lpVbObj[i].lpSZObjectName;
          lpVbTV->TV.hItem = TreeView_InsertItemA(hTV_vb, &tvis);
        }

      }
    }

    tvis.item.pszText = sTmp0;

    swprintf_s(sTmp0, MAX_PATH, L"窗体  [Form]  (%u)", (DWORD)frmCount);
    ctv_vb.SetItemText(hti_frm, sTmp0);
    swprintf_s(sTmp0, MAX_PATH, L"模块  [Module]  (%u)", (DWORD)basCount);
    ctv_vb.SetItemText(hti_bas, sTmp0);
    swprintf_s(sTmp0, MAX_PATH, L"类模块  [Class]  (%u)", (DWORD)clsCount);
    ctv_vb.SetItemText(hti_cls, sTmp0);
    swprintf_s(sTmp0, MAX_PATH, L"用户控件  [UserControl]  (%u)", (DWORD)ctlCount);
    ctv_vb.SetItemText(hti_ctl, sTmp0);

    mf->SetDbgState(DS_Idle);
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    ResetView();
    mf->SetStateText(L"在解析VB结构时访问内存错误.", RGB(255, 0, 0));
    return;
  }

  return;
#undef PRIV_BUFLEN
}

void MC_VBInfo::ParseModule(PVOID lpMod)
{
  if (this->m_hWnd == NULL) return;

  __try
  {
    PIMAGE_DOS_HEADER lpDos = (PIMAGE_DOS_HEADER)lpMod;
    if (lpDos->e_magic != IMAGE_DOS_SIGNATURE) return;
    PIMAGE_NT_HEADERS lpNt = (PIMAGE_NT_HEADERS)((DWORD)lpMod + lpDos->e_lfanew);
    if (lpNt->Signature != IMAGE_NT_SIGNATURE) return;
    PBYTE OEP = (PBYTE)((DWORD)lpMod + lpNt->OptionalHeader.AddressOfEntryPoint);

    //push xxxxxxxx; call xxxxxxxx
    if (OEP[0] != 0x68 || OEP[5] != 0xE8)
    {
      mf->SetStateText(L"未定位到关于VB结构的代码!", RGB(255, 0, 0));
      return;
    }

    PVBHDR lpVbHdr = (PVBHDR)(*((PDWORD)((DWORD)OEP + 1)));

    if (lpVbHdr->Sign.dwSign != VB_MAGIC)
    {
      mf->SetStateText(L"VB结构标志不正确!", RGB(255, 0, 0));
      return;
    }

    ParseHeader(lpVbHdr);
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    mf->SetStateText(L"在解析VB头部结构时读取内存错误.", RGB(255, 0, 0));
    return;
  }
}


void MC_VBInfo::PrintThreadFlags(uint32_t flgThread, PWCHAR strBuffer, size_t maxLength)
{
  if (!strBuffer || !maxLength) return;

  strBuffer[0] = 0;

  if (CHK_FLAG(flgThread, VBTF_APARTMOD))
  {
    wcscpy_s(strBuffer, maxLength, L"单元模型");
  }

  if (CHK_FLAG(flgThread, VBTF_REQUIRELIC))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");
    
    wcscat_s(strBuffer, maxLength, L"请求授权 (仅OCX)");
  }

  if (CHK_FLAG(flgThread, VBTF_UNATTENDED))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"无管理者");
  }

  if (CHK_FLAG(flgThread, VBTF_SNGTHREAD))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"单线程");
  }

  if (CHK_FLAG(flgThread, VBTF_RETAINED))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"留存");
  }
}

void MC_VBInfo::PrintOLEMISC(OLEMISC OleMisc, PWCHAR strBuffer, size_t maxLength)
{
  if (!strBuffer || !maxLength) return;

  strBuffer[0] = 0;

  if (CHK_FLAG(OleMisc, OLEMISC_RECOMPOSEONRESIZE))
  {
    wcscpy_s(strBuffer, maxLength, L"OLEMISC_RECOMPOSEONRESIZE");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_ONLYICONIC))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_ONLYICONIC");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_INSERTNOTREPLACE))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_INSERTNOTREPLACE");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_STATIC))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_STATIC");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_CANTLINKINSIDE))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_CANTLINKINSIDE");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_CANLINKBYOLE1))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_CANLINKBYOLE1");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_ISLINKOBJECT))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_ISLINKOBJECT");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_INSIDEOUT))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_INSIDEOUT");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_ACTIVATEWHENVISIBLE))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_ACTIVATEWHENVISIBLE");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_RENDERINGISDEVICEINDEPENDENT))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_RENDERINGISDEVICEINDEPENDENT");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_INVISIBLEATRUNTIME))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_INVISIBLEATRUNTIME");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_ALWAYSRUN))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_ALWAYSRUN");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_ACTSLIKEBUTTON))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_ACTSLIKEBUTTON");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_ACTSLIKELABEL))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_ACTSLIKELABEL");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_NOUIACTIVATE))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_NOUIACTIVATE");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_ALIGNABLE))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_ALIGNABLE");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_SIMPLEFRAME))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_SIMPLEFRAME");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_SETCLIENTSITEFIRST))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_SETCLIENTSITEFIRST");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_IMEMODE))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_IMEMODE");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_IGNOREACTIVATEWHENVISIBLE))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_IGNOREACTIVATEWHENVISIBLE");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_WANTSTOMENUMERGE))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_WANTSTOMENUMERGE");
  }

  if (CHK_FLAG(OleMisc, OLEMISC_SUPPORTSMULTILEVELUNDO))
  {
    if (strBuffer[0] != 0)
      wcscat_s(strBuffer, maxLength, L" | ");

    wcscat_s(strBuffer, maxLength, L"OLEMISC_SUPPORTSMULTILEVELUNDO");
  }
}

LRESULT CALLBACK MC_VBInfo::vbwpTreeView(HWND phWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
  case WM_NCHITTEST:
  {
    int nFrame = 2;
    CRect rect;
    CPoint pt(LOWORD(lParam), HIWORD(lParam));

    GetWindowRect(phWnd, &rect);
    //GetClientRect(phWnd, &rect);
    //ScreenToClient(phWnd, &pt);

    rect.DeflateRect(nFrame, nFrame);

    if (!rect.PtInRect(pt))
    {
      if (pt.x >= rect.right - nFrame)
      {
        return HTRIGHT;
      }
    }
  }
  break;
  case WM_SIZE:
  {
    if ((This != NULL) && (This->clv_vb.m_hWnd != NULL))
    {
      CRect rect;

      GetWindowRect(phWnd, &rect);

      This->clv_vb.MoveWindow(rect.Width(), 28, This->m_Width - rect.Width(), This->m_Height - 28, TRUE);
    }
  }
  break;
  }

  if (wpVbTV != NULL)
    return CallWindowProc(wpVbTV, phWnd, Msg, wParam, lParam);
  else
    return 0;
}

LRESULT CALLBACK MC_VBInfo::vbWndProc(HWND phWnd, UINT Msg, WPARAM wParam, LPARAM lParam)
{
  switch (Msg)
  {
  case WM_CREATE:
  {
    LONG ExtStyle = GetWindowLong(phWnd, GWL_EXSTYLE);

    ExtStyle |= WS_EX_CLIENTEDGE;
    SetWindowLong(phWnd, GWL_EXSTYLE, ExtStyle);

    This->hEdit_lphdr = CreateWindowEx(WS_EX_CLIENTEDGE, CN_EDIT, T_NULL, DEF_VCHILD | ES_CENTER, 80, 4, 80, 20, phWnd, 0, DllBaseAddr, 0);
    This->hBtn_vbok = CreateWindowEx(0, CN_BUTTON, L"分析", DEF_VCHILD, 170, 2, 50, 24, phWnd, 0, DllBaseAddr, 0);
    This->hBtn_sethk = CreateWindowEx(0, CN_BUTTON, L"HOOK结构指针", DEF_VCHILD, 230, 2, 100, 24, phWnd, 0, DllBaseAddr, 0);
    This->hTV_vb = CreateWindowEx(WS_EX_CLIENTEDGE, WC_TREEVIEW, TN_VBTREE, TV_STYLE, 0, 0, 0, 0, phWnd, NULL, DllBaseAddr, 0);
    This->hLV_vb = CreateWindowEx(WS_EX_CLIENTEDGE, WC_LISTVIEW, LN_VBLIST, LV_STYLE, 0, 0, 0, 0, phWnd, NULL, DllBaseAddr, 0);
    ListView_SetExtendedListViewStyle(This->hLV_vb, LVS_EX_FULLROWSELECT);
    This->hMenu_pop_4 = GetSubMenu(MainForm::hm_pop, 4);

    if (!This->hEdit_lphdr || !This->hBtn_vbok || !This->hBtn_sethk || !This->hTV_vb || !This->hLV_vb)
      return -1;

    SendMessage(This->hEdit_lphdr, WM_SETFONT, (WPARAM)MainForm::hf_tam, 1);
    SendMessage(This->hEdit_lphdr, EM_SETLIMITTEXT, 8, 0);
    SetWindowLong(This->hBtn_vbok, GWL_ID, PDIDC_VBOKBTN);
    SendMessage(This->hBtn_vbok, WM_SETFONT, (WPARAM)MainForm::hf_tas, 1);
    SetWindowLong(This->hBtn_sethk, GWL_ID, PDIDC_VBSHBTN);
    SendMessage(This->hBtn_sethk, WM_SETFONT, (WPARAM)MainForm::hf_tas, 1);

    wpVbTV = (WNDPROC)SetWindowLong(This->hTV_vb, GWL_WNDPROC, (LONG)&vbwpTreeView);
    This->ctv_vb.Attach(This->hTV_vb);
    This->ctv_vb.SetFont(&MainForm::cf_tam, TRUE);
    This->ctv_vb.SetBkColor(RGB(225, 250, 250));
    This->ctv_vb.SetImageList(&MainForm::cImageList, TVSIL_NORMAL);

    This->clv_vb.Attach(This->hLV_vb);
    This->clv_vb.SetFont(&MainForm::cf_tam, TRUE);
    This->clv_vb.SetImageList(&MainForm::cImageList, LVSIL_NORMAL);
  }
  return 0;
  case WM_DESTROY:
    if (This->ctv_vb.m_hWnd)
    {
      This->ctv_vb.SetImageList(NULL, TVSIL_NORMAL);
      This->ctv_vb.Detach();
    }
    if (This->clv_vb.m_hWnd)
    {
      This->clv_vb.SetImageList(NULL, LVSIL_NORMAL);
      This->clv_vb.Detach();
    }
    This->m_hWnd = NULL;
    This->hEdit_lphdr = NULL;
    This->hBtn_vbok = NULL;
    This->hTV_vb = NULL;
    This->hLV_vb = NULL;
    return 0;
  case WM_NOTIFY:
  {
    if (lParam == 0) break;

    LPNMHDR pNm = (LPNMHDR)lParam;

    switch (pNm->code)
    {
    case TVN_DELETEITEMA:
    case TVN_DELETEITEMW:
    {
      LPNMTREEVIEWA pnmtv = (LPNMTREEVIEWA)lParam;
      if (pnmtv->itemOld.lParam)
      {
        PVB_TV_TYPE pTv = (PVB_TV_TYPE)pnmtv->itemOld.lParam;

        if ((pTv != NULL) && (pTv->TV.Sign == MY_MAGIC))
        {
          free((PVOID)pTv);
          pnmtv->itemOld.lParam = NULL;
        }
      }
    }
    break;
    case TVN_SELCHANGED:
      if (pNm->hwndFrom == This->hTV_vb)
      {
        LPNMTREEVIEW lpnmtv = (LPNMTREEVIEW)lParam;
        PVB_TV_TYPE lpTv = (PVB_TV_TYPE)lpnmtv->itemNew.lParam;

        if (lpTv == NULL || lpTv->TV.Sign != MY_MAGIC) return 1;

        switch (lpTv->TV.NCode)
        {
        case VBPC_STRUCT:
        {
          switch (lpTv->TV.Index)
          {
          case VBII_VBHDR:
            This->ListHeader();
            break;
          case VBII_PRJI:
            This->ListProjectInfo();
            break;
          case VBII_COMD:
            This->ListCOMRegData();
            break;
          case VBII_COMI:
            This->ListCOMRegInfo();
            break;
          case VBII_COMISUB:
            This->ListCOMRegInfoSub((PVBCRI)lpTv->ExParam);
            break;
          case VBII_OBJTAB:
            This->ListObjectTable();
            break;
          }
        }
        break;
        case VBPC_OBJECT:
          This->ListPublicObjectDescr((PVB_PUB_OBJ_DESCR)lpTv->ExParam);
          break;
        case VBPC_OBJINFO:
          This->ListObjectInfo((PVB_OBJECT_INFO)lpTv->ExParam);
          break;
        case VBPC_OBJOPTINFO:
          This->ListObjectOptionalInfo((PVB_OPTIONAL_OBJECT_INFO)lpTv->ExParam);
          break;
        }

      }
      break;
    case NM_RCLICK:
      if (pNm->hwndFrom == This->hTV_vb)
      {
        PVB_TV_TYPE pTv;
        RECT rect;
        TVHITTESTINFO tvhti;
        TVITEM tvi;

        GetCursorPos(&tvhti.pt);
        GetWindowRect(This->hTV_vb, &rect);
        tvhti.pt.x -= rect.left;
        tvhti.pt.y -= rect.top;
        This->ctv_vb.HitTest(&tvhti);
        if (CHK_EMPTY(tvhti.flags, TVHT_ONITEM)) return 1;
        This->ctv_vb.SelectItem(tvhti.hItem);
        tvi.mask = TVIF_PARAM;
        tvi.hItem = tvhti.hItem;
        This->ctv_vb.GetItem(&tvi);
        pTv = (PVB_TV_TYPE)tvi.lParam;
        if (pTv == NULL || pTv->TV.Sign != MY_MAGIC) return 1;
        tvhti.pt.x += rect.left;
        tvhti.pt.y += rect.top;
        if (pTv->TV.NCode == VBPC_FRMCTL)
          TrackPopupMenu(This->hMenu_pop_4, TPM_LEFTALIGN, tvhti.pt.x, tvhti.pt.y, 0, phWnd, NULL);
      }
      return 1;
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

      if (Width < 800) lprc->right = lprc->left + 800;
      if (Height < 400) lprc->bottom = lprc->top + 400;
    }
  }
  return 1;
  case WM_PAINT:
  {
    PAINTSTRUCT ps;
    HDC hDC = BeginPaint(phWnd, &ps);

    if (hDC == NULL) break;
    SelectObject(hDC, MainForm::hf_tam);
    SetTextColor(hDC, RGB(0, 0, 255));
    TextOut(hDC, 4, 6, L"VB结构地址:", 7);
    EndPaint(phWnd, &ps);
  }
  return 0;
  case WM_SIZE:
    if (wParam != SIZE_MINIMIZED)
    {
      WORD Width = LOWORD(lParam);
      WORD Height = HIWORD(lParam);

      This->m_Width = Width;
      This->m_Height = Height;

      MoveWindow(This->hTV_vb, 0, 28, 280, Height - 28, TRUE);
      MoveWindow(This->hLV_vb, 280, 28, Width - 280, Height - 28, TRUE);
    }
    break;
  }

  return DefMDIChildProc(phWnd, Msg, wParam, lParam);
}

void MC_VBInfo::ResetView()
{
  ctv_vb.DeleteAllItems();
  clv_vb.DeleteAllItems();
  while (clv_vb.DeleteColumn(0));
}

void MC_VBInfo::vbResetColumn(DWORD dwType)
{
  LV_COLUMN lvc;
  lvc.mask = LVCF_TEXT | LVCF_WIDTH;

  clv_vb.DeleteAllItems();
  while (clv_vb.DeleteColumn(0));

  switch (dwType)
  {
  case 1:
    lvc.pszText = L"成员名称";
    lvc.cx = 160;
    clv_vb.InsertColumn(0, &lvc);

    lvc.pszText = L"成员数值";
    lvc.cx = 160;
    clv_vb.InsertColumn(1, &lvc);

    lvc.pszText = L"成员描述";
    lvc.cx = 152;
    clv_vb.InsertColumn(2, &lvc);
    break;
  default:
    lvc.pszText = L"成员名称";
    lvc.cx = 160;
    clv_vb.InsertColumn(0, &lvc);

    lvc.pszText = L"成员数值";
    lvc.cx = 160;
    clv_vb.InsertColumn(1, &lvc);

    lvc.pszText = L"成员描述";
    lvc.cx = 152;
    clv_vb.InsertColumn(2, &lvc);
    break;
  }
}

void MC_VBInfo::ListHeader()
{
  __try
  {
    vbResetColumn(1);

    LV_ITEM lvi;
    WCHAR stmp[MAX_PATH];

    lvi.mask = LVIF_TEXT;
    
    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = L"结构签名";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->Sign.dwSign);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"VB5!";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 1;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 运行时构建版本";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lp_VbHdr->wRuntimeBuild);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"VB运行库构建版本";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 2;
    lvi.iSubItem = 0;
    lvi.pszText = L"文本: 语言动态库";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->szLangDll);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = stmp;
    if (lp_VbHdr->szLangDll[0] == 0x7F)
    {
      lvi.pszText = L"*默认值*";
    }
    else
    {
      lvi.pszText = stmp;
      mbstowcs_s(NULL, stmp, MAX_PATH, lp_VbHdr->szLangDll, 14);
    }
    clv_vb.SetItem(&lvi);


    lvi.iItem = 3;
    lvi.iSubItem = 0;
    lvi.pszText = L"文本: 第二语言动态库";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->szSecLangDll);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    if (lp_VbHdr->szSecLangDll[0] == 0x2A)
    {
      lvi.pszText = L"*默认值*";
    }
    else
    {
      lvi.pszText = stmp;
      mbstowcs_s(NULL, stmp, MAX_PATH, lp_VbHdr->szSecLangDll, 14);
    }
    clv_vb.SetItem(&lvi);


    lvi.iItem = 4;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 运行时修订版本";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%u", lp_VbHdr->wRuntimeRevision);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"内部运行时修订版本";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 5;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 语言ID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%u", lp_VbHdr->dwLCID);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    if (If_LCIDToLocaleName(lp_VbHdr->dwLCID, stmp, MAX_PATH, LOCALE_ALLOW_NEUTRAL_NAMES) == 0)
      lvi.pszText = L"????";
    else
      lvi.pszText = stmp;
    clv_vb.SetItem(&lvi);


    lvi.iItem = 6;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 第二语言ID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%u", lp_VbHdr->dwSecLCID);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    if (If_LCIDToLocaleName(lp_VbHdr->dwSecLCID, stmp, MAX_PATH, LOCALE_ALLOW_NEUTRAL_NAMES) == 0)
      lvi.pszText = L"????";
    else
      lvi.pszText = stmp;
    clv_vb.SetItem(&lvi);


    lvi.iItem = 7;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: Sub Main";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->lpExtComponentTable);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"Sub Main 代码地址";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 8;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: VB工程信息";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->lpProjInfo);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"工程信息地址";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 9;
    lvi.iSubItem = 0;
    lvi.pszText = L"标志: MDL内部对象";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->fMdlIntCtls);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"VB控制标志ID < 32";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 10;
    lvi.iSubItem = 0;
    lvi.pszText = L"标志: MDL内部对象2";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->fMdlIntCtls2);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"VB控制标志ID > 32";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 11;
    lvi.iSubItem = 0;
    lvi.pszText = L"标志: 线程";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->dwThreadFlags);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = stmp;
    PrintThreadFlags(lp_VbHdr->dwThreadFlags, stmp, MAX_PATH);
    clv_vb.SetItem(&lvi);

    lvi.iItem = 12;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: 线程";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%u", lp_VbHdr->dwThreadCount);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"Threads to support in pool";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 13;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 窗体计数";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lp_VbHdr->wFormCount);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"窗体数量";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 14;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 外部组件计数";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lp_VbHdr->wExtComponentCount);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"外部组件数量";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 15;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: Thunk";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%u", lp_VbHdr->dwThunkCount);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"Thunk创建数量";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 16;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: GUI元素表";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->lpGuiTable);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"GUI表基地址";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 17;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 外部组件表";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->lpExtComponentTable);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"外部组件表基地址";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 18;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: COM注册数据";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->lpComRegData);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"COM注册数据地址";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 19;
    lvi.iSubItem = 0;
    lvi.pszText = L"文本: 工程文件名称";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
      lp_VbHdr->oSZProjectDescription, MAKE_UINTPTR(lp_VbHdr, lp_VbHdr->oSZProjectDescription));
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    if (TryCopyWCharFromChar(stmp, MAX_PATH, (PCHAR)(MAKE_UINTPTR(lp_VbHdr, lp_VbHdr->oSZProjectDescription))))
      lvi.pszText = stmp;
    else
      lvi.pszText = L"????";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 20;
    lvi.iSubItem = 0;
    lvi.pszText = L"文本: 工程标题";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
      lp_VbHdr->oSZProjectExeName, MAKE_UINTPTR(lp_VbHdr, lp_VbHdr->oSZProjectExeName));
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    if (TryCopyWCharFromChar(stmp, MAX_PATH, (PCHAR)(MAKE_UINTPTR(lp_VbHdr, lp_VbHdr->oSZProjectExeName))))
      lvi.pszText = stmp;
    else
      lvi.pszText = L"????";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 21;
    lvi.iSubItem = 0;
    lvi.pszText = L"文本: 帮助文件";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
      lp_VbHdr->oSZProjectHelpFile, MAKE_UINTPTR(lp_VbHdr, lp_VbHdr->oSZProjectHelpFile));
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    if (TryCopyWCharFromChar(stmp, MAX_PATH, (PCHAR)(MAKE_UINTPTR(lp_VbHdr, lp_VbHdr->oSZProjectHelpFile))))
      lvi.pszText = stmp;
    else
      lvi.pszText = L"????";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 22;
    lvi.iSubItem = 0;
    lvi.pszText = L"文本: 工程名称";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
      lp_VbHdr->oSZProjectName, MAKE_UINTPTR(lp_VbHdr, lp_VbHdr->oSZProjectName));
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    if (TryCopyWCharFromChar(stmp, MAX_PATH, (PCHAR)(MAKE_UINTPTR(lp_VbHdr, lp_VbHdr->oSZProjectName))))
      lvi.pszText = stmp;
    else
      lvi.pszText = L"????";
    clv_vb.SetItem(&lvi);
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    mf->SetStateText(L"在解析VB头部结构时读取内存错误 [1].", RGB(255, 0, 0));
    return;
  }
}

void MC_VBInfo::ListProjectInfo()
{
#define PRIV_BUFLEN     600
  __try
  {
    vbResetColumn(1);

    LV_ITEM lvi;
    WCHAR stmp[PRIV_BUFLEN];

    lvi.mask = LVIF_TEXT;

    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = L"结构签名";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lp_VbHdr->lpProjInfo->Sign.dwSign);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 1;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 对象表";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lp_VbHdr->lpProjInfo->lpObjectTable);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"对象表地址";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 2;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 代码起始位置";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lp_VbHdr->lpProjInfo->lpCodeStart);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指向代码起始的位置,未使用";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 3;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 代码结束位置";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lp_VbHdr->lpProjInfo->lpCodeEnd);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指向代码结束的位置,未使用";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 4;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 数据大小";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lp_VbHdr->lpProjInfo->dwDataSize);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"VB对象结构大小,未使用";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 5;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 线程空间";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lp_VbHdr->lpProjInfo->lpThreadSpace);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向线程对象";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 6;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: VBA异常处理器";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lp_VbHdr->lpProjInfo->lpVBAExceptHandler);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向VBA异常处理";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 7;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 本地代码";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lp_VbHdr->lpProjInfo->lpNativeCode);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向.data段";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 8;
    lvi.iSubItem = 0;
    lvi.pszText = L"文本: 原始路径名称";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    wcsncpy_s(stmp, MAX_PATH, lp_VbHdr->lpProjInfo->szPathInformation, 264);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"包含路径和ID的字符串 < SP6";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 9;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 引用表";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lp_VbHdr->lpProjInfo->lpExternalTable);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向引用表";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 10;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: 引用表";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"%u", lp_VbHdr->lpProjInfo->dwExternalCount);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"引用表数量";
    clv_vb.SetItem(&lvi);
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    mf->SetStateText(L"在解析VB工程信息结构时读取内存错误.", RGB(255, 0, 0));
    return;
  }
#undef PRIV_BUFLEN 
}

void MC_VBInfo::ListCOMRegData()
{
  __try
  {
    vbResetColumn(1);

    LV_ITEM lvi;
    WCHAR stmp[MAX_PATH];

    lvi.mask = LVIF_TEXT;

    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = L"偏移: 注册信息";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    if (lp_VbHdr->lpComRegData->oRegInfo == 0)
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X", lp_VbHdr->lpComRegData->oRegInfo);
    else
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X", 
        lp_VbHdr->lpComRegData->oRegInfo, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lp_VbHdr->lpComRegData->oRegInfo));
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"COM接口信息偏移";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 1;
    lvi.iSubItem = 0;
    lvi.pszText = L"文本: 工程名称";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    if (lp_VbHdr->lpComRegData->oSZProjectName == 0)
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X", lp_VbHdr->lpComRegData->oSZProjectName);
      clv_vb.SetItem(&lvi);
    }
    else
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
        lp_VbHdr->lpComRegData->oSZProjectName, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lp_VbHdr->lpComRegData->oSZProjectName));
      clv_vb.SetItem(&lvi);

      lvi.iSubItem = 2;
      if (TryCopyWCharFromChar(stmp, MAX_PATH, (PCHAR)MAKE_UINTPTR(lp_VbHdr->lpComRegData, lp_VbHdr->lpComRegData->oSZProjectName)))
        lvi.pszText = stmp;
      else
        lvi.pszText = L"????";
      clv_vb.SetItem(&lvi);
    }

    lvi.iSubItem = 2;
    lvi.pszText = L"工程/类型库 名称偏移";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 2;
    lvi.iSubItem = 0;
    lvi.pszText = L"文本: 帮助文件目录";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    if (lp_VbHdr->lpComRegData->oSZHelpDirectory == 0)
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X", lp_VbHdr->lpComRegData->oSZHelpDirectory);
      clv_vb.SetItem(&lvi);
    }
    else
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
        lp_VbHdr->lpComRegData->oSZHelpDirectory, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lp_VbHdr->lpComRegData->oSZHelpDirectory));
      clv_vb.SetItem(&lvi);

      lvi.iSubItem = 2;
      if (TryCopyWCharFromChar(stmp, MAX_PATH, (PCHAR)MAKE_UINTPTR(lp_VbHdr->lpComRegData, lp_VbHdr->lpComRegData->oSZHelpDirectory)))
        lvi.pszText = stmp;
      else
        lvi.pszText = L"????";
      clv_vb.SetItem(&lvi);
    }

    lvi.iSubItem = 2;
    lvi.pszText = L"帮助目录偏移";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 3;
    lvi.iSubItem = 0;
    lvi.pszText = L"文本: 工程描述";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    if (lp_VbHdr->lpComRegData->oSZProjectDescription == 0)
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X", lp_VbHdr->lpComRegData->oSZProjectDescription);
      clv_vb.SetItem(&lvi);
    }
    else
    { 
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
        lp_VbHdr->lpComRegData->oSZProjectDescription, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lp_VbHdr->lpComRegData->oSZProjectDescription));
      clv_vb.SetItem(&lvi);

      lvi.iSubItem = 2;
      if (TryCopyWCharFromChar(stmp, MAX_PATH, (PCHAR)MAKE_UINTPTR(lp_VbHdr->lpComRegData, lp_VbHdr->lpComRegData->oSZProjectDescription)))
        lvi.pszText = stmp;
      else
        lvi.pszText = L"????";
      clv_vb.SetItem(&lvi);
    }

    lvi.iSubItem = 2;
    lvi.pszText = L"工程描述偏移";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 4;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 工程CLSID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    MFUNC_PRINTGUIDW(stmp, MAX_PATH, lp_VbHdr->lpComRegData->uuidProjectClsId);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"工程/类型库 CLSID";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 5;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: TypeLib语言ID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%u", lp_VbHdr->lpComRegData->dwTlbLcid);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    if (If_LCIDToLocaleName(lp_VbHdr->lpComRegData->dwTlbLcid, stmp, MAX_PATH, LOCALE_ALLOW_NEUTRAL_NAMES) == 0)
      lvi.pszText = L"????";
    else
      lvi.pszText = stmp;
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"类型库语言ID";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 6;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 未知";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%04hX", lp_VbHdr->lpComRegData->wUnknown);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"可能有用,必须检查";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 7;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: TypeLib主版本号";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lp_VbHdr->lpComRegData->wTlbVerMajor);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"类型库主版本号";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 8;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: TypeLib次版本号";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lp_VbHdr->lpComRegData->wTlbVerMinor);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"类型库次版本号";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 9;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 未知2";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%04hX", lp_VbHdr->lpComRegData->wUnknown2);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 10;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 未知3";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lp_VbHdr->lpComRegData->dwUnknown);
    clv_vb.SetItem(&lvi);

  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    mf->SetStateText(L"在解析VB COM注册数据结构时读取内存错误.", RGB(255, 0, 0));
    return;
  }
}

void MC_VBInfo::ListCOMRegInfo()
{
  __try
  {
    vbResetColumn(1);

    PVBCRI lpvbcri = (PVBCRI)MAKE_UINTPTR(lp_VbHdr->lpComRegData, lp_VbHdr->lpComRegData->oRegInfo);
    LV_ITEM lvi;
    WCHAR stmp[MAX_PATH];

    lvi.mask = LVIF_TEXT;

    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = L"偏移: 注册信息链表头";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
      lp_VbHdr->lpComRegData->oRegInfo, lpvbcri);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"偏移相对于'COM注册数据'基址,指向'COM注册信息'链表头";
    clv_vb.SetItem(&lvi);
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    mf->SetStateText(L"在解析VB COM注册信息结构时读取内存错误.", RGB(255, 0, 0));
    return;
  }
}

void MC_VBInfo::ListCOMRegInfoSub(PVB_COM_REG_INFO lpComRegInfo)
{
  if (lpComRegInfo == NULL) return;

  __try
  {
    vbResetColumn(1);

    PVBCRI lpvbcri = (PVBCRI)MAKE_UINTPTR(lp_VbHdr->lpComRegData, lp_VbHdr->lpComRegData->oRegInfo);
    LV_ITEM lvi;
    WCHAR stmp[MAX_PATH];

    lvi.mask = LVIF_TEXT;


    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = L"偏移: 下个注册信息";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    if (lpComRegInfo->oNextObject != 0)
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
        lpComRegInfo->oNextObject, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lpComRegInfo->oNextObject));
    else
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X", lpComRegInfo->oNextObject);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"偏移相对于'COM注册数据'基址,指向下一个COM接口信息";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 1;
    lvi.iSubItem = 0;
    lvi.pszText = L"偏移: 对象名称";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    if (lpComRegInfo->oObjectName == 0)
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X", lpComRegInfo->oObjectName);
      clv_vb.SetItem(&lvi);
    }
    else
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X,  VA: 0x%08X",
        lpComRegInfo->oObjectName, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lpComRegInfo->oObjectName));
      clv_vb.SetItem(&lvi);

      lvi.iSubItem = 2;
      if (TryCopyWCharFromChar(stmp, MAX_PATH, (PCHAR)MAKE_UINTPTR(lp_VbHdr->lpComRegData, lpComRegInfo->oObjectName)))
        lvi.pszText = stmp;
      else
        lvi.pszText = L"????";
      clv_vb.SetItem(&lvi);
    }
    

    lvi.iItem = 2;
    lvi.iSubItem = 0;
    lvi.pszText = L"偏移: 对象描述";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    if (lpComRegInfo->oObjectDescription == 0)
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X", lpComRegInfo->oObjectDescription);
      clv_vb.SetItem(&lvi);
    }
    else
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
        lpComRegInfo->oObjectDescription, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lpComRegInfo->oObjectDescription));
      clv_vb.SetItem(&lvi);

      lvi.iSubItem = 2;
      if (TryCopyWCharFromChar(stmp, MAX_PATH, (PCHAR)MAKE_UINTPTR(lp_VbHdr->lpComRegData, lpComRegInfo->oObjectDescription)))
        lvi.pszText = stmp;
      else
        lvi.pszText = L"????";
      clv_vb.SetItem(&lvi);
    }


    lvi.iItem = 3;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 实例化模式";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpComRegInfo->dwInstancing);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 4;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 对象ID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%u", lpComRegInfo->dwObjectId);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"当前对象在工程中的ID";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 5;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 对象CLSID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    MFUNC_PRINTGUIDW(stmp, MAX_PATH, lpComRegInfo->uuidObject);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 6;
    lvi.iSubItem = 0;
    lvi.pszText = L"标志: 接口";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpComRegInfo->fIsInterface);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"如果下面指定的CLSID是有效的";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 7;
    lvi.iSubItem = 0;
    lvi.pszText = L"偏移: 对象接口CLSID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    if (lpComRegInfo->bUuidObjectIFace == 0)
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X", lpComRegInfo->bUuidObjectIFace);
      clv_vb.SetItem(&lvi);

      lvi.iSubItem = 2;
      lvi.pszText = L"对象接口CLSID的偏移";
      clv_vb.SetItem(&lvi);
    }
    else
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
        lpComRegInfo->bUuidObjectIFace, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lpComRegInfo->bUuidObjectIFace));
      clv_vb.SetItem(&lvi);

      lvi.iSubItem = 2;
      MFUNC_PRINTLPGUIDW(stmp, MAX_PATH, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lpComRegInfo->bUuidObjectIFace));
      clv_vb.SetItem(&lvi);
    }


    lvi.iItem = 8;
    lvi.iSubItem = 0;
    lvi.pszText = L"偏移: 控件接口CLSID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    if (lpComRegInfo->bUuidEventsIFace == 0)
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X", lpComRegInfo->bUuidEventsIFace);
      clv_vb.SetItem(&lvi);

      lvi.iSubItem = 2;
      lvi.pszText = L"控件接口CLSID的偏移";
      clv_vb.SetItem(&lvi);
    }
    else
    {
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
        lpComRegInfo->bUuidEventsIFace, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lpComRegInfo->bUuidEventsIFace));
      clv_vb.SetItem(&lvi);

      lvi.iSubItem = 2;
      MFUNC_PRINTLPGUIDW(stmp, MAX_PATH, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lpComRegInfo->bUuidEventsIFace));
      clv_vb.SetItem(&lvi);
    }


    lvi.iItem = 9;
    lvi.iSubItem = 0;
    lvi.pszText = L"标志: 控件事件";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpComRegInfo->fHasEvents);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"如果上面指定的CLSID是有效的";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 10;
    lvi.iSubItem = 0;
    lvi.pszText = L"标志: OLEMISC状态";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpComRegInfo->dwMiscStatus);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = stmp;
    PrintOLEMISC(lpComRegInfo->dwMiscStatus, stmp, MAX_PATH);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 11;
    lvi.iSubItem = 0;
    lvi.pszText = L"标志: Class类型";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%02X", lpComRegInfo->fClassType);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 12;
    lvi.iSubItem = 0;
    lvi.pszText = L"标志: 对象类型";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%02X", lpComRegInfo->fObjectType);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    switch (lpComRegInfo->fObjectType)
    {
    case 0x02:
      lvi.pszText = L"设计器";
      break;
    case 0x10:
      lvi.pszText = L"类模块";
      break;
    case 0x20:
      lvi.pszText = L"用户控件";
      break;
    case 0x80:
      lvi.pszText = L"用户文档";
      break;
    default:
      lvi.pszText = L"";
      break;
    }
    clv_vb.SetItem(&lvi);


    lvi.iItem = 13;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 控件工具栏位图ID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lpComRegInfo->wToolboxBitmap32);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 14;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 控件窗口默认的最小化图标";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lpComRegInfo->wDefaultIcon);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 15;
    lvi.iSubItem = 0;
    lvi.pszText = L"标志: 设计器";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%04hX", lpComRegInfo->fIsDesigner);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指定是否是一个'设计器'";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 16;
    lvi.iSubItem = 0;
    lvi.pszText = L"偏移: 设计器数据";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    if (lpComRegInfo->oDesignerData == 0)
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X", lpComRegInfo->oDesignerData);
    else
      swprintf_s(stmp, MAX_PATH, L"相对偏移: 0x%02X, VA: 0x%08X",
        lpComRegInfo->oDesignerData, MAKE_UINTPTR(lp_VbHdr->lpComRegData, lpComRegInfo->oDesignerData));
    clv_vb.SetItem(&lvi);
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    mf->SetStateText(L"在解析VB COM注册信息结构链表时读取内存错误.", RGB(255, 0, 0));
    return;
  }
}

void MC_VBInfo::ListObjectTable()
{
  __try
  {
    vbResetColumn(1);

    if (lp_VbHdr->lpProjInfo->lpObjectTable == NULL) return;

    PVBOT lpvbot = lp_VbHdr->lpProjInfo->lpObjectTable;
    LV_ITEM lvi;
    WCHAR stmp[MAX_PATH];

    lvi.mask = LVIF_TEXT;

    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: ExecProj";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpvbot->lpExecProj);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向VB工程Exec COM对象";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 1;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 工程信息2";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpvbot->lpProjectInfo2);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"第二工程信息";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 2;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 保留";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpvbot->dwReserved);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"编译后总为-1,未使用";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 3;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 工程对象";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpvbot->lpProjectObject);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向内存中的工程数据";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 4;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 对象表GUID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    MFUNC_PRINTGUIDW(stmp, MAX_PATH, lpvbot->uuidObject);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 5;
    lvi.iSubItem = 0;
    lvi.pszText = L"标志: 编译类型";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%04hX", lpvbot->fCompileState);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"在编译过程中使用的内部标志";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 6;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: 对象总数";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lpvbot->dwTotalObjects);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 7;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: 已编译对象总数";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lpvbot->dwCompiledObjects);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 8;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: 使用中的对象";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lpvbot->dwObjectsInUse);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"更新在IDE对应的总数,但在初始化或卸载模块时,该值会上升或下降";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 9;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 公有对象描述符";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpvbot->lpObjectArray);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向公有对象描述符数组";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 10;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 工程名称";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpvbot->lpSZProjectName);
    clv_vb.SetItem(&lvi);


    lvi.iSubItem = 2;
    if (TryCopyWCharFromChar(stmp, MAX_PATH, lpvbot->lpSZProjectName))
      lvi.pszText = stmp;
    else
      lvi.pszText = L"????";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 11;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 备用语言ID1";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%u", lpvbot->dwLcid);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    if (If_LCIDToLocaleName(lpvbot->dwLcid, stmp, MAX_PATH, LOCALE_ALLOW_NEUTRAL_NAMES) == 0)
      lvi.pszText = L"????";
    else
      lvi.pszText = stmp;
    clv_vb.SetItem(&lvi);


    lvi.iItem = 12;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 备用语言ID2";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%u", lpvbot->dwLcid2);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    if (If_LCIDToLocaleName(lpvbot->dwLcid2, stmp, MAX_PATH, LOCALE_ALLOW_NEUTRAL_NAMES) == 0)
      lvi.pszText = L"????";
    else
      lvi.pszText = stmp;
    clv_vb.SetItem(&lvi);


    lvi.iItem = 13;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 结构模板版本";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpvbot->dwIdentifier);
    clv_vb.SetItem(&lvi);
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    mf->SetStateText(L"在解析VB对象表时读取内存错误.", RGB(255, 0, 0));
    return;
  }
}

void MC_VBInfo::ListPublicObjectDescr(PVB_PUB_OBJ_DESCR lpVbObject)
{
  __try
  {
    vbResetColumn(1);

    if (lpVbObject == NULL) return;

    LV_ITEM lvi;
    WCHAR stmp[MAX_PATH];

    lvi.mask = LVIF_TEXT;

    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 对象信息";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObject->lpObjectInfo);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向此对象的对象信息";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 1;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: Public变量表";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObject->lpPublicBytes);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向Public变量大小的整数";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 2;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: Static变量表";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObject->lpStaticBytes);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向Static变量大小的整数";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 3;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 模块Public变量表";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObject->lpModulePublic);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向Public变量在DATA区段";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 4;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 模块Static变量表";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObject->lpModuleStatic);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向Static变量在DATA区段";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 5;
    lvi.iSubItem = 0;
    lvi.pszText = L"文本: 对象名称";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObject->lpSZObjectName);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    if (TryCopyWCharFromChar(stmp, MAX_PATH, lpVbObject->lpSZObjectName))
      lvi.pszText = stmp;
    else
      lvi.pszText = L"????";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 6;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: 方法计数";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%u", lpVbObject->dwMethodCount);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"该对象中方法的数量 (Event / Function / Sub 的总数)";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 7;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 过程名称数组";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObject->lpMethodNamesArray);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"如果存在,指针指向方法名称的数组";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 8;
    lvi.iSubItem = 0;
    lvi.pszText = L"偏移: 模块Static变量表";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObject->oStaticVars);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"模块Static变量表的偏移 (Offset to where to copy Static Variables)";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 9;
    lvi.iSubItem = 0;
    lvi.pszText = L"标志: 对象类型";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObject->fObjectType);
    clv_vb.SetItem(&lvi);
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    mf->SetStateText(L"在解析VB对象时读取内存错误.", RGB(255, 0, 0));
    return;
  }
}

void MC_VBInfo::ListObjectInfo(PVB_OBJECT_INFO lpVbObjInfo)
{
  __try
  {
    vbResetColumn(1);

    if (lpVbObjInfo == NULL) return;

    LV_ITEM lvi;
    WCHAR stmp[MAX_PATH];

    lvi.mask = LVIF_TEXT;

    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: Ref数量";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%04hX", lpVbObjInfo->wRefCount);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"编译后总是为1";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 1;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 对象索引";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lpVbObjInfo->wObjectIndex);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"该对象在对象数组中的索引";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 2;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 对象表";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObjInfo->lpObjectTable);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向对象表";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 3;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 私有对象描述符";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObjInfo->lpPrivateObject);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向私有对象描述符;当对象是模块时,该值为0xFFFFFFFF (-1)";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 4;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 公有对象描述符";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObjInfo->lpPublicObject);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"反向指针指向公共对象描述符";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 5;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 工程数据";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObjInfo->lpProjectData);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向内存中的工程对象";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 6;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: 方法数量";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lpVbObjInfo->wMethodCount);
    clv_vb.SetItem(&lvi);


    lvi.iItem = 7;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: 方法数量2";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lpVbObjInfo->wMethodCount2);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"仅IDE模式有效,编译后置0";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 8;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 方法";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObjInfo->lpMethods);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"方法指针数组";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 9;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: 常量数量";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lpVbObjInfo->wConstants);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"在常量池中的常量数量";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 10;
    lvi.iSubItem = 0;
    lvi.pszText = L"数值: 最大常量分配";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"%hu", lpVbObjInfo->wMaxConstants);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"常量在常量池中的分配";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 11;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 常量池";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, MAX_PATH, L"0x%08X", lpVbObjInfo->lpConstants);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"指针指向常量池;如果'常量池指针'指向'常量池指针'之后的地址,那么不存在可选信息";
    clv_vb.SetItem(&lvi);
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    mf->SetStateText(L"在解析VB对象信息时读取内存错误.", RGB(255, 0, 0));
    return;
  }
}

void MC_VBInfo::ListObjectOptionalInfo(PVB_OPTIONAL_OBJECT_INFO lpVbObjOptInfo)
{
#define PRIV_BUFLEN   1024
  __try
  {
    vbResetColumn(1);

    if (lpVbObjOptInfo == NULL) return;

    LV_ITEM lvi;
    WCHAR stmp[PRIV_BUFLEN];

    lvi.mask = LVIF_TEXT;

    lvi.iItem = 0;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: 注册对象GUID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"%hu", lpVbObjOptInfo->wObjectGuids);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"有多少GUID要注册;如果该值为2,表示是一个'设计器'";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 1;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 对象GUID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lpVbObjOptInfo->lpObjectGuid);
    clv_vb.SetItem(&lvi);

    if (lpVbObjOptInfo->lpObjectGuid != NULL)
    {
      lvi.iSubItem = 2;
      lvi.pszText = stmp;
      MFUNC_PRINTLPGUIDW(stmp, PRIV_BUFLEN, lpVbObjOptInfo->lpObjectGuid);
      clv_vb.SetItem(&lvi);
    }


    lvi.iItem = 2;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 对象类型GUID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lpVbObjOptInfo->lpUuidObjectGUI);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = stmp;
    wcscpy_s(stmp, PRIV_BUFLEN, L"指针指向对象接口的GUID数组: ");
    if (lpVbObjOptInfo->lpObjectGuid != NULL)
    {
      uint32_t idx, dwmax = lpVbObjOptInfo->wObjectControls;
      WCHAR stmp1[MAX_PATH];
      
      for (idx = 0; idx < dwmax; idx++)
      {
        MFUNC_PRINTLPGUIDW(stmp1, MAX_PATH, lpVbObjOptInfo->lpUuidObjectGUI[idx]);
        wcscpy_s(stmp, PRIV_BUFLEN, stmp1);

        if (idx + 1 < dwmax)
          wcscat_s(stmp, PRIV_BUFLEN, L", ");
      }
    }
    clv_vb.SetItem(&lvi);


    lvi.iItem = 3;
    lvi.iSubItem = 0;
    lvi.pszText = L"计数: 对象类型GUID";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"%hu", lpVbObjOptInfo->wObjectControls);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"上面的GUID数组的数量";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 4;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 对象控件2";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lpVbObjOptInfo->lpControls2);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"通常和'对象控件'一样";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 5;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 对象控件2";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lpVbObjOptInfo->lpControls2);
    clv_vb.SetItem(&lvi);

    lvi.iSubItem = 2;
    lvi.pszText = L"通常和'对象控件'一样";
    clv_vb.SetItem(&lvi);


    lvi.iItem = 6;
    lvi.iSubItem = 0;
    lvi.pszText = L"地址: 对象GUID2";
    clv_vb.InsertItem(&lvi);

    lvi.iSubItem = 1;
    lvi.pszText = stmp;
    swprintf_s(stmp, PRIV_BUFLEN, L"0x%08X", lpVbObjOptInfo->lpObjectGuid2);
    clv_vb.SetItem(&lvi);

    if (lpVbObjOptInfo->lpObjectGuid2 != NULL)
    {
      lvi.iSubItem = 2;
      lvi.pszText = stmp;
      MFUNC_PRINTLPGUIDW(stmp, PRIV_BUFLEN, lpVbObjOptInfo->lpObjectGuid2);
      clv_vb.SetItem(&lvi);
    }
  }
  __except (TEXPECT_NOACCESS(GetExceptionCode()))
  {
    mf->SetStateText(L"在解析VB对象可选信息时读取内存错误.", RGB(255, 0, 0));
    return;
  }
#undef PRIV_BUFLEN
}

void MC_VBInfo::RefreshStructList()
{
  //LV_ITEM lvi;

}