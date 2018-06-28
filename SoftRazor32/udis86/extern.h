/* udis86 - udis86/extern.h
 *
 * Copyright (c) 2002-2009, 2013 Vivek Thampi
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

#ifndef UD_EXTERN_H
#define UD_EXTERN_H

#ifdef __cplusplus
extern "C" {
#endif

#include "udisconfig.h"
#include "types.h"


/* ============================= 公共 API ================================= */

extern void ud_init(struct ud *pud_t);    //初始化 pud_t 对象

extern void ud_set_mode(struct ud *pud_t, uint8_t u1b);   //设置反汇编模式.

extern void ud_set_pc(struct ud *pud_t, uint64_t u8b);    //设置代码来源. 

extern void ud_set_input_hook(struct ud *pud_t, func_inp_hook pfunc_hook); //设置输入钩子

extern void ud_set_input_buffer(struct ud *pud_t, const uint8_t *pu1b, size_t clen);

#ifndef __UD_STANDALONE__
extern void ud_set_input_file(struct ud *pud_t, FILE *phfile);
#endif /* __UD_STANDALONE__ */

extern void ud_set_vendor(struct ud *pud_t, unsigned ui);

extern void ud_set_syntax(struct ud *pud_t, func_translator pfunc_tran);

extern void ud_input_skip(struct ud *pud_t, size_t st);

extern int ud_input_end(const struct ud *pud_t);

extern unsigned int ud_get_count(struct ud *pud_t);

extern unsigned int ud_decode(struct ud *pud_t);

extern unsigned int ud_disassemble(struct ud *pud_t);

extern void ud_translate_intel(struct ud *pud_t);

extern void ud_translate_att(struct ud *pud_t);

extern const wchar_t* ud_insn_asm(const struct ud *pud_t);

extern const wchar_t* ud_insn_casm(const struct ud *pud_t);

extern const uint8_t* ud_insn_ptr(const struct ud *pud_tu);

extern uint64_t ud_insn_off(const struct ud *pud_t);

extern const wchar_t* ud_insn_hex(struct ud *pud_t);

extern unsigned int ud_insn_len(const struct ud *pud_t);

extern const struct ud_operand* ud_insn_opr(const struct ud *pud_t, unsigned int n);

extern int ud_opr_is_sreg(const struct ud_operand *opr);

extern int ud_opr_is_gpr(const struct ud_operand *opr);

extern enum ud_mnemonic_code ud_insn_mnemonic(const struct ud *pud_t);

extern const wchar_t* ud_lookup_mnemonic(enum ud_mnemonic_code c);

extern void ud_set_user_opaque_data(struct ud *pud_t, void *pany);

extern void* ud_get_user_opaque_data(const struct ud *pud_t);

extern uint64_t ud_insn_sext_imm(const struct ud *pud_t, const struct ud_operand *pudo);

extern void ud_set_asm_buffer(struct ud *pud_t, wchar_t *buf, size_t size);

extern void ud_enable_casm(struct ud *pud_t,int bl);

extern void ud_set_casm_buffer(struct ud *pud_t, wchar_t *buf, size_t size);

extern void ud_set_sym_resolver(struct ud *pud_t, func_resolver pfunc_reso);

/* ========================================================================== */

#ifdef __cplusplus
}
#endif
#endif /* UD_EXTERN_H */
