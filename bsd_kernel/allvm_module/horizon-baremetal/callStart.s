;
; Horizon bytecode compiler
; Copyright (C) 2010 James Molloy
;
; Horizon is open source software, released under the terms of the Non-Profit
; Open Software License 3.0. You should have received a copy of the
; licensing information along with the source code distribution. If you
; have not received a copy of the license, please refer to the Horizon
; project website.
;
; Please note that if you modify this file, the license requires you to
; ADD your name to the list of contributors. This boilerplate is not the
; license itself; please refer to the copy of the license you have received
; for complete terms.
;

;;; Taken from the glibc source:

;;; This is the canonical entry point, usually the first thing in the text
;;; segment.  The SVR4/i386 ABI (pages 3-31, 3-32) says that when the entry
;;; point runs, most registers' values are unspecified, except for:

;;; %rdx         Contains a function pointer to be registered with `atexit'.
;;;              This is how the dynamic linker arranges to have DT_FINI
;;;              functions called for shared libraries that have been loaded
;;;              before this code runs.

;;; %rsp         The stack contains the arguments and environment:
;;;              0(%rsp)                 argc
;;;              8(%rsp)                 argv[0]
;;;              ...
;;;              (8*argc)(%rsp)          NULL
;;;              (8*(argc+1))(%rsp)      envp[0]
;;;              ...
;;;                                      NULL

global CallStart
extern _start

CallStart:
        ;; Store a dummy function into rdx as the atexit() function.
        mov rdx, dummy_atexit

        ;; Construct the stack frame: NULL for env, NULL for argv and 0 for argc.
        ;; After env comes the aux vector, which MUST contain an AT_PAGESZ entry (we set the page size to 0x1000),
        ;; and must terminate with a NULL entry.

        ;; auxv NULL terminator
	push 0
	push 0
	;; auxv page size entry, AT_PAGESZ == 6, page size == 4KB
	push 0x1000
	push 6
        ;; env NULL terminator
        push 0
        ;; argv NULL terminator
        push 0
        ;; argc
        push 0
        
        jmp _start

dummy_atexit:
        ;; TMP so we know if this fires.
        int 0xfe
        ret
        
