#pragma once

#ifndef _INC_HMM_H_
#define _INC_HMM_H_

#ifndef _INC_WINDOWS
#include <Windows.h>
#endif

#ifndef _INC_MALLOC
#include <malloc.h>
#endif

#define HMMF_ENABLE_LFH                         0x01U           //启用低碎片堆
#define HMMF_ENABLE_EXECUTE                     0x02U           //创建可执行的堆
#define HMMF_ENABLE_EXCEPTIONS                  0x04U           //申请失败时,抛出异常而不是返回NULL
#define HMMF_ENABLE_COMPACT                     0x08U           //启用压紧堆
#define HMMF_COMPACT_ALLOC                      0x10U           /* 申请内存前自动压紧堆 
(开启HMMF_ENABLE_COMPACT而不设置此位的话,默认是释放内存后自动压紧堆) */

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

  /* 等同于malloc */
  void  * WINAPI  hmalloc(SIZE_T bsize);

  /* 等同于calloc */
  void  * WINAPI  hcalloc(SIZE_T bsize);
  void  * WINAPI  hcalloc(SIZE_T bnum, SIZE_T usize);

  /* 等同于realloc */
  void  * WINAPI  hrealloc(void * pmemblk, SIZE_T newsize);

  /* 等同于realloc,但不移动内存块 */
  void  * WINAPI  hrealloc_nom(void * pmemblk, SIZE_T newsize);

  /* 等同于recalloc */
  void  * WINAPI  hrecalloc(void * pmemblk, SIZE_T newsize);
  void  * WINAPI  hrecalloc(void * pmemblk, SIZE_T newnum, SIZE_T usize);

  /* 等同于recalloc,但不移动内存块 */
  void  * WINAPI  hrecalloc_nom(void * pmemblk, SIZE_T newsize);
  void  * WINAPI  hrecalloc_nom(void * pmemblk, SIZE_T newnum, SIZE_T usize);

  /* 取内存块大小 */
  DWORD   WINAPI  hsize(void * pmemblk);

  /* 验证内存块 */
  BOOL    WINAPI  hvalidate(void * pmemblk);

  /* 释放内存块 */
  BOOL    WINAPI  hfree(void * pmemblk);

  /* 获取最后一次错误码,0即为没有错误 */
  DWORD   WINAPI  hgeterrorcode();

  /* 压紧堆 */
  SIZE_T  WINAPI  hcompact();

  /* 重置堆(创建堆,如果存在,先销毁) */
  BOOL    WINAPI  hreset(DWORD dwFlag, SIZE_T InitSize);

  /* 销毁堆 */
  BOOL    WINAPI  hdestroy();

  /* 是否低碎片堆 */
  BOOL    WINAPI  IsLowFragmentationHeap();
};

#endif