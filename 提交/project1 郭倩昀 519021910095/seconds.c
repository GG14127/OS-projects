#include <linux/init.h>
#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/proc_fs.h>
#include <linux/sched.h>
#include <linux/uaccess.h>
#include <linux/jiffies.h>/* jiffies*/
#include <asm/param.h> /* HZ*/

#define BUFFER_SIZE 128
#define PROC_NAME "seconds"


unsigned long int volatile jiffies1,jiffies2;
const int hz=HZ;

ssize_t proc_read(struct file *file,char __user *usr_buf,size_t count,loff_t *pos);


static struct file_operations proc_ops ={
.owner = THIS_MODULE,
.read = proc_read,   
};

/* when the module loaded*/
int proc_init(void)
{
    proc_create(PROC_NAME,0666,NULL,&proc_ops);
    jiffies1 = jiffies;  
    printk(KERN_INFO "/proc/%s created\n", PROC_NAME);
    return 0;
}

void proc_exit(void)
{
    remove_proc_entry(PROC_NAME,NULL);
    printk( KERN_INFO "/proc/%s removed\n", PROC_NAME);
}

/*Implemention of proc_read*/
ssize_t proc_read(struct file *file, char __user *usr_buf, size_t count, loff_t *pos)
{
	jiffies2 = jiffies;  
    int rv=0;
    char buffer[BUFFER_SIZE];
    static int completed=0;
    if (completed){
      completed=0;
      return 0; 
    }
    completed=1;
    rv=sprintf(buffer, "The running time is %d s\n", ((jiffies2-jiffies1)/hz));
 
    copy_to_user(usr_buf,buffer,rv);
    return rv;
}
module_init(proc_init);
module_exit(proc_exit);

MODULE_LICENSE("GPL");
MODULE_DESCRIPTION("Show the program execution time");
MODULE_AUTHOR("GQY");

