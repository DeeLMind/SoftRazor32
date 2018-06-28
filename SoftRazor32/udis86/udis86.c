/* udis86 - udis86/udis86.c
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

#include "udint.h"
#include "extern.h"
#include "decode.h"

static void ud_inp_init(struct ud *u);

/* =============================================================================
* ud_init
*    初始化 ud_t 对象.
* =============================================================================
*/
void 
  ud_init(struct ud *u)
{
  memset((void*)u, 0, sizeof(struct ud));
  ud_set_mode(u, 16);
  u->mnemonic = UD_Iinvalid;
  ud_set_pc(u, 0);
#ifndef __UD_STANDALONE__
  ud_set_input_file(u, stdin);
#endif /* __UD_STANDALONE__ */

  ud_set_asm_buffer(u, u->asm_buf_int, sizeof(u->asm_buf_int) / 2);
  ud_enable_casm(u,1);
}

unsigned int
  ud_get_count(struct ud *u)
{
  unsigned int dicount = 0;

  if (u->inp_end)
    return 0;

  while (u->inp_buf_index < u->inp_buf_count)
  {
    if (ud_decode(u) == 0)
      u->inp_buf_index++;
    else
      dicount++;
  }

  return dicount;
}


/* =============================================================================
* ud_disassemble
*    反汇编一个指令,并返回反汇编的字节数.0表示反汇编结束.
* =============================================================================
*/
unsigned int
  ud_disassemble(struct ud *u)
{
  int len;

  if (u->inp_end)
    return 0;

  if ((len = ud_decode(u)) > 0)
  {
    if (u->translator != NULL)
    {
      u->asm_buf[0] = 0;
      u->asm_index = 0;
      if (u->casm_buf)
      {
        u->casm_buf[0] = 0;
        u->casm_index = 0;
        u->casm_error = 0;
      }
      u->translator(u);
    }
  }
  return len;
}


/* =============================================================================
* ud_set_mode() - 设置反汇编模式.
* =============================================================================
*/
void 
  ud_set_mode(struct ud *u, uint8_t m)
{
  switch(m) {
  case 16:
  case 32:
  case 64:
    u->dis_mode = m ;
    return;
  default:
    u->dis_mode = 16;
    return;
  }
}

/* =============================================================================
* ud_set_vendor() - Set vendor.
* =============================================================================
*/
void 
  ud_set_vendor(struct ud *u, unsigned v)
{
  switch(v) {
  case UD_VENDOR_INTEL:
    u->vendor = v;
    break;
  case UD_VENDOR_ANY:
    u->vendor = v;
    break;
  default:
    u->vendor = UD_VENDOR_AMD;
  }
}

/* =============================================================================
* ud_set_pc() - 设置代码来源. 
* =============================================================================
*/
void 
  ud_set_pc(struct ud *u, uint64_t o)
{
  u->pc = o;
}

/* =============================================================================
* ud_set_syntax() - 设置输出语法.
* =============================================================================
*/
void 
  ud_set_syntax(struct ud *u, func_translator pfunc_translator)
{
  u->translator = pfunc_translator;
}

/* =============================================================================
* ud_insn() - 返回反汇编指令
* =============================================================================
*/
const wchar_t* 
  ud_insn_asm(const struct ud *u) 
{
  return u->asm_buf;
}

const wchar_t* 
  ud_insn_casm(const struct ud *u) 
{
  return u->casm_buf;
}

/* =============================================================================
* ud_insn_offset() - 返回偏移.
* =============================================================================
*/
uint64_t
  ud_insn_off(const struct ud *u) 
{
  return u->insn_offset;
}


/* =============================================================================
* ud_insn_hex() - 返回反汇编指令的16进制形式.
* =============================================================================
*/
const wchar_t* 
  ud_insn_hex(struct ud *u) 
{
  u->insn_hexcode[0] = 0;
  if (!u->error) {
    unsigned int i;
    const unsigned char *src_ptr = ud_insn_ptr(u);
    wchar_t *src_hex;
    src_hex = (wchar_t *) u->insn_hexcode;
    /* for each byte used to decode instruction */
    for (i = 0; i < ud_insn_len(u) && i < sizeof(u->insn_hexcode) / 2; ++i, ++src_ptr)
    {
      swprintf_s(src_hex, 64, L"%02X", *src_ptr & 0xFF);
      src_hex += 2;
    }
  }
  return u->insn_hexcode;
}


/* =============================================================================
* ud_insn_ptr
*    返回一个指向包含反汇编字节的缓冲区.
* =============================================================================
*/
const uint8_t* 
  ud_insn_ptr(const struct ud *u) 
{
  return (u->inp_buf == NULL) ? 
    u->inp_sess : u->inp_buf + (u->inp_buf_index - u->inp_ctr);
}


/* =============================================================================
* ud_insn_len
*    返回反汇编的字节数.
* =============================================================================
*/
unsigned int 
  ud_insn_len(const struct ud *u) 
{
  return u->inp_ctr;
}


/* =============================================================================
* ud_insn_get_opr
*    返回操作数结构表示当前反汇编指令的第n个操作数.
*    如果没有这样的操作数返回NULL.
* =============================================================================
*/
const struct ud_operand*
  ud_insn_opr(const struct ud *u, unsigned int n)
{
  if (n > 2 || u->operand[n].type == UD_NONE)
    return NULL; 
  else
    return &u->operand[n];
}


/* =============================================================================
* ud_opr_is_sreg
*    如果给定的操作数是段寄存器类型返回非零.
* =============================================================================
*/
int
  ud_opr_is_sreg(const struct ud_operand *opr)
{
  return opr->type == UD_OP_REG && 
    opr->base >= UD_R_ES   &&
    opr->base <= UD_R_GS;
}


/* =============================================================================
* ud_opr_is_sreg
*    如果给定的操作数是通用寄存器类型返回非零.
* =============================================================================
*/
int
  ud_opr_is_gpr(const struct ud_operand *opr)
{
  return opr->type == UD_OP_REG && 
    opr->base >= UD_R_AL   &&
    opr->base <= UD_R_R15;
}


/* =============================================================================
* ud_set_user_opaque_data
* ud_get_user_opaque_data
*    获取/设置用户opaqute数据指针
* =============================================================================
*/
void
  ud_set_user_opaque_data(struct ud *u, void *opaque)
{
  u->user_opaque_data = opaque;
}

void*
  ud_get_user_opaque_data(const struct ud *u)
{
  return u->user_opaque_data;
}


/* =============================================================================
* ud_set_asm_buffer
*    允许用户设置一个汇编程序的输出缓冲区.如果`buf`为NULL,我们切换回内部缓冲区.
* =============================================================================
*/
void
  ud_set_asm_buffer(struct ud *u, wchar_t *buf, size_t maxcount)
{
  if (buf == NULL) {
    buf = u->asm_buf_int;
    maxcount = sizeof(u->asm_buf_int) / 2;
  } 
  u->asm_buf = buf;
  u->asm_index = 0;
  u->asm_buf_max_count = maxcount;
}

void 
  ud_enable_casm(struct ud *u,int bl)
{
  if (bl)
    ud_set_casm_buffer(u,0,0);
  else
  {
    u->casm_buf = NULL;
    u->casm_index = 0;
    u->casm_error = 0;
    u->casm_buf_max_count = 0;
  }
}

void 
  ud_set_casm_buffer(struct ud *u, wchar_t *buf, size_t maxcount)
{
  if (buf == NULL || maxcount < 4) {
    buf = u->casm_buf_int;
    maxcount = sizeof(u->casm_buf_int);
  } 
  u->casm_buf = buf;
  u->casm_index = 0;
  u->casm_error = 0;
  u->casm_buf_max_count = maxcount;
}

/* =============================================================================
* ud_set_sym_resolver
*    Set symbol resolver for relative targets used in the translation
*    phase.
*
*    The resolver is a function that takes a uint64_t address and returns a
*    symbolic name for the that address. The function also takes a second
*    argument pointing to an integer that the client can optionally set to a
*    non-zero value for offsetted targets. (symbol+offset) The function may
*    also return NULL, in which case the translator only prints the target
*    address.
*
*    The function pointer maybe NULL which resets symbol resolution.
* =============================================================================
*/

void
  ud_set_sym_resolver(struct ud *u, func_resolver resolver)
{
  u->sym_resolver = resolver;
}


/* =============================================================================
* ud_insn_mnemonic
*    返回当前指令助记符.
* =============================================================================
*/
enum ud_mnemonic_code
  ud_insn_mnemonic(const struct ud *u)
{
  return u->mnemonic;
}


/* =============================================================================
* ud_lookup_mnemonic
*    Looks up mnemonic code in the mnemonic string table.
*    Returns NULL if the mnemonic code is invalid.
* =============================================================================
*/
const wchar_t*
  ud_lookup_mnemonic(enum ud_mnemonic_code c)
{
  if (c < UD_MAX_MNEMONIC_CODE)
  return ud_mnemonics_str[c];
  else
    return NULL;
}


/* 
* ud_inp_init
*    初始化输入系统.
*/
static void
  ud_inp_init(struct ud *u)
{
  u->inp_hook      = NULL;
  u->inp_buf       = NULL;
  u->inp_buf_count  = 0;
  u->inp_buf_index = 0;
  u->inp_curr      = 0;
  u->inp_ctr       = 0;
  u->inp_end       = 0;
  UD_NON_STANDALONE(u->inp_file = NULL);
}


/* =============================================================================
* ud_inp_set_hook
*    设定输入Hook.
* =============================================================================
*/
void 
  ud_set_input_hook(register struct ud *u, func_inp_hook pfunc_inp_hook)
{
  ud_inp_init(u);
  u->inp_hook = pfunc_inp_hook;
}

/* =============================================================================
* ud_inp_set_buffer
*    设置输入缓冲区.
* =============================================================================
*/
void 
  ud_set_input_buffer(register struct ud *u, const uint8_t *buf, size_t clen)
{
  ud_inp_init(u);
  u->inp_buf = buf;
  u->inp_buf_count = clen;
  u->inp_buf_index = 0;
}


#ifndef __UD_STANDALONE__
/* =============================================================================
* ud_input_set_file
*    设置输入文件.
* =============================================================================
*/
static int __stdcall
  inp_file_hook(struct ud *u)
{
  return fgetc(u->inp_file);
}

void 
  ud_set_input_file(register struct ud *u, FILE* f)
{
  ud_inp_init(u);
  u->inp_hook = inp_file_hook;
  u->inp_file = f;
}
#endif /* __UD_STANDALONE__ */


/* =============================================================================
* ud_input_skip
*    跳过n个输入字节.
* ============================================================================
*/
void 
  ud_input_skip(struct ud *u, size_t n)
{
  if (u->inp_end)
    return;
  if (u->inp_buf == NULL)
  {
    while (n--)
    {
      int c = u->inp_hook(u);

      if (c == UD_EOI)
        goto eoi;
    }
    return;
  } else {
    if (n > u->inp_buf_count || u->inp_buf_index > u->inp_buf_count - n)
    {
      u->inp_buf_index = u->inp_buf_count; 
      goto eoi;
    }
    u->inp_buf_index += n; 
    return;
  }
eoi:
  u->inp_end = 1;
  UDERR(u, "不能跳过,收到EOI\b");
  return;
}


/* =============================================================================
* ud_input_end
*    如果输入流结束,返回非零.
* =============================================================================
*/
int
  ud_input_end(const struct ud *u)
{
  return u->inp_end;
}

/* vim:set ts=2 sw=2 expandtab */
