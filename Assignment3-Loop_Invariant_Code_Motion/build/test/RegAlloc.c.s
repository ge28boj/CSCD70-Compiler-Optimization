	.text
	.file	"RegAlloc.c"
	.globl	def                             # -- Begin function def
	.p2align	4, 0x90
	.type	def,@function
def:                                    # @def
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movl	g(%rip), %ecx
	leal	1(%rcx), %eax
	movl	%eax, g(%rip)
	movl	%ecx, %eax
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end0:
	.size	def, .Lfunc_end0-def
	.cfi_endproc
                                        # -- End function
	.globl	use                             # -- Begin function use
	.p2align	4, 0x90
	.type	use,@function
use:                                    # @use
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	movl	%edi, %esi
	movl	$.L.str, %edi
	xorl	%eax, %eax
	callq	printf
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end1:
	.size	use, .Lfunc_end1-use
	.cfi_endproc
                                        # -- End function
	.globl	main                            # -- Begin function main
	.p2align	4, 0x90
	.type	main,@function
main:                                   # @main
	.cfi_startproc
# %bb.0:
	pushq	%rbp
	.cfi_def_cfa_offset 16
	.cfi_offset %rbp, -16
	movq	%rsp, %rbp
	.cfi_def_cfa_register %rbp
	pushq	%r14
	pushq	%rbx
	.cfi_offset %rbx, -32
	.cfi_offset %r14, -24
	callq	def
	movl	%eax, %r14d
	testl	%eax, %eax
	je	.LBB2_2
# %bb.1:
	callq	def
	movl	%eax, %ebx
	movl	%r14d, %edi
	callq	use
	addl	$1, %ebx
	jmp	.LBB2_3
.LBB2_2:
	callq	def
	movl	%eax, %ebx
	movl	%r14d, %edi
	callq	use
	addl	$2, %ebx
.LBB2_3:
	callq	def
	movl	%eax, %edi
	callq	use
	movl	%ebx, %edi
	callq	use
	xorl	%eax, %eax
	popq	%rbx
	popq	%r14
	popq	%rbp
	.cfi_def_cfa %rsp, 8
	retq
.Lfunc_end2:
	.size	main, .Lfunc_end2-main
	.cfi_endproc
                                        # -- End function
	.type	g,@object                       # @g
	.bss
	.globl	g
	.p2align	2
g:
	.long	0                               # 0x0
	.size	g, 4

	.type	.L.str,@object                  # @.str
	.section	.rodata.str1.1,"aMS",@progbits,1
.L.str:
	.asciz	"%d"
	.size	.L.str, 3

	.ident	"Ubuntu clang version 12.0.1-++20211029101322+fed41342a82f-1~exp1~20211029221816.4"
	.section	".note.GNU-stack","",@progbits
