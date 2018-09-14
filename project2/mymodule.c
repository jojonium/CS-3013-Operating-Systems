/* Joseph Petitti and Justin Cheng */

#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/syscalls.h>
#include <linux/sched.h>
//#include "/usr/src/linux-source-4.4.0/linux-source-4.4.0/include/asm-generic/current.h"
//#include "/usr/src/linux-source-4.4.0/linux-source-4.4.0/include/asm-generic/cputime.h"
//#include "/usr/src/linux-source-4.4.0/linux-source-4.4.0/include/asm-generic/uaccess.h"
#include <linux/list.h>
#include <linux/time.h>

unsigned long **sys_call_table;

asmlinkage long (*ref_sys_cs3013_syscall1)(void);
asmlinkage long (*ref_sys_open)(const char *filename, int flags, mode_t mode);
asmlinkage long (*ref_sys_close)(int filedes);
asmlinkage long (*ref_sys_cs3013_syscall2)(void);

struct processinfo {
	long state;
	pid_t pid;
	pid_t parent_pid;
	pid_t youngest_child;
	pid_t younger_sibling;
	pid_t older_sibling;
	uid_t uid;
	long long start_time;
	long long user_time;
	long long sys_time;
	long long cutime;
	long long cstime;
};

asmlinkage long new_sys_cs3013_syscall1(void) {
    printk(KERN_INFO "\"'Hello world?!' More like 'Goodbye, world!' EXTERMINATE!\" -- Dalek");
    return 0;
}

// new open system call
asmlinkage long new_sys_open(const char *filename, int flags, mode_t mode) {
	int uid;
	if ((uid = current_uid().val) > 999) {
		printk(KERN_INFO "User %d is opening the file: %s\n", uid, filename);
	}
	return ref_sys_open(filename, flags, mode); // call the original with the same arguments
}

// new close system call
asmlinkage long new_sys_close(int filedes) {
	printk(KERN_INFO "USER %d is closing file descriptor: %d\n", current_uid().val, filedes);
	return ref_sys_close(filedes);
}

// new cs3013_syscall2
asmlinkage long new_sys_cs3013_syscall2(struct processinfo *info) {
	struct processinfo out;
	struct task_struct *ts = current;
	struct task_struct *temp;
	struct list_head *pos;
	long long youngest_child_time;
	long long younger_sibling_time;
	long long older_sibling_time;

	printk(KERN_INFO "Started new_sys_cs3013_syscall2\n");

	out.state = ts->state;
	out.pid = ts->pid;
	out.parent_pid = ts->parent->pid;
	out.start_time = ts->start_time;

	if (list_empty(&(ts->children))) { // if no children
		out.youngest_child = -1;
		out.cutime = -1;
		out.cstime = -1;
	} else { // walk through list of children
		out.cutime = 0;
		out.cstime = 0;
		youngest_child_time = -1;
		list_for_each(pos, &(ts->children)) {
			temp = list_entry(pos, struct task_struct, sibling);
			out.cutime += cputime_to_usecs((temp->utime));
			out.cstime += cputime_to_usecs((temp->stime));

			// check if this is the youngest child we've seen
			if (temp->start_time < youngest_child_time || youngest_child_time == -1) {
				youngest_child_time = temp->start_time;
				out.youngest_child = temp->pid;
			}
		}
	}


	if (list_is_singular(&(ts->sibling))) {
		out.younger_sibling = -1;
		out.older_sibling = -1;
		younger_sibling_time = -1;
	} else {
		older_sibling_time = -1;
		younger_sibling_time = -1;

		list_for_each(pos, &(ts->sibling)) {
			temp = list_entry(pos, struct task_struct, sibling);
			
			// check if this is the oldest younger child we've seen
			if (temp->start_time > out.start_time && (temp->start_time < younger_sibling_time || younger_sibling_time == -1)) {
				younger_sibling_time = temp->start_time;
				out.younger_sibling = temp->pid;
			}

			// check if this is the youngest older child we've seen
			if (temp->start_time < out.start_time && (temp->start_time > older_sibling_time || older_sibling_time == -1)) {
				older_sibling_time = temp->start_time;
				out.older_sibling = temp->pid;
			}
		}
	}

	out.uid = current_uid().val;
	out.user_time = cputime_to_usecs(ts->utime);
	out.sys_time = cputime_to_usecs(ts->stime);

	if (copy_to_user(info, &out, sizeof(out)))
		return EFAULT;

	return 0;
}

static unsigned long **find_sys_call_table(void) {
	unsigned long int offset = PAGE_OFFSET;
	unsigned long **sct;
  
	while (offset < ULLONG_MAX) {
	  	sct = (unsigned long **)offset;

  		if (sct[__NR_close] == (unsigned long *) sys_close) {
    			printk(KERN_INFO "Interceptor: Found syscall table at address: 0x%02lX\n",
       		       	(unsigned long) sct);
    			return sct;
  		}
    
  	offset += sizeof(void *);
	}

	return NULL;
}

static void disable_page_protection(void) {
	/*
  	Control Register 0 (cr0) governs how the CPU operates.

  	Bit #16, if set, prevents the CPU from writing to memory marked as
  	read only. Well, our system call table meets that description.
  	But, we can simply turn off this bit in cr0 to allow us to make
  	changes. We read in the current value of the register (32 or 64
  	bits wide), and AND that with a value where all bits are 0 except
  	the 16th bit (using a negation operation), causing the write_cr0
  	value to have the 16th bit cleared (with all other bits staying
  	the same. We will thus be able to write to the protected memory.

  	It's good to be the kernel!
 	*/

	write_cr0 (read_cr0 () & (~ 0x10000));
}

static void enable_page_protection(void) {
	/*
 	See the above description for cr0. Here, we use an OR to set the 
 	16th bit to re-enable write protection on the CPU.
	*/
	write_cr0 (read_cr0 () | 0x10000);
}

static int __init interceptor_start(void) {
	/* Find the system call table */
	if(!(sys_call_table = find_sys_call_table())) {
  		/* Well, that didn't work. 
     		Cancel the module loading step. */
  		return -1;
	}
	
	/* Store a copy of all the existing functions */
	ref_sys_cs3013_syscall1 = (void *)sys_call_table[__NR_cs3013_syscall1];
	ref_sys_open = (void *)sys_call_table[__NR_open];
	ref_sys_close = (void *)sys_call_table[__NR_close];
	ref_sys_cs3013_syscall2 = (void *)sys_call_table[__NR_cs3013_syscall2];
	disable_page_protection();

	/* Replace the existing system calls */
	sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)new_sys_cs3013_syscall1;
	sys_call_table[__NR_open] = (unsigned long *)new_sys_open;
	sys_call_table[__NR_close] = (unsigned long *)new_sys_close;
	sys_call_table[__NR_cs3013_syscall2] = (unsigned long *)new_sys_cs3013_syscall2;

	enable_page_protection();
	
	/* And indicate the load was successful */
	printk(KERN_INFO "Loaded interceptor!\n");

	return 0;
}

static void __exit interceptor_end(void) {
	/* If we don't know what the syscall table is, don't bother. */
	if(!sys_call_table)
 		return;
	
	/* Revert all system calls to what they were before we began. */
	disable_page_protection();
	sys_call_table[__NR_cs3013_syscall1] = (unsigned long *)ref_sys_cs3013_syscall1;
	sys_call_table[__NR_open] = (unsigned long *)ref_sys_open;
	sys_call_table[__NR_close] = (unsigned long *)ref_sys_close;
	sys_call_table[__NR_close] = (unsigned long *)ref_sys_cs3013_syscall2;
	enable_page_protection();

	printk(KERN_INFO "Unloaded interceptor!\n");
}

MODULE_LICENSE("GPL");
module_init(interceptor_start);
module_exit(interceptor_end);
