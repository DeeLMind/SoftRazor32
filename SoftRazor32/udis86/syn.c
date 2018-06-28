/* udis86 - udis86/syn.c
 *
 * Copyright (c) 2002-2013 Vivek Thampi
 * All rights reserved.
 * 
 * Redistribution and use in source and binary forms, with or without modification, 
 * are permitted provided that the following conditions are met:
 * 
 *     * Redistributions of source code must retain the above copyright notice, 
 *       this list of conditions and the following disclaimer.
 *     * Redistributions in binary form must reproduce the above copyright notice, 
 *       this list of conditions and the following disclaimer in the documentation 
 *       and/or other materials provided with the distribution.
 * 
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS" AND 
 * ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE IMPLIED 
 * WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE ARE 
 * DISCLAIMED. IN NO EVENT SHALL THE COPYRIGHT OWNER OR CONTRIBUTORS BE LIABLE FOR 
 * ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL DAMAGES 
 * (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR SERVICES; 
 * LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER CAUSED AND ON 
 * ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT LIABILITY, OR TORT 
 * (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY OUT OF THE USE OF THIS 
 * SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH DAMAGE.
 */
#include "types.h"
#include "decode.h"
#include "syn.h"
#include "udint.h"

/* -----------------------------------------------------------------------------
 * Intel Register Table - Order Matters (types.h)!
 * -----------------------------------------------------------------------------
 */
const wchar_t* ud_reg_tab[] = 
{
  L"al",   L"cl",   L"dl",   L"bl",
  L"ah",   L"ch",   L"dh",   L"bh",
  L"spl",  L"bpl",    L"sil",    L"dil",
  L"r8b",  L"r9b",    L"r10b",   L"r11b",
  L"r12b", L"r13b",   L"r14b",   L"r15b",

  L"ax",   L"cx",   L"dx",   L"bx",
  L"sp",   L"bp",   L"si",   L"di",
  L"r8w",  L"r9w",    L"r10w",   L"r11w",
  L"r12w", L"r13w"  , L"r14w",   L"r15w",
  
  L"eax",  L"ecx",    L"edx",    L"ebx",
  L"esp",  L"ebp",    L"esi",    L"edi",
  L"r8d",  L"r9d",    L"r10d",   L"r11d",
  L"r12d", L"r13d",   L"r14d",   L"r15d",
  
  L"rax",  L"rcx",    L"rdx",    L"rbx",
  L"rsp",  L"rbp",    L"rsi",    L"rdi",
  L"r8",   L"r9",   L"r10",    L"r11",
  L"r12",  L"r13",    L"r14",    L"r15",

  L"es",   L"cs",   L"ss",   L"ds",
  L"fs",   L"gs", 

  L"cr0",  L"cr1",    L"cr2",    L"cr3",
  L"cr4",  L"cr5",    L"cr6",    L"cr7",
  L"cr8",  L"cr9",    L"cr10",   L"cr11",
  L"cr12", L"cr13",   L"cr14",   L"cr15",
  
  L"dr0",  L"dr1",    L"dr2",    L"dr3",
  L"dr4",  L"dr5",    L"dr6",    L"dr7",
  L"dr8",  L"dr9",    L"dr10",   L"dr11",
  L"dr12", L"dr13",   L"dr14",   L"dr15",

  L"mm0",  L"mm1",    L"mm2",    L"mm3",
  L"mm4",  L"mm5",    L"mm6",    L"mm7",

  L"st0",  L"st1",    L"st2",    L"st3",
  L"st4",  L"st5",    L"st6",    L"st7", 

  L"xmm0", L"xmm1",   L"xmm2",   L"xmm3",
  L"xmm4", L"xmm5",   L"xmm6",   L"xmm7",
  L"xmm8", L"xmm9",   L"xmm10",  L"xmm11",
  L"xmm12",  L"xmm13",  L"xmm14",  L"xmm15",

  L"rip"
};


uint64_t
ud_syn_rel_target(struct ud *u, struct ud_operand *opr)
{
  uint64_t trunc_mask = 0xffffffffffffffffull >> (64 - u->opr_mode);
  if (u->dis_mode < 32) trunc_mask >>= (64 - u->opr_mode);

  switch (opr->size)
  {
  case 8: return (u->pc + opr->lval.sbyte)  & trunc_mask;
  case 16:
  {
    int delta = (opr->lval.sword & trunc_mask);
    if ((u->pc + delta) > 0xffff)
      return (u->pc & 0xf0000) + ((u->pc + delta) & 0xffff);
    return (u->pc + delta);
  }
  case 32: return (u->pc + opr->lval.sdword) & trunc_mask;
  default: UD_ASSERT(!"无效的相对偏移大小.");
    return 0ull;
  }
}


/*
 * asmprintf
 *    Printf style function for printing translated assembly
 *    output. Returns the number of characters written and
 *    moves the buffer pointer forward. On an overflow,
 *    returns a negative number and truncates the output.
 *
 *    printf类型的函数进行打印翻译汇编输出。返回写入的字符数和移动缓冲区
 *    的指针向前。如果溢出，则返回一个负数，并截断输出。
 */

int
wprintfcat(wchar_t *dest,size_t destcount,wchar_t *_format,...)
{
  int ret = 0;
  wchar_t *buff = (wchar_t *)calloc(destcount,sizeof(wchar_t));
  va_list ap;

  if (!buff)
    return ret;

  va_start(ap, _format);

  if (_vsnwprintf_s(buff,destcount,destcount,_format,ap) == -1)
    goto ErrExit;
  ret = wcscat_s(dest,destcount,buff);

ErrExit:
  free(buff);
  va_end(ap);
  return ret;
}

int
ud_asmprintf(struct ud *u, const wchar_t *fmt, ...)
{
  int ret;
  int over = u->asm_buf_max_count - u->asm_index; /* nullchar */
  va_list ap;
  va_start(ap, fmt);

  ret = _vsnwprintf_s(&u->asm_buf[u->asm_index], over, over, fmt, ap);
  if (ret < 0 || ret > over) {
      u->asm_index = u->asm_buf_max_count - 1;
  } else {
      u->asm_index += ret;
  }
  va_end(ap);
  return ret;
}

int
ud_casmprint(struct ud *u,short stype,short pflag,wchar_t *pstr)
{
  size_t slen = wcslen(pstr);
  size_t over = u->casm_buf_max_count - u->casm_index;

  if (!slen) return 0;
  if (!u->casm_buf || !u->casm_buf_max_count || over < slen + 2 || stype == UDCA_Null) goto TestFlag;

  u->casm_buf[u->casm_index++] = stype;
  wmemcpy_s(&u->casm_buf[u->casm_index],slen,pstr,slen);
  u->casm_index += slen;
  u->casm_buf[u->casm_index++] = 0;

  if (u->casm_index < u->casm_buf_max_count) u->casm_buf[u->casm_index] = 0;
TestFlag:
  if (pflag & 1) ud_asmprintf(u,L"%s",pstr);
  if (pflag & 2) pstr[0] = 0;
  return 1;
}

void
ud_syn_print_addr(struct ud *u,uint64_t addr,wchar_t *wtmp,size_t wcount)      //打印地址
{
  const wchar_t *name = NULL;
  if (u->sym_resolver)  /*如果设置了符号解析函数*/{
    int64_t offset = 0;
    name = u->sym_resolver(u, addr, &offset);
    if (name) {
      if (offset) {
        wprintfcat(wtmp,wcount,L"%s%+" FMT64 L"d", name, offset); //符号+偏移 如:testsym+10000
      } else {
        wprintfcat(wtmp,wcount,L"%s", name);  //纯符号 如:testsym
      }
      return;
    }
  }
  /*switch (u->dis_mode) {
  case 16:

  }*/
  wprintfcat(wtmp,wcount,L"0x%" FMT64 L"X", addr);
}

void
ud_syn_print_imm(struct ud *u,const struct ud_operand *op,wchar_t *wtmp,size_t wcount)     //打印立即数
{
  uint64_t v;
  if (op->_oprcode == OP_sI && op->size != u->opr_mode) {
    if (op->size == 8) {
      v = (int64_t)op->lval.sbyte;
    } else {
      UD_ASSERT(op->size == 32);
      v = (int64_t)op->lval.sdword;
    }
    if (u->opr_mode < 64) {
      v = v & ((1ull << u->opr_mode) - 1ull);
    }
  } else {
    switch (op->size) {
    case 8 : v = op->lval.ubyte;  break;
    case 16: v = op->lval.uword;  break;
    case 32: v = op->lval.udword; break;
    case 64: v = op->lval.uqword; break;
    default: UD_ASSERT(!L"无效偏移"); v = 0; /* keep cc happy */
    }
  }
  wprintfcat(wtmp,wcount,L"0x%" FMT64 L"X", v);
}


void
ud_syn_print_mem_disp(struct ud *u,const struct ud_operand *op,int sign,wchar_t *wtmp,size_t wcount)
{
  UD_ASSERT(op->offset != 0);
 if (op->base == UD_NONE && op->index == UD_NONE) {
    uint64_t v;
    UD_ASSERT(op->scale == UD_NONE && op->offset != 8);
    /* unsigned mem-offset */
    switch (op->offset) {
    case 16: v = op->lval.uword;  break;
    case 32: v = op->lval.udword; break;
    case 64: v = op->lval.uqword; break;
    default: UD_ASSERT(!L"无效偏移"); v = 0; /* keep cc happy */
    }
    wprintfcat(wtmp,wcount,L"0x%" FMT64 L"X", v);
  } else {
    int64_t v;
    UD_ASSERT(op->offset != 64);
    switch (op->offset) {
    case 8 : v = op->lval.sbyte;  break;
    case 16: v = op->lval.sword;  break;
    case 32: v = op->lval.sdword; break;
    default: UD_ASSERT(!L"无效偏移"); v = 0; /* keep cc happy */
    }
    if (v < 0) {
      wprintfcat(wtmp,wcount,L"-0x%" FMT64 L"X", -v);
    } else if (v > 0) {
      wprintfcat(wtmp,wcount,L"%s0x%" FMT64 L"X", sign? L"+" : L"", v);
    }
  }
}

/*
vim: set ts=2 sw=2 expandtab
*/
