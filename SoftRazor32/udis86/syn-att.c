/* udis86 - udis86/syn-att.c
 *
 * Copyright (c) 2002-2009 Vivek Thampi
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
#include "extern.h"
#include "decode.h"
#include "itab.h"
#include "syn.h"
#include "udint.h"

/* -----------------------------------------------------------------------------
 * opr_cast() - Prints an operand cast.
 * -----------------------------------------------------------------------------
 */
static void 
opr_cast(struct ud *u, struct ud_operand *op,wchar_t *wtmp,size_t wcount)
{
  switch(op->size) {
  case 16 : case 32 :
    wprintfcat(wtmp,wcount,L"*");
    break;
  default:
    break;
  }
}

/* -----------------------------------------------------------------------------
 * gen_operand() - Generates assembly output for each operand.
 * -----------------------------------------------------------------------------
 */
static void 
gen_operand(struct ud *u,struct ud_operand *op,wchar_t *wtmp,size_t wcount)
{
  switch(op->type) {
  case UD_OP_CONST:
    wprintfcat(wtmp,wcount,L"$0x%x", op->lval.udword);
    break;

  case UD_OP_REG:
    wprintfcat(wtmp,wcount,L"%%%s", ud_reg_tab[op->base - UD_R_AL]);
    break;

  case UD_OP_MEM:
    if (u->br_far) {
      opr_cast(u, op,wtmp,wcount);
    }
    if (u->pfx_seg) {
      wprintfcat(wtmp,wcount,L"%%%s:", ud_reg_tab[u->pfx_seg - UD_R_AL]);
    }
    if (op->offset != 0) { 
      ud_syn_print_mem_disp(u, op, 0,wtmp,wcount);
    }
    if (op->base) {
      wprintfcat(wtmp,wcount,L"(%%%s", ud_reg_tab[op->base - UD_R_AL]);
    }
    if (op->index) {
      if (op->base) {
        wprintfcat(wtmp,wcount,L",");
      } else {
        wprintfcat(wtmp,wcount,L"(");
      }
      wprintfcat(wtmp,wcount,L"%%%s", ud_reg_tab[op->index - UD_R_AL]);
    }
    if (op->scale) {
      wprintfcat(wtmp,wcount,L",%d", op->scale);
    }
    if (op->base || op->index) {
      wprintfcat(wtmp,wcount,L")");
    }
    break;

  case UD_OP_IMM:
    wprintfcat(wtmp,wcount,L"$");
    ud_syn_print_imm(u, op,wtmp,wcount);
    break;

  case UD_OP_JIMM:
    ud_syn_print_addr(u, ud_syn_rel_target(u, op),wtmp,wcount);
    break;

  case UD_OP_PTR:
    switch (op->size) {
      case 32:
        wprintfcat(wtmp,wcount,L"$0x%x, $0x%x", op->lval.ptr.seg,op->lval.ptr.off & 0xFFFF);
        break;
      case 48:
        wprintfcat(wtmp,wcount,L"$0x%x, $0x%x", op->lval.ptr.seg,op->lval.ptr.off);
        break;
    }
    break;
      
  default: return;
  }
}

/* =============================================================================
 * translates to AT&T syntax 
 * =============================================================================
 */
extern void 
ud_translate_att(struct ud *u)
{
  int size = 0;
  int star = 0;
  wchar_t stmp[128] = {0};

  /* check if P_OSO prefix is used */
  if (! P_OSO(u->itab_entry->prefix) && u->pfx_opr) {
  switch (u->dis_mode) {
    case 16: 
      wprintfcat(stmp,128,L"o32 ");
      break;
    case 32:
    case 64:
      wprintfcat(stmp,128,L"o16 ");
      break;
  }
  }

  /* check if P_ASO prefix was used */
  if (! P_ASO(u->itab_entry->prefix) && u->pfx_adr) {
  switch (u->dis_mode) {
    case 16: 
      wprintfcat(stmp,128,L"a32 ");
      break;
    case 32:
      wprintfcat(stmp,128,L"a16 ");
      break;
    case 64:
      wprintfcat(stmp,128,L"a32 ");
      break;
  }
  }

  if (u->pfx_lock)
    wprintfcat(stmp,128,L"lock ");
  if (u->pfx_rep) {
    wprintfcat(stmp,128,L"rep ");
  } else if (u->pfx_repe) {
    wprintfcat(stmp,128,L"repe ");
  } else if (u->pfx_repne) {
    wprintfcat(stmp,128,L"repne ");
  }

  /* special instructions */
  switch (u->mnemonic) {
  case UD_Iretf: 
    wprintfcat(stmp,128,L"lret ");
    break;
  case UD_Idb:
    wprintfcat(stmp,128,L".byte 0x%x", u->operand[0].lval.ubyte);
    return;
  case UD_Ijmp:
  case UD_Icall:
    if (u->br_far)
      wprintfcat(stmp,128,L"l");
    if (u->operand[0].type == UD_OP_REG) {
      star = 1;
    }
    wprintfcat(stmp,128,L"%s", ud_lookup_mnemonic(u->mnemonic));
    break;
  case UD_Ibound:
  case UD_Ienter:
    if (u->operand[0].type != UD_NONE)
      gen_operand(u, &u->operand[0],stmp,128);
    if (u->operand[1].type != UD_NONE) {
      wprintfcat(stmp,128,L",");
      gen_operand(u, &u->operand[1],stmp,128);
    }
    return;
  default:
    wprintfcat(stmp,128,L"%s", ud_lookup_mnemonic(u->mnemonic));
  }

  if (size == 8)
    wprintfcat(stmp,128,L"b");
  else if (size == 16)
    wprintfcat(stmp,128,L"w");
  else if (size == 64)
    wprintfcat(stmp,128,L"q");

  if (star) {
    wprintfcat(stmp,128,L" *");
  } else {
    wprintfcat(stmp,128,L" ");
  }

  if (u->operand[2].type != UD_NONE) {
    gen_operand(u, &u->operand[2],stmp,128);
    wprintfcat(stmp,128,L", ");
  }

  if (u->operand[1].type != UD_NONE) {
    gen_operand(u, &u->operand[1],stmp,128);
    wprintfcat(stmp,128,L", ");
  }

  if (u->operand[0].type != UD_NONE)
    gen_operand(u, &u->operand[0],stmp,128);
}

/*
vim: set ts=2 sw=2 expandtab
*/
