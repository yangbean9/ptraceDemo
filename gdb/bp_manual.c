/* Code sample: manual setting of a breakpoint, using ptrace
**
** Eli Bendersky (http://eli.thegreenplace.net)
** This code is in the public domain.
*/
#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <string.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>


extern char* strsignal(int);

/* Run a target process in tracing mode by exec()-ing the given program name.
*/
void run_target(const char* programname)
{
	printf("target started. will run '%s'\n", programname);

    /* Allow tracing of this process */
    if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
        perror("ptrace");
        return;
    }

    /* Replace this process's image with the given program */
    execl(programname, programname, 0);
}
void run_debugger(pid_t child_pid)
{
    int wait_status;
    struct user_regs_struct regs;

    /* 等待子进程信号 */
    wait(&wait_status);

    /* 取子进程寄存器 */
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
    /* eip是子进程执行的指令地址 */
    printf("Child started. EIP = 0x%08x\n", regs.eip);

    /* 读取地址0x804808a处 对应的指令*/
    unsigned addr = 0x804808a;
    unsigned data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)addr, 0);
    printf("Original data at 0x%08x: 0x%08x\n", addr, data);

    /* 把'int 3'替换进 0x804808a处 对应的指令*/
    unsigned data_with_trap = (data & 0xFFFFFF00) | 0xCC;
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr, (void*)data_with_trap);

    /* 再读 地址0x804808a处 对应的指令*/
    unsigned readback_data = ptrace(PTRACE_PEEKTEXT, child_pid, (void*)addr, 0);
    printf("After trap, data at 0x%08x: 0x%08x\n", addr, readback_data);

    /* 让子进程运行直到遇到断点 int 3*/
    ptrace(PTRACE_CONT, child_pid, 0, 0);

    wait(&wait_status);
	if (WIFSTOPPED(wait_status)) {
		printf("Child got a signal: %s\n", strsignal(WSTOPSIG(wait_status)));
	} else {
		perror("wait");
		return;
	}

    /* 看看子进程目前执行到哪了 */
    ptrace(PTRACE_GETREGS, child_pid, 0, &regs);
    printf("Child stopped at EIP = 0x%08x\n", regs.eip);
    /*等 用户响应*/
    getchar();

    /* 移除int 3,把原先的指令写回去*/
    ptrace(PTRACE_POKETEXT, child_pid, (void*)addr, (void*)data);
    /* 指令地址 -1*/
    regs.eip -= 1;
    ptrace(PTRACE_SETREGS, child_pid, 0, &regs);

    ptrace(PTRACE_CONT, child_pid, 0, 0);

    wait(&wait_status);

    if (WIFEXITED(wait_status)) {
    	printf("Child exited\n");
    }
    else {
    	printf("Unexpected signal\n");
    }
}


int main(int argc, char** argv)
{
    pid_t child_pid;

    if (argc < 2) {
        fprintf(stderr, "Expected a program name as argument\n");
        return -1;
    }

    child_pid = fork();
    if (child_pid == 0)
        run_target(argv[1]);
    else if (child_pid > 0)
        run_debugger(child_pid);
    else {
        perror("fork");
        return -1;
    }

    return 0;
}
