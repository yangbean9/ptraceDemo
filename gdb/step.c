#include <stdio.h>
#include <stdarg.h>
#include <stdlib.h>
#include <signal.h>
#include <syscall.h>
#include <sys/ptrace.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <sys/reg.h>
#include <sys/user.h>
#include <unistd.h>
#include <errno.h>

/**
 * 演示PTRACE_SINGLESTEP
 * */
int main(int argc, char** argv) {
	if (argc < 2) {
		perror("set a progarm to exec");
		return -1;
	}
	pid_t child_pid = fork();
	if (child_pid == 0) {
		//子进程执行
		if (ptrace(PTRACE_TRACEME, 0, 0, 0) < 0) {
			perror("ptrace_traceme error");
			return -1;
		}
		execl(argv[1], argv[1], 0);
	} else if (child_pid > 0) {
		//父进程执行
		int counter = 0;
		int status;
		while (1) {
			//等待子进程信号
			wait(&status);
			if (WIFEXITED(status)) //子进程发送退出信号，退出循环
				break;
			counter++;
			//调用ptrace从子进程取数据
			struct user_regs_struct regs;
			//取eip寄存器，这里存放的是cpu将要执行的指令地址
			ptrace(PTRACE_GETREGS, child_pid, NULL, &regs);
			//取指令内容
			unsigned instr = ptrace(PTRACE_PEEKTEXT, child_pid, regs.eip, 0);
			printf("counter = %u,EIP = 0x%08x,instr = 0x%08x\n", counter,
					regs.eip, instr);
			//重新启动子进程，当子进程执行了下一条指令后再将其停止
			if (ptrace(PTRACE_SINGLESTEP, child_pid, 0, 0) < 0) {
				perror("ptrace singlestep error");
				return -1;
			}
		}
	} else {
		perror("fork error");
		return -1;
	}

	return 0;
}
