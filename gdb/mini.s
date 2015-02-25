.section .data
output:.string "hello,mini\n"

.section .text
.globl _start

_start:

# ssize_t write(int fd, const void *buf, size_t count);

movl $4, %eax
movl $1, %ebx
movl $output, %ecx
movl $11, %edx
int $0x80

movl $1, %eax
int $0x80
