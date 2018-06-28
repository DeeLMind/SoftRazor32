#pragma once

#ifndef _SR_INC_COMMON_H_
#include "../src/common.h"
#endif

#include "../src/g_type.h"

extern BOOL     AddClassTable(HWND hWnd, PVOID pClass);
extern PVOID    GetClassPointer(HWND hWnd);
extern BOOL     DelClassTable(HWND hWnd);

#define DLFG_DATSTK           0
#define DLFG_ASMREG           1
#define DLFG_DLADDR           2
#define DLFG_DLBYTE           3
#define DLFG_DLMNEM           4
#define DLFG_DLINFO           5
#define DLFG_DLNOTE           6

//�ඨ��:

/* MDI���ĵ����ڻ��� */
class MDIForm
{
protected:
  MDIForm(HWND h_MDI) : hw_MDI(h_MDI), m_hWnd(NULL) {}
  virtual HWND CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style) = 0;
  HWND m_hWnd;
  HWND hw_MDI;

public:
  ~MDIForm();

  HWND GetMainhWnd();
  BOOL SetTop();
  BOOL DestroyForm();
};

/* �������ڻ��� */
class FloatDialog
{
protected:
  FloatDialog(HWND hOwner) : m_hWnd(NULL), m_hOwner(hOwner) {}
  virtual HWND CreateForm() = 0;
  HWND  m_hWnd;
  HWND  m_hOwner;

public:
  ~FloatDialog();

  HWND GetMainhWnd();
  BOOL SetTop();
  BOOL DestroyForm();
};

/*ģ�鴰����*/
class MC_Module : public MDIForm
{
private:
  static MC_Module * This;

  HWND mdllv;
  HWND sctlv;
  BOOL mdlASCEND;

  MC_Module(HWND hMDI) : mdllv(0), sctlv(0), mdlASCEND(0), MDIForm(hMDI) {}

  void mdlInsertColumn();
  void sctInsertColumn();
  void UpdateModuleList();
  void UpdateSectionInfo(HMODULE hMod);
  static void UpdatelParam(HWND hlv);
  static int CALLBACK mdl_CompareFunc(LPARAM lParam1, LPARAM lParam2, LPARAM lParam);

public:
  ~MC_Module();

  /* ��ȡ��ʵ�� */
  static MC_Module * GetInstance(HWND hMDI)
  {
    if (This == NULL)
      This = new MC_Module(hMDI);
    return This;
  }

  HWND CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style);
  HWND GetModuleListhWnd();
  static LRESULT CALLBACK mdlWndProc(HWND, UINT, WPARAM, LPARAM);
};


/*API�ҹ�������*/
class MC_ApiHook : public MDIForm
{
private:
  static MC_ApiHook * This;
  HWND ahlv;
  HWND chk_iat;
  HWND chk_eat;
  HWND chk_inline;
  HWND cb_mod;
  HWND btn_upmod;
  HWND btn_chk;
  HWND lv_hook;
  DWORD drCur;
  DWORD drMax;
  int drpl;
  int iidx;
  BOOL IsCheck;
  BOOL blStop;
  WCHAR StateText[64];

  MC_ApiHook(HWND hMDI) : drCur(0), drMax(0), drpl(0), IsCheck(0), blStop(0), MDIForm(hMDI) {}
  void InsertColumn();
  void UpdateModule();
  void SetStateText(const PWCHAR ptext);
  void SetProgress(DWORD Cur, DWORD Max);
  BOOL AddItem(PWCHAR hMod, PWCHAR hObj, PWCHAR hType, PWCHAR hAddr, PWCHAR hOVal, PWCHAR hCVal, PAPIHOOK_LV_TYPE lParam);
  void CheckIAT(HMODULE hMod, LPMODULEENTRY32 lpme);
  void CheckEAT(HMODULE hMod, LPMODULEENTRY32 lpme);
  void CheckInline(HMODULE hMod, LPMODULEENTRY32 lpme);
  void CheckHook();
  void StopCheck();

public:
  ~MC_ApiHook();

  /* ��ȡ��ʵ�� */
  static MC_ApiHook * GetInstance(HWND hMDI)
  {
    if (This == NULL)
      This = new MC_ApiHook(hMDI);
    return This;
  }

  HWND CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style);

  static LRESULT CALLBACK ahWndProc(HWND, UINT, WPARAM, LPARAM);
};

/*�ڴ洰����*/
class MC_Memory : public MDIForm
{
private:
  static MC_Memory * This;
  HWND memlv;
  HMENU PM_0;
  int iSelect;
  BOOL OnlyCommit;

  MC_Memory(HWND hMDI) : PM_0(0), iSelect(-1), OnlyCommit(FALSE), MDIForm(hMDI) {}
  void memInsertColumn();
  void UpdateMemoryList();

public:
  ~MC_Memory();

  /* ��ȡ��ʵ�� */
  static MC_Memory  * GetInstance(HWND hMDI)
  {
    if (This == NULL)
      This = new MC_Memory(hMDI);
    return This;
  }

  HWND CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style);
  static LRESULT CALLBACK memWndProc(HWND, UINT, WPARAM, LPARAM);
};


/*����Ϣ������*/
class MC_Heap : public MDIForm
{
private:
  static MC_Heap * This;
  HWND hw_heaplv;

  MC_Heap(HWND hMDI) : MDIForm(hMDI) {}
  void heapInsertColumn();
  void UpdateHeapList();

public:
  ~MC_Heap();

  /* ��ȡ��ʵ�� */
  static MC_Heap  * GetInstance(HWND hMDI)
  {
    if (This == NULL)
      This = new MC_Heap(hMDI);
    return This;
  }

  HWND CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style);
  static LRESULT CALLBACK heapWndProc(HWND, UINT, WPARAM, LPARAM);
};

/*�̴߳�����*/
class MC_Thread : public MDIForm
{
private:
  static MC_Thread * This;
  HWND hw_thlv;
  HMENU PM_3;
  int iSelect;

  MC_Thread(HWND hMDI) : PM_3(NULL), iSelect(-1), MDIForm(hMDI) {}
  void threadInsertColumn();
  void UpdateThreadList();
public:
  ~MC_Thread();

  /* ��ȡ��ʵ�� */
  static MC_Thread  * GetInstance(HWND hMDI)
  {
    if (This == NULL)
      This = new MC_Thread(hMDI);
    return This;
  }

  HWND CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style);
  static LRESULT CALLBACK threadWndProc(HWND, UINT, WPARAM, LPARAM);
};

/* �����Ĵ�������  */
class FW_RegWnd : public FloatDialog
{
private:
  HANDLE        cThread;

  void UpdateThreadInfo();
  void UpdateRegistersByHandle(HANDLE hThread);
  void UpdateRegistersById(DWORD TID);
  void ResetRegistersText();

public:
  FW_RegWnd(HWND hOwner) : cThread(NULL), FloatDialog(hOwner) {}
  ~FW_RegWnd();

  HWND          CreateForm();
  static        INT_PTR CALLBACK fw_regWnd(HWND, UINT, WPARAM, LPARAM);
};

/*����ര����*/
class MC_Disasm : public MDIForm
{
private:
  static MC_Disasm    * This;
  FW_RegWnd           * fw_reg;
  HWND                  hw_vsb;             //��ֱ������
  HWND                  hw_tb;              //������
  BOOL                  blDraw;             //�Ƿ����
  UINT                  cdFlag0;            //�Զ�����Ʊ�־0
  UINT                  cdFlag1;            //�Զ�����Ʊ�־1
  PVOID                 aBaseAddr;          //������ַ
  UINT                  aBlockSize;         //�������С
  UINT                  aDrawIndex;         //����������
  PVOID                 aDrawAddr;          //����౻���Ƶĵ�ַ
  PVOID                 aSelAddrA;          //��ѡ��ַA
  PVOID                 aSelAddrB;          //��ѡ��ַB
  UINT                  diCount;            //�������Ŀ����
  PVOID                 dBaseAddr;          //����ת����ַ
  SIZE_T                dBlockSize;         //����ת����С
  PVOID                 dDrawAddr;          //������ͼ�����Ƶĵ�ַ
  PVOID                 dSelAddrA;          //��ѡ��ַA
  PVOID                 dSelAddrB;          //��ѡ��ַB

  HeapMemoryManager   * hmm;
  HeapMemoryManager   * smm;
  PPDAI                 ItemArray;

  DWORD                 sbdc;

  WORD                  cWidth;
  WORD                  cHeight;

  WORD                  drLeft;             //�Ի�����
  WORD                  drTop;              //Draw Rect
  WORD                  drWidth;
  WORD                  drHeight;

  WORD                  asmLeft;            //���������
  WORD                  asmregTop;
  WORD                  asmWidth;
  WORD                  asmregHeight;
  WORD                  asmRight;

  WORD                  datLeft;            //Data Rect
  WORD                  datstkTop;
  WORD                  datWidth;
  WORD                  datstkHeight;

  WORD                  regLeft;            //Registers Rect
  WORD                  regWidth;

  WORD                  stkLeft;            //Stack Rect
  WORD                  stkWidth;

  WORD                  dlAddr;             //Draw Addr Left
  WORD                  dlByte;             //Draw Byte Left
  WORD                  dlMnem;             //Draw Mnemonic Left
  WORD                  dlInfo;             //Draw Info Left
  WORD                  dlNote;             //Draw Note Left

  WORD                  dtText;
  WORD                  dwText;
  WORD                  dhText;

  WORD                  aMaxItem;
  WORD                  dMaxItem;

  WCHAR                 aOwnerName[64];     //�������������
  WCHAR                 dOwnerName[64];     //����ת����������

  MC_Disasm(HWND hMDI) : fw_reg(NULL), blDraw(TRUE), aBaseAddr(0), aBlockSize(0), aDrawAddr(0),
    aSelAddrA(0), aSelAddrB(0), diCount(0), ItemArray(0), drTop(24), drLeft(0),
    asmWidth(440), asmregHeight(280), datWidth(300),
    drWidth(0), drHeight(0), dlAddr(68), dlByte(140), dlMnem(224), dlInfo(284), dlNote(344), dwText(0),
    dhText(42), aMaxItem(0), hmm(0), smm(0), MDIForm(hMDI)
  {
    hmm = new HeapMemoryManager(HMMF_ENABLE_LFH | HMMF_ENABLE_COMPACT, 4194304);
    smm = new HeapMemoryManager(HMMF_ENABLE_LFH | HMMF_ENABLE_COMPACT, 2097152);
  }

  HWND                  CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style);
  PDAI                  GetItemByIndex(UINT Idx);
  PDAI                  GetItemByAddress(PVOID Address);
  UINT                  GetItemIndexByAddress(PVOID Address);
  BOOL                  DelItemByAddress(PVOID Address);
  void                  DelItemArray();
  void                  DrawDisasmRect(HDC hDC);
  void                  DrawDataRect(HDC hDC);
  void                  DrawRegistersRect(HDC hDC);
  void                  DrawStackRect(HDC hDC);
  void                  DrawTextRect(HDC hDC);
  void                  DrawDisasmSubRect(HDC hDC, UINT dFlag);
  void                  UpdateDisasmSubRect();
  void                  DeviceContextDraw(HDC hDC, LPPAINTSTRUCT lpps);
  void                  SingleBufferDraw(HDC hDC, LPPAINTSTRUCT lpps);
  BOOL                  DoubleBufferDraw(HDC hDC, LPPAINTSTRUCT lpps);
  void                  SetScrollBar(UINT CurrPos, UINT MaxPos);

public:
  ~MC_Disasm();

  /* ��ȡ��ʵ�� */
  static MC_Disasm  * GetInstance(HWND hMDI)
  {
    if (This == NULL)
      This = new MC_Disasm(hMDI);
    return This;
  }

  int                 AssembleOfAddress(PVOID Addr, XEDPARSE * XEDP);
  BOOL                DisasmOfAddress(LPCVOID dsAddress, DWORD Flag);
  BOOL                GotoAddress(PBYTE dsAddress, DWORD Flag);
  BOOL                DumpOfAddress(PBYTE ddAddress, DWORD Flag);
  PVOID               GetSelLineAddress();
  UINT                GetSelLineIndex();
  UINT                GetAsmTextByAddress(PVOID Addr, PWCHAR pasmt, UINT scount);
  UINT                GetOpLenByAddress(PVOID Addr);
  void                InvalidateDisasmRect();
  void                InvalidateDataRect();
  void                InvalidateRegistersRect();
  void                InvalidateStackRect();
  void                InvalidateDisasmAndRegistersRect();
  void                InvalidateDataAndStackRect();
  void                InvalidateAllRect();
  void                InvalidateHeightRect(WORD Top, WORD Bottom);
  void                InvalidateRectByRect(WORD Left, WORD Top, WORD Right, WORD Bottom);
  void                OuterRedrawDisasmRect();
  void                OuterRedrawDataRect();
  void                OuterRedrawRegistersRect();
  void                OuterRedrawStackRect();
  void                OuterRedrawDisasmAndRegistersRect();
  void                OuterRedrawDataAndStackRect();
  void                OuterRedrawAllRect();
  void                OuterRedrawDisasmSubRect(UINT dFlag);

  PDAI                HitTest(WORD px, WORD py);
  BOOL                HitRect(LPCRECT rect0, LPCRECT rect1);
  static              LRESULT CALLBACK disasmWndProc(HWND, UINT, WPARAM, LPARAM);
};

/*VB�ṹ��Ϣ������*/
class MC_VBInfo : public MDIForm
{
private:
  static MC_VBInfo * This;
  HWND hEdit_lphdr;
  HWND hBtn_vbok;
  HWND hBtn_sethk;
  HWND hTV_vb;
  HWND hLV_vb;
  HMENU hMenu_pop_4;
  PVBHDR lp_VbHdr;
  HTREEITEM hti_struct;
  HTREEITEM hti_cominfo;
  HTREEITEM hti_object;
  HTREEITEM hti_frm;
  HTREEITEM hti_bas;
  HTREEITEM hti_cls;
  HTREEITEM hti_ctl;
  CTreeCtrl ctv_vb;
  CListCtrl clv_vb;
  WORD m_Width, m_Height;

  MC_VBInfo(HWND hMDI) : hMenu_pop_4(0), lp_VbHdr(0), MDIForm(hMDI) {}

  void ResetView();
  void vbResetColumn(DWORD dwType);
  void ListHeader();
  void ListProjectInfo();
  void ListCOMRegData();
  void ListCOMRegInfo();
  void ListCOMRegInfoSub(PVB_COM_REG_INFO lpComRegInfo);
  void ListObjectTable();
  void ListPublicObjectDescr(PVB_PUB_OBJ_DESCR lpVbObject);
  void ListObjectInfo(PVB_OBJECT_INFO lpVbObjInfo);
  void ListObjectOptionalInfo(PVB_OPTIONAL_OBJECT_INFO lpVbObjOptInfo);
  void RefreshStructList();

public:
  ~MC_VBInfo();

  /* ��ȡ��ʵ�� */
  static MC_VBInfo  * GetInstance(HWND hMDI)
  {
    if (This == NULL)
      This = new MC_VBInfo(hMDI);
    return This;
  }

  HWND CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style);
  void ParseHeader(PVBHDR lpVbHdr);
  void ParseModule(PVOID lpMod);
  static void PrintThreadFlags(uint32_t flgThread, PWCHAR strBuffer, size_t maxLength);
  static void PrintOLEMISC(OLEMISC OleMisc, PWCHAR strBuffer, size_t maxLength);
  static LRESULT CALLBACK vbwpTreeView(HWND, UINT, WPARAM, LPARAM);
  static LRESULT CALLBACK vbWndProc(HWND, UINT, WPARAM, LPARAM);
};

/*PCode���̴�����*/
class MC_PCodeProc : public MDIForm
{
private:
  static MC_PCodeProc * This;
  HWND    hObjCbx;
  HWND    hEvtCbx;
  HWND    hList;
  PVBHDR  CVBH;
  PPPLST  ProcListHead;
  PPPLST  ProcListLast;

  MC_PCodeProc(HWND hMDI) : hObjCbx(0), hEvtCbx(0), hList(0), ProcListHead(0), ProcListLast(0), MDIForm(hMDI) {}

  void    AddProc(PWCHAR pProcName, PVOID pAddr, int cb_idx);
  PVOID   GetProcAddrByName(PWCHAR pProcName);
  PVOID   GetProcAddrByIndex(int cbIdx);
  void    DelAllProc();
  void    InsertColumn();
  void    ClearList();
  void    UpdateList(PVBPDI pVbpDI);

  HWND    CreateForm(int x, int y, int cx, int cy, LPCTCH wName, DWORD Style);
  void    UpdateProc(PWCHAR pObjName);

public:
  ~MC_PCodeProc();

  /* ��ȡ��ʵ�� */
  static MC_PCodeProc  * GetInstance(HWND hMDI)
  {
    if (This == NULL)
      This = new MC_PCodeProc(hMDI);
    return This;
  }

  BOOL ParseHeader(PVBHDR pVbHeader);
  BOOL ParseProc(PVOID pProc);
  static LRESULT CALLBACK pcpWndProc(HWND, UINT, WPARAM, LPARAM);
};

/* �������� */
class MainForm
{
private:
  /* Ψһ����ʵ��ָ�� */
  static MainForm           * p_mf;

  /* �Ƿ��ʼ�� */
  static BOOL                 IsInit;

  /* ������״̬ */
  SRDBG_STATE                 SRDBGS;

  /* ״̬���ı���ɫ */
  static COLORREF             stclr;

  /* ״̬���ı� */
  static WCHAR                StateText[64];

  PLOG_TYPE                   LogHeader;
  PLOG_TYPE                   LogLast;

  MainForm(HINSTANCE hInst) : LogHeader(NULL), LogLast(NULL)
  {
    MdlBase = hInst;
    InitVar();
    LoadForm();
  }

  ~MainForm()
  {
    UnloadForm();
    p_mf = NULL;
  }

  void InitVar();
  BOOL LoadForm();
  BOOL DestroyHandle();
  BOOL UnloadForm();

public:
  static HINSTANCE            MdlBase;
  static ATOM                 rcls_sr;
  static ATOM                 rcls_mc_apihook;
  static ATOM                 rcls_mc_module;
  static ATOM                 rcls_mc_memory;
  static ATOM                 rcls_mc_heap;
  static ATOM                 rcls_mc_thread;
  static ATOM                 rcls_mc_disasm;
  static ATOM                 rcls_mc_vbinfo;
  static ATOM                 rcls_mc_pcodeproc;

  static HICON                hi_main;
  static HICON                hi_apihook;
  static HICON                hi_module;
  static HICON                hi_memory;
  static HICON                hi_thread;
  static HICON                hi_disasm;
  static HICON                hi_vbp;

  static HBRUSH               hb_white;
  static HBRUSH               hb_ltgray;
  static HBRUSH               hb_gray;
  static HBRUSH               hb_dkgray;
  static HBRUSH               hb_black;

  static HMENU                hm_main;
  static HMENU                hm_pop;
  static HMENU                hm_vb;

  static HIMAGELIST           hImageList;
  static CImageList           cImageList;

  static int                  bpi_root;
  static int                  bpi_info;
  static int                  bpi_object;
  static int                  bpi_event;
  static int                  bpi_ukctl;
  static int                  bpi_vbs;
  static int                  bpi_frm;
  static int                  bpi_bas;
  static int                  bpi_cls;
  static int                  bpi_ctl;
  static int                  bpi_run;
  static int                  bpi_pause;
  static int                  bpi_stepin;
  static int                  bpi_stepover;
  static int                  bpi_trun;
  static int                  bpi_tpause;
  static int                  bpi_tstop;

  static HFONT                hf_tas;
  static HFONT                hf_tam;
  static HFONT                hf_tal;
  static HFONT                hf_ves;
  static HFONT                hf_vem;
  static HFONT                hf_vel;
  static HFONT                hf_fix;

  static CFont                cf_tas;
  static CFont                cf_tam;
  static CFont                cf_tal;
  static CFont                cf_ves;
  static CFont                cf_vem;
  static CFont                cf_vel;
  static CFont                cf_fix;

  static HWND                 hw_main;
  static HWND                 hw_progress;
  static HWND                 hw_cmdedit;
  static HWND                 hw_MDI;

  static MC_Disasm          * mc_disasm;
  static MC_ApiHook         * mc_apihook;
  static MC_Module          * mc_module;
  static MC_Memory          * mc_memory;
  static MC_Heap            * mc_heap;
  static MC_Thread          * mc_thread;
  static MC_VBInfo          * mc_vbinfo;
  static MC_PCodeProc       * mc_pcodeproc;

  static MemoryManager      * m_mer;   //�ڴ������
  static HeapMemoryManager  * m_hmm;   //�ѹ�����

  static PVBOBJLST              VbObjHeader;
  static PVBOBJLST              VbObjLast;

  /* ��ȡ��ʵ�� */
  static MainForm           * GetInstance(HINSTANCE hInst)
  {
    if (p_mf == NULL) p_mf = new MainForm(hInst);
    return p_mf;
  }

  /* ��������� */
  static LRESULT CALLBACK MainProc(HWND, UINT, WPARAM, LPARAM);

  /* ��Ӽ�¼ */
  int               AddLog(LPCTCH pText, LOGTYPE LogType, LPARAM lParam, func_logcb CallBack);

  /* ��ȫ����Ӷ������� */
  PVBOBJLST           AddObjectList(PVBOBJLST pobjlst);

  /* ���ݶ������ӹ��̱� */
  PVBPL             AddProcListByObject(PVBOBJLST pobjlst, PVBPL vbprclst);

  /* ���ݶ���������ӹ��̱� */
  PVBPL             AddProcListByName(PWCHAR pObjName, PVBPL vbprclst);

  /* ���ݶ���ָ���ȡ�������� */
  PVBOBJLST           GetObjectListByObject(PVBPUBOBJ pVbObj);

  /* ���ݶ������ƻ�ȡ�������� */
  PVBOBJLST           GetObjectListByName(PWCHAR pObjName);

  /* ���ݹ���������ȡ�������� */
  PVBPL             GetProcListByAddress(PWCHAR pOptObjName, PVBPDI pProcDesc);

  /* ���õ�����״̬�ı� */
  int               SetStateText(PWCHAR pText, COLORREF tc);

  /* ���õ�����״̬ */
  void              SetDbgState(SRDBG_STATE dbgs);

  /* ��ȡ������״̬ */
  SRDBG_STATE       GetDbgState();

  /* �������� */
  HWND              CreateForm(LPCTCH pwName, DWORD dwStyle, HINSTANCE hOwner);

  /* ��ȡ�������� */
  HWND              GetMainWindow();

  /* ���ٴ��� */
  BOOL              DestroyForm();

  /* �ͷ� */
  void              Release();
};