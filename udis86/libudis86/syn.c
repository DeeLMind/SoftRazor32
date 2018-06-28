/* udis86 - libudis86/syn.c
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

#include <string.h>
/*
 * Register Table - Order Matters (types.h)!
 *
 */
const char* ud_reg_tab[] =
{
  "al", "cl", "dl", "bl",
  "ah", "ch", "dh", "bh",
  "spl", "bpl", "sil", "dil",
  "r8b", "r9b", "r10b", "r11b",
  "r12b", "r13b", "r14b", "r15b",

  "ax", "cx", "dx", "bx",
  "sp", "bp", "si", "di",
  "r8w", "r9w", "r10w", "r11w",
  "r12w", "r13w", "r14w", "r15w",

  "eax", "ecx", "edx", "ebx",
  "esp", "ebp", "esi", "edi",
  "r8d", "r9d", "r10d", "r11d",
  "r12d", "r13d", "r14d", "r15d",

  "rax", "rcx", "rdx", "rbx",
  "rsp", "rbp", "rsi", "rdi",
  "r8", "r9", "r10", "r11",
  "r12", "r13", "r14", "r15",

  "es", "cs", "ss", "ds",
  "fs", "gs",

  "cr0", "cr1", "cr2", "cr3",
  "cr4", "cr5", "cr6", "cr7",
  "cr8", "cr9", "cr10", "cr11",
  "cr12", "cr13", "cr14", "cr15",

  "dr0", "dr1", "dr2", "dr3",
  "dr4", "dr5", "dr6", "dr7",
  "dr8", "dr9", "dr10", "dr11",
  "dr12", "dr13", "dr14", "dr15",

  "mm0", "mm1", "mm2", "mm3",
  "mm4", "mm5", "mm6", "mm7",

  "st0", "st1", "st2", "st3",
  "st4", "st5", "st6", "st7",

  "xmm0", "xmm1", "xmm2", "xmm3",
  "xmm4", "xmm5", "xmm6", "xmm7",
  "xmm8", "xmm9", "xmm10", "xmm11",
  "xmm12", "xmm13", "xmm14", "xmm15",

  "ymm0", "ymm1", "ymm2", "ymm3",
  "ymm4", "ymm5", "ymm6", "ymm7",
  "ymm8", "ymm9", "ymm10", "ymm11",
  "ymm12", "ymm13", "ymm14", "ymm15",

  "rip"
};


uint64_t
ud_syn_rel_target(struct ud *u, struct ud_operand *opr)
{
  const uint64_t trunc_mask = 0xffffffffffffffffull >> (64 - u->opr_mode);
  switch (opr->size)
  {
  case 8: return (u->pc + opr->lval.sbyte)  & trunc_mask;
  case 16: return (u->pc + opr->lval.sword)  & trunc_mask;
  case 32: return (u->pc + opr->lval.sdword) & trunc_mask;
  default: UD_ASSERT(!"invalid relative offset size.");
    return 0ull;
  }
}


/*
 * asmprintf
 *    Printf style function for printing translated assembly
 *    output. Returns the number of characters written and
 *    moves the buffer pointer forward. On an overflow,
 *    returns a negative number and truncates the output.
 */
int
ud_asmprintf(struct ud *u, const char *fmt, ...)
{
  int ret;
  int avail;
  va_list ap;

  va_start(ap, fmt);
  avail = u->asm_buf_size - u->asm_index - 1 /* nullchar */;
  ret = vsnprintf(&(u->asm_buf[u->asm_index]), avail, fmt, ap);

  if (ret < 0 || ret > avail)
  {
    u->asm_index = u->asm_buf_size - 1;
  }
  else
  {
    u->asm_index += ret;
  }

  va_end(ap);
  return ret;
}

/*
ud_casmprint
flag:
0x01      ͬʱʹ��ud_asmprintf
0x02      ����str
*/
int
ud_casmprint(struct ud *u, int flag, char descr, char *str)
{
  int ret = 0;

  if ((str != NULL) && (u->casm_buf != NULL) &&
    (u->casm_buf_size != 0) && (u->casm_buf_size > u->casm_index))
  {
    size_t slen = strlen(str);
    size_t avail = u->casm_buf_size - u->casm_index - 1;

    if (slen == 0) return 0;

    if (avail >= ((descr != CD_NULL) ? slen + 2 : slen + 1))
    {
      if (descr != CD_NULL)
        u->casm_buf[u->casm_index++] = descr;

      ret = memcpy_s(&u->casm_buf[u->casm_index], avail, str, slen);
      u->casm_index += slen;
      u->casm_buf[u->casm_index++] = 0;

      if (u->casm_index < u->casm_buf_size)
        u->casm_buf[u->casm_index] = 0;
    }
    else
      u->casm_error = 2;

    if (flag & 0x0001)
      ud_asmprintf(u, "%s", str);

    if (flag & 0x0002)
      str[0] = 0;
  }
  else
    u->casm_error = 1;

  return 0;
}

int
ud_casmprintf(struct ud *u, int flag, char descr, const char *fmt, ...)
{
  int ret = 0;
  va_list ap;

  va_start(ap, fmt);

  if ((u->casm_buf != NULL) && (u->casm_buf_size != 0) && 
    (u->casm_buf_size > u->casm_index))
  {
    size_t index = u->casm_index;
    size_t avail = u->casm_buf_size - u->casm_index - 1;

    if (avail >= 2)
    {
      if (descr != CD_NULL)
      {
        u->casm_buf[u->casm_index++] = descr;
        avail--;
      }

      if ((ret = vsprintf_s(&u->casm_buf[u->casm_index], avail, fmt, ap)) > 0)
      {
        u->casm_index += (size_t)ret + 1;
        if (u->casm_index < u->casm_buf_size)
          u->casm_buf[u->casm_index] = 0;
      }
      else
      {
        u->casm_index = index;
        u->casm_buf[index] = 0;
        u->casm_error = 3;
      }
    }
  }
  else
    u->casm_error = 1;

  if (flag & 0x0001)
    ud_asmprintf(u, fmt, ap);

  va_end(ap);

  return ret;
}

void
ud_syn_print_addr(struct ud *u, uint64_t addr)
{
  const char *name = NULL;
  if (u->sym_resolver)
  {
    int64_t offset = 0;
    name = u->sym_resolver(u, addr, &offset);
    if (name)
    {
      if (offset)
      {
        ud_casmprintf(u, 0x01, CD_SYMBOL, "%s", name);
        ud_casmprintf(u, 0x01, CD_DECIMAL, "%+" FMT64 "d", offset);
      }
      else
      {
        ud_casmprintf(u, 0x01, CD_SYMBOL, "%s", name);
      }
      return;
    }
  }

  //δ���ò���ӡ��ַ��־�Ļ�,�ʹ�ӡ��ַ
  if (UD_NOT_FLAG(u->asm_mode, UD_AMODE_NOT_PRINT_ADDR))
  {
    ud_casmprintf(u, 0x01, CD_IMMADDR, "0x%" FMT64 UD_HEX, addr);
  }
}


void
ud_syn_print_imm(struct ud* u, const struct ud_operand *op)
{
  uint64_t v;
  if (op->_oprcode == OP_sI && op->size != u->opr_mode)
  {
    if (op->size == 8)
    {
      v = (int64_t)op->lval.sbyte;
    }
    else
    {
      UD_ASSERT(op->size == 32);
      v = (int64_t)op->lval.sdword;
    }
    if (u->opr_mode < 64)
    {
      v = v & ((1ull << u->opr_mode) - 1ull);
    }
  }
  else
  {
    switch (op->size)
    {
    case 8: v = op->lval.ubyte;  break;
    case 16: v = op->lval.uword;  break;
    case 32: v = op->lval.udword; break;
    case 64: v = op->lval.uqword; break;
    default: UD_ASSERT(!"invalid offset"); v = 0; /* keep cc happy */
    }
  }
  ud_casmprintf(u, 0x01, CD_IMM, "0x%" FMT64 UD_HEX, v);
}


void
ud_syn_print_mem_disp(struct ud* u, const struct ud_operand *op, int sign)
{
  UD_ASSERT(op->offset != 0);
  if (op->base == UD_NONE && op->index == UD_NONE)
  {
    uint64_t v;
    UD_ASSERT(op->scale == UD_NONE && op->offset != 8);
    /* unsigned mem-offset */
    switch (op->offset)
    {
    case 16: v = op->lval.uword;  break;
    case 32: v = op->lval.udword; break;
    case 64: v = op->lval.uqword; break;
    default: UD_ASSERT(!"invalid offset"); v = 0; /* keep cc happy */
    }
    ud_casmprintf(u, 0x01, CD_IMM, "0x%" FMT64 UD_HEX, v);
  }
  else
  {
    int64_t v;
    UD_ASSERT(op->offset != 64);
    switch (op->offset)
    {
    case 8: v = op->lval.sbyte;  break;
    case 16: v = op->lval.sword;  break;
    case 32: v = op->lval.sdword; break;
    default: UD_ASSERT(!"invalid offset"); v = 0; /* keep cc happy */
    }
    if (v < 0)
    {
      ud_casmprintf(u, 0x01, CD_DEFCLR, "-");
      ud_casmprintf(u, 0x01, CD_IMM, "0x%" FMT64 UD_HEX, -v);
    }
    else if (v > 0)
    {
      if (sign)
        ud_casmprintf(u, 0x01, CD_DEFCLR, "+");

      ud_casmprintf(u, 0x01, CD_IMM, "0x%" FMT64 UD_HEX, v);
    }
  }
}

/*
vim: set ts=2 sw=2 expandtab
*/
