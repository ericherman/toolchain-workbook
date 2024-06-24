# SPDX-License-Identifier: LGPL-2.1-or-later
# Copyright (C) 2024 Eric Herman <eric@freesa.org>

# grep '^[0-9].*sys_exit\s*$' ~/src/linux/arch/x86/entry/syscalls/syscall_64.tbl
# 60	common	exit			sys_exit

.global _start

.section .text
_start:
	pop %rdi		# argc
	mov %rsp, %rsi		# argv

	call main

	mov %rax, %rdi		# Set the exit code from main's return value
	mov $60, %rax		# System call number for exit (sys_exit)
	syscall
