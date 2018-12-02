// http://cs.smith.edu/~nhowe/262/labs/src/hellotty.c
//
// https://stackoverflow.com/questions/20719713/how-can-i-write-to-tty-from-a-kernel-module
// use tty = get_current_tty(); instead of tty = current->signal->tty;
// linux-4.18.5/drivers/tty/tty_jobctrl.c:156:struct tty_struct *get_current_tty(void)


/* hellotty.c
 *
 * "Hello, world" - the loadable kernel module version.
 * This time, print to the terminal that executed the command.
 *
 * Compile this with
 *
 * $ gcc -c hellotty.c -Wall -nostdinc -I /usr/src/linux/include -I /usr/lib/gcc/i586-mandrake-linux-gnu/3.4.1/include/
 *
 * Once compiled, it must be converted to a kernel object (.ko) file
 * using the linux kernel makefiles, and inserted into the kernel.
 * If you have your Makefile set up properly, that should look like this
 *
 * $ sudo make -C /usr/src/linux-`uname -r` SUBDIRS=$PWD modules
 * $ sudo insmod hellotty.ko
 *
 * Your Makefile must include this line first:
 *
 * obj-m += hellotty.o
 *
 */

/* Declare what kind of code we want from the header files */
#define __KERNEL__         /* We're part of the kernel */
#define MODULE             /* Not a permanent part, though. */
/* If the lines above were not included, we could use compiler flags instead:
 * gcc -c hellotty.c -Wall -nostdinc -I /usr/src/linux/include -I /usr/lib/gcc/i586-mandrake-linux-gnu/3.4.1/include/ -D MODULE -D __KERNEL__
 */

/* Standard headers */
#include <linux/module.h>       /* Needed by all LKMs */
#include <linux/kernel.h>       /* Needed for KERN_ALERT */
#include <linux/init.h>         /* Needed for the macros */
#include <linux/sched.h>        /* For current */
#include <linux/tty.h>          /* For the tty declarations */

/* Documentation macros.  These are not used by the kernel, but it
 * may complain if they are not provided. */

MODULE_AUTHOR("Nicholas R. Howe");
MODULE_DESCRIPTION("Simple Kernel Module");
MODULE_LICENSE("GPL");

/* This function locates the tty device of the current task and
 * prints a message using its print function.
 * This will work only on Linux kernel 2.6.6 and higher.
 * (See http://www.tldp.org/LDP/lkmpg/2.6/html/x1143.html)
 */
static void print_string(char *str)
{
        struct tty_struct *my_tty;

        my_tty = current->signal->tty;

        /*
         * If my_tty is NULL, the current task has no tty you can print to
         * (ie, if it's a daemon).  If so, there's nothing we can do.
         */
        if (my_tty != NULL) {

                /*
                 * my_tty->driver is a struct which holds the tty's functions,
                 * one of which (write) is used to write strings to the tty.
                 * It can be used to take a string either from the user's or
                 * kernel's memory segment.
                 *
                 * The function's 1st parameter is the tty to write to,
                 * because the same function would normally be used for all
                 * tty's of a certain type.  The 2nd parameter controls
                 * whether the function receives a string from kernel
                 * memory (false, 0) or from user memory (true, non zero).
                 * BTW: this param has been removed in Kernels > 2.6.9
                 * The (2nd) 3rd parameter is a pointer to a string.
                 * The (3rd) 4th parameter is the length of the string.
                 */
                ((my_tty->driver)->write) (my_tty,0,str,strlen(str));

                /*
                 * ttys were originally hardware devices, which (usually)
                 * strictly followed the ASCII standard.  In ASCII, to move to
                 * a new line you need two characters, a carriage return and a
                 * line feed.  On Unix, the ASCII line feed is used for both
                 * purposes - so we can't just use \n, because it wouldn't have
                 * a carriage return and the next line will start at the
                 * column right after the line feed.
                 *
                 * This is why text files are different between Unix and
                 * MS Windows.  In CP/M and derivatives, like MS-DOS and
                 * MS Windows, the ASCII standard was strictly adhered to,
                 * and therefore a newline requirs both a LF and a CR.
                 */
                ((my_tty->driver)->write) (my_tty, 0, "\015\012", 2);
        }
}

/* Initialize the LKM */
static int __init hellotty_init_module(void)
{
  print_string("Hello, world - this is the kernel speaking.\n");

  /* If we return a non zero value, it means that
   * init_module failed and the LKM can't be loaded
   */
  return 0;
}


/* Cleanup - undo whatever init_module did */
static void __exit hellotty_exit_module(void)
{
  print_string("Short is the life of an LKM.\n");
}

/* The macros below register the init and exit functions with the kernel */
module_init(hellotty_init_module);
module_exit(hellotty_exit_module);
