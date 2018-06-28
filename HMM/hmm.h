#pragma once

#ifndef _INC_HMM_H_
#define _INC_HMM_H_

#ifndef _INC_WINDOWS
#include <Windows.h>
#endif

#ifndef _INC_MALLOC
#include <malloc.h>
#endif

#define HMMF_ENABLE_LFH                         0x01U           //���õ���Ƭ��
#define HMMF_ENABLE_EXECUTE                     0x02U           //������ִ�еĶ�
#define HMMF_ENABLE_EXCEPTIONS                  0x04U           //����ʧ��ʱ,�׳��쳣�����Ƿ���NULL
#define HMMF_ENABLE_COMPACT                     0x08U           //����ѹ����
#define HMMF_COMPACT_ALLOC                      0x10U           /* �����ڴ�ǰ�Զ�ѹ���� 
(����HMMF_ENABLE_COMPACT�������ô�λ�Ļ�,Ĭ�����ͷ��ڴ���Զ�ѹ����) */

class HeapMemoryManager
{
private:
  HANDLE          h_heap;
  BOOL            b_lfh;
  DWORD           InitFlag;
  DWORD           ExceptionCode;

public:
  HeapMemoryManager(DWORD dwFlag, SIZE_T InitSize);
  ~HeapMemoryManager();

  /* ��ͬ��malloc */
  void  * WINAPI  hmalloc(SIZE_T bsize);

  /* ��ͬ��calloc */
  void  * WINAPI  hcalloc(SIZE_T bsize);
  void  * WINAPI  hcalloc(SIZE_T bnum, SIZE_T usize);

  /* ��ͬ��realloc */
  void  * WINAPI  hrealloc(void * pmemblk, SIZE_T newsize);

  /* ��ͬ��realloc,�����ƶ��ڴ�� */
  void  * WINAPI  hrealloc_nom(void * pmemblk, SIZE_T newsize);

  /* ��ͬ��recalloc */
  void  * WINAPI  hrecalloc(void * pmemblk, SIZE_T newsize);
  void  * WINAPI  hrecalloc(void * pmemblk, SIZE_T newnum, SIZE_T usize);

  /* ��ͬ��recalloc,�����ƶ��ڴ�� */
  void  * WINAPI  hrecalloc_nom(void * pmemblk, SIZE_T newsize);
  void  * WINAPI  hrecalloc_nom(void * pmemblk, SIZE_T newnum, SIZE_T usize);

  /* ȡ�ڴ���С */
  DWORD   WINAPI  hsize(void * pmemblk);

  /* ��֤�ڴ�� */
  BOOL    WINAPI  hvalidate(void * pmemblk);

  /* �ͷ��ڴ�� */
  BOOL    WINAPI  hfree(void * pmemblk);

  /* ��ȡ���һ�δ�����,0��Ϊû�д��� */
  DWORD   WINAPI  hgeterrorcode();

  /* ѹ���� */
  SIZE_T  WINAPI  hcompact();

  /* ���ö�(������,�������,������) */
  BOOL    WINAPI  hreset(DWORD dwFlag, SIZE_T InitSize);

  /* ���ٶ� */
  BOOL    WINAPI  hdestroy();

  /* �Ƿ����Ƭ�� */
  BOOL    WINAPI  IsLowFragmentationHeap();
};

#endif