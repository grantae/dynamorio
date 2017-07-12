/* **********************************************************
 * Copyright (c) 2017 Google, Inc.  All rights reserved.
 * **********************************************************/

/*
 * Redistribution and use in source and binary forms, with or without
 * modification, are permitted provided that the following conditions are met:
 *
 * * Redistributions of source code must retain the above copyright notice,
 *   this list of conditions and the following disclaimer.
 *
 * * Redistributions in binary form must reproduce the above copyright notice,
 *   this list of conditions and the following disclaimer in the documentation
 *   and/or other materials provided with the distribution.
 *
 * * Neither the name of VMware, Inc. nor the names of its contributors may be
 *   used to endorse or promote products derived from this software without
 *   specific prior written permission.
 *
 * THIS SOFTWARE IS PROVIDED BY THE COPYRIGHT HOLDERS AND CONTRIBUTORS "AS IS"
 * AND ANY EXPRESS OR IMPLIED WARRANTIES, INCLUDING, BUT NOT LIMITED TO, THE
 * IMPLIED WARRANTIES OF MERCHANTABILITY AND FITNESS FOR A PARTICULAR PURPOSE
 * ARE DISCLAIMED. IN NO EVENT SHALL VMWARE, INC. OR CONTRIBUTORS BE LIABLE
 * FOR ANY DIRECT, INDIRECT, INCIDENTAL, SPECIAL, EXEMPLARY, OR CONSEQUENTIAL
 * DAMAGES (INCLUDING, BUT NOT LIMITED TO, PROCUREMENT OF SUBSTITUTE GOODS OR
 * SERVICES; LOSS OF USE, DATA, OR PROFITS; OR BUSINESS INTERRUPTION) HOWEVER
 * CAUSED AND ON ANY THEORY OF LIABILITY, WHETHER IN CONTRACT, STRICT
 * LIABILITY, OR TORT (INCLUDING NEGLIGENCE OR OTHERWISE) ARISING IN ANY WAY
 * OUT OF THE USE OF THIS SOFTWARE, EVEN IF ADVISED OF THE POSSIBILITY OF SUCH
 * DAMAGE.
 */

#include "dr_api.h"

#define ASSERT(x) \
    ((void)((!(x)) ? \
        (dr_fprintf(STDERR, "ASSERT FAILURE: %s:%d: %s\n", __FILE__,  __LINE__, #x),\
         dr_abort(), 0) : 0))

void
test_dr_insert_it_instrs_cbr(void *dcontext)
{
    instrlist_t *ilist = instrlist_create(dcontext);
    instr_t *where = INSTR_CREATE_label(dcontext);

    instr_t *instr_it1, *instr_it2, *instr_it3;
    byte buffer[4096];

    instrlist_append(ilist, where);
    instrlist_preinsert(ilist, where, XINST_CREATE_move
            (dcontext,
             opnd_create_reg(DR_REG_R1),
             opnd_create_reg(DR_REG_R2)));
    instrlist_preinsert(ilist, where, XINST_CREATE_jump
            (dcontext,
             opnd_create_instr(where)));
    instrlist_preinsert(ilist, where, XINST_CREATE_move
            (dcontext,
             opnd_create_reg(DR_REG_R1),
             opnd_create_reg(DR_REG_R2)));
    instrlist_preinsert(ilist, where, XINST_CREATE_move
            (dcontext,
             opnd_create_reg(DR_REG_R1),
             opnd_create_reg(DR_REG_R2)));
    instrlist_preinsert(ilist, where, XINST_CREATE_jump
            (dcontext,
             opnd_create_instr(where)));
    instrlist_preinsert(ilist, where, XINST_CREATE_move
            (dcontext,
             opnd_create_reg(DR_REG_R1),
             opnd_create_reg(DR_REG_R2)));
    instrlist_preinsert(ilist, where, XINST_CREATE_move
            (dcontext,
             opnd_create_reg(DR_REG_R1),
             opnd_create_reg(DR_REG_R2)));
    instrlist_preinsert(ilist, where, XINST_CREATE_move
            (dcontext,
             opnd_create_reg(DR_REG_R1),
             opnd_create_reg(DR_REG_R2)));
    instrlist_preinsert(ilist, where, XINST_CREATE_jump
            (dcontext,
             opnd_create_instr(where)));
    /* set them all to be predicated and reinstate it instrs */
    for (where = instrlist_first(ilist); where; where = instr_get_next(where)) {
        bool ok = instr_set_isa_mode(where, DR_ISA_ARM_THUMB);
        ASSERT(ok);
        instr_set_predicate(where, DR_PRED_LS);
    }
    dr_insert_it_instrs(dcontext, ilist);

    /* make sure it was encoded properly
     *  itt    #9, #12
     *  mov.ls r1, r2
     *  b.ls   @0x47366864
     *  ittt   #9, #14
     *  mov.ls r1, r2
     *  mov.ls r1, r2
     *  b.ls   @0x47366864
     *  itttt  #9, #15
     *  mov.ls r1, r2
     *  mov.ls r1, r2
     *  mov.ls r1, r2
     *  b.ls   @0x47366864
     */
    instr_it1 = instrlist_first(ilist);
    instr_it2 = instr_get_next(instr_get_next(instr_get_next(instr_it1)));
    instr_it3 = instr_get_next(instr_get_next(instr_get_next(instr_get_next(instr_it2))));
    ASSERT(instr_get_opcode(instr_it1) == OP_it);
    ASSERT(instr_get_opcode(instr_it2) == OP_it);
    ASSERT(instr_get_opcode(instr_it3) == OP_it);

    instrlist_encode(dcontext, ilist, buffer, true);
}

int
main(int argc, char *argv[])
{
    void *dcontext = dr_standalone_init();

    /* i#1702: test that a cbr is correctly made to end an IT-block */
    test_dr_insert_it_instrs_cbr(dcontext);
    return 0;
}
