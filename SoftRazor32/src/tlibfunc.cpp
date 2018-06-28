#include "../sr_pch.h"

void fdswap(FUNCDESC **fda, FUNCDESC **fdb)
{
  FUNCDESC *fdtmp = *fda;

  *fda = *fdb;
  *fdb = fdtmp;
}

void fdsort(FUNCDESC **pfd, UINT mn)
{
  UINT i;
  UINT j;

  for (i = 0; i < mn; i++)
    for (j = 1; j < mn - i; j++)
      if (pfd[j - 1] && pfd[j])
        if (pfd[j - 1]->oVft > pfd[j]->oVft)
          fdswap(&pfd[j - 1], &pfd[j]);
}

BOOL GetMemberNameByIndex(GUID ClsID, DWORD Idx, PWCHAR pText, DWORD tCount, PDWORD pMemberID)
{
  ITypeLib *pITL = NULL;
  ITypeInfo *pITI = NULL;
  TYPEATTR *pITA = NULL;
  FUNCDESC **pFD = NULL;
  HREFTYPE hRT = NULL;
  BSTR bStr = NULL;
  UINT i;
  UINT OS;
  HRESULT hr;

  if (!tCount) return FALSE;
  hr = QueryPathOfRegTypeLib(vbclsid, 6, 0, 9, &bStr);
  hr = LoadTypeLib(bStr, &pITL);
  SysFreeString(bStr);
  if (hr != S_OK)
    if (LoadRegTypeLib(vbclsid, 6, 0, 9, &pITL) != S_OK) return FALSE;

  hr = pITL->GetTypeInfoOfGuid(ClsID, &pITI);
  if (hr != S_OK) goto RetFalse;
  hr = pITI->GetTypeAttr(&pITA);
  if (hr != S_OK) goto RetFalse;
  if (pITA->typekind != TKIND_INTERFACE) goto RetFalse;
  if (pITA->cFuncs == 0) goto RetFalse;

  /**/
  pFD = (FUNCDESC **)calloc(pITA->cFuncs, sizeof(FUNCDESC *));
  if (!pFD) goto RetFalse;
  if (Idx >= pITA->cFuncs) goto RetFalse;

  for (i = 0; i < pITA->cFuncs; i++)
    hr = pITI->GetFuncDesc(i, &pFD[i]);

  fdsort(pFD, pITA->cFuncs);

  /**/
  if (pMemberID) *pMemberID = pFD[Idx]->memid;
  hr = pITI->GetNames(pFD[Idx]->memid, &bStr, 1, &OS);
  if (hr != S_OK) goto RetFalse;

  wcscpy_s(pText, tCount, bStr);
  SysFreeString(bStr);

  for (i = 0; i < pITA->cFuncs; i++)
    if (pFD[i]) pITI->ReleaseFuncDesc(pFD[i]);
  free(pFD);
  pITI->ReleaseTypeAttr(pITA);
  pITI->Release();
  pITL->Release();
  return TRUE;

RetFalse:
  if (pITI && pITA && pFD)
  {
    for (i = 0; i < pITA->cFuncs; i++)
      if (pFD[i]) pITI->ReleaseFuncDesc(pFD[i]);
    free(pFD);
  }

  if (pITI && pITA)
    pITI->ReleaseTypeAttr(pITA);

  if (pITI)
    pITI->Release();

  if (pITL)
    pITL->Release();

  return FALSE;
}

BOOL GetEventNameByIndex(GUID ClsID, DWORD Idx, PWCHAR pText, DWORD tCount, PDWORD pMemberID)
{
  ITypeLib *pITL = NULL;
  ITypeInfo *pITI = NULL;
  ITypeInfo *pETI = NULL;
  TYPEATTR *pITA = NULL;
  TYPEATTR *pETA = NULL;
  FUNCDESC *pFD = NULL;
  HREFTYPE hRT = NULL;
  BSTR bStr = NULL;
  UINT OS;
  HRESULT hr;

  if (!tCount) return FALSE;
  hr = QueryPathOfRegTypeLib(vbclsid, 6, 0, 9, &bStr);
  hr = LoadTypeLib(bStr, &pITL);
  SysFreeString(bStr);
  if (hr != S_OK)
    if (LoadRegTypeLib(vbclsid, 6, 0, 9, &pITL) != S_OK) return FALSE;

  hr = pITL->GetTypeInfoOfGuid(ClsID, &pITI);
  if (hr != S_OK)
  {
    pITL->Release();
    return FALSE;
  }
  hr = pITI->GetTypeAttr(&pITA);
  if (hr != S_OK)
  {
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  if (pITA->typekind != TKIND_COCLASS || pITA->cImplTypes != 2)
  {
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  hr = pITI->GetRefTypeOfImplType(1, &hRT);
  if (hr != S_OK)
  {
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  hr = pITI->GetRefTypeInfo(hRT, &pETI);
  if (hr != S_OK)
  {
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  hr = pETI->GetTypeAttr(&pETA);
  if (hr != S_OK)
  {
    pETI->Release();
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  if (pETA->typekind != TKIND_INTERFACE)
  {
    pETI->ReleaseTypeAttr(pETA);
    pETI->Release();
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  hr = pETI->GetFuncDesc(Idx, &pFD);
  if (hr != S_OK)
  {
    pETI->ReleaseTypeAttr(pETA);
    pETI->Release();
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }

  if (pMemberID) *pMemberID = pFD->memid;
  hr = pETI->GetNames(pFD->memid, &bStr, 1, &OS);
  if (hr != S_OK)
  {
    pETI->ReleaseFuncDesc(pFD);
    pETI->ReleaseTypeAttr(pETA);
    pETI->Release();
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }

  wcscpy_s(pText, tCount, bStr);
  SysFreeString(bStr);
  pETI->ReleaseFuncDesc(pFD);
  pETI->ReleaseTypeAttr(pETA);
  pETI->Release();
  pITI->ReleaseTypeAttr(pITA);
  pITI->Release();
  pITL->Release();
  return TRUE;
}

BOOL GetEventNameByMemberID(GUID ClsID, DWORD MemberID, PWCHAR pText, DWORD tCount)
{
  ITypeLib *pITL = NULL;
  ITypeInfo *pITI = NULL;
  ITypeInfo *pETI = NULL;
  TYPEATTR *pITA = NULL;
  TYPEATTR *pETA = NULL;
  HREFTYPE hRT = NULL;
  BSTR bStr = NULL;
  HRESULT hr;
  UINT OS;

  if (!pText || !tCount) return FALSE;
  hr = QueryPathOfRegTypeLib(vbclsid, 6, 0, 9, &bStr);
  hr = LoadTypeLib(bStr, &pITL);
  SysFreeString(bStr);
  if (hr != S_OK)
    if (LoadRegTypeLib(vbclsid, 6, 0, 9, &pITL) != S_OK) return FALSE;

  hr = pITL->GetTypeInfoOfGuid(ClsID, &pITI);
  if (hr != S_OK)
  {
    pITL->Release();
    return FALSE;
  }
  hr = pITI->GetTypeAttr(&pITA);
  if (hr != S_OK)
  {
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  if (pITA->typekind != TKIND_COCLASS || pITA->cImplTypes != 2)
  {
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  hr = pITI->GetRefTypeOfImplType(1, &hRT);
  if (hr != S_OK)
  {
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  hr = pITI->GetRefTypeInfo(hRT, &pETI);
  if (hr != S_OK)
  {
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  hr = pETI->GetTypeAttr(&pETA);
  if (hr != S_OK)
  {
    pETI->Release();
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  if (pETA->typekind != TKIND_INTERFACE)
  {
    pETI->ReleaseTypeAttr(pETA);
    pETI->Release();
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  pETI->ReleaseTypeAttr(pETA);
  hr = pETI->GetNames(MemberID, &bStr, 1, &OS);
  if (hr != S_OK)
  {
    pETI->Release();
    pITI->ReleaseTypeAttr(pITA);
    pITI->Release();
    pITL->Release();
    return FALSE;
  }
  wcscpy_s(pText, tCount, bStr);
  SysFreeString(bStr);
  pETI->Release();
  pITI->ReleaseTypeAttr(pITA);
  pITI->Release();
  pITL->Release();
  return TRUE;
}

BOOL GetPropertyText(GUID guid, WORD VTOffset, PWCHAR szNote, DWORD slNote)
{
  ITypeLib * ITLib = NULL;
  ITypeInfo * ITInfo = NULL;
  TYPEATTR * PTA = NULL;
  FUNCDESC * PFD = NULL;
  BSTR bStr = NULL;
  UINT i;
  UINT OS;
  HRESULT hr;

  if (!szNote || !slNote) return FALSE;

  hr = QueryPathOfRegTypeLib(vbclsid, 6, 0, 9, &bStr);
  hr = LoadTypeLib(bStr, &ITLib);
  SysFreeString(bStr);
  if (hr != S_OK) if (LoadRegTypeLib(vbclsid, 6, 0, 9, &ITLib) != S_OK) return FALSE;

  if (ITLib->GetTypeInfoOfGuid(guid, &ITInfo) != S_OK)
  {
    ITLib->Release();
    return FALSE;
  }

  if (ITInfo->GetTypeAttr(&PTA) != S_OK)
  {
    ITInfo->Release();
    ITLib->Release();
    return FALSE;
  }

  if (PTA->typekind != TKIND_INTERFACE)
  {
    ITInfo->ReleaseTypeAttr(PTA);
    ITInfo->Release();
    ITLib->Release();
    return FALSE;
  }

  ITInfo->ReleaseTypeAttr(PTA);

  if (ITInfo->GetDocumentation(MEMBERID_NIL, &bStr, NULL, NULL, NULL) != S_OK)
    wcscpy_s(szNote, slNote, L"_????.");
  else
  {
    swprintf_s(szNote, slNote, L"%s.", bStr);
    SysFreeString(bStr);
    bStr = NULL;
  }

  for (i = 0; i < PTA->cFuncs;i++)
  {
    if (ITInfo->GetFuncDesc(i, &PFD) != S_OK) continue;

    if (PFD->oVft == VTOffset)
    {
      if (ITInfo->GetNames(PFD->memid, &bStr, 1, &OS) != S_OK)
      {
        ITInfo->ReleaseFuncDesc(PFD);
        ITInfo->Release();
        ITLib->Release();
        return FALSE;
      }

      wcscat_s(szNote, slNote, bStr);
      SysFreeString(bStr);
      
      wcscat_s(szNote, slNote, L" [Property=");
      switch (PFD->invkind)
      {
      case INVOKE_FUNC:
        wcscat_s(szNote, slNote, L"Func");
        break;
      case INVOKE_PROPERTYGET:
        wcscat_s(szNote, slNote, L"Get");
        break;
      case INVOKE_PROPERTYPUT:
        wcscat_s(szNote, slNote, L"Let");
        break;
      case INVOKE_PROPERTYPUTREF:
        wcscat_s(szNote, slNote, L"Set");
        break;
      default:
        wcscat_s(szNote, slNote, L"*Unkn");
        break;
      }

      wcscat_s(szNote, slNote, L"], [RetType=");

      switch (PFD->elemdescFunc.tdesc.vt)
      {
      case VT_I2:
        wcscat_s(szNote, slNote, L"Integer");
        break;
      case VT_I4:
        wcscat_s(szNote, slNote, L"Long");
        break;
      case VT_R4:
        wcscat_s(szNote, slNote, L"Single");
        break;
      case VT_R8:
        wcscat_s(szNote, slNote, L"Double");
        break;
      case VT_CY:
        wcscat_s(szNote, slNote, L"Currency");
        break;
      case VT_DATE:
        wcscat_s(szNote, slNote, L"Date");
        break;
      case VT_BSTR:
        wcscat_s(szNote, slNote, L"String");
        break;
      case VT_DISPATCH:
        wcscat_s(szNote, slNote, L"Object");
        break;
      case VT_BOOL:
        wcscat_s(szNote, slNote, L"Boolean");
        break;
      case VT_VARIANT:
        wcscat_s(szNote, slNote, L"Variant");
        break;
      case VT_UNKNOWN:
        wcscat_s(szNote, slNote, L"DataObject");
        break;
      case VT_DECIMAL:
        wcscat_s(szNote, slNote, L"Decimal");
        break;
      case VT_UI1:
        wcscat_s(szNote, slNote, L"Byte");
        break;
      case VT_RECORD:
        wcscat_s(szNote, slNote, L"UserDefinedType");
        break;
      default:
        wcscat_s(szNote, slNote, L"*Othter");
        break;
      }

      wcscat_s(szNote, slNote, L"]");

      ITInfo->ReleaseFuncDesc(PFD);
      ITInfo->Release();
      ITLib->Release();
      return TRUE;
    }

    ITInfo->ReleaseFuncDesc(PFD);
  }

  ITInfo->Release();
  ITLib->Release();
  return FALSE;
}