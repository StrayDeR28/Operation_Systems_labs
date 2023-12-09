#include <linux/kernel.h>
#include <linux/module.h>
#include <linux/printk.h>
#include <linux/proc_fs.h>
#include <linux/uaccess.h>
#include <linux/version.h>
#include <linux/init.h>
#include <linux/seq_file.h>
#include <linux/jiffies.h>

#define procfs_name "tsulab"

static struct proc_dir_entry *our_proc_file = NULL;
static unsigned long last_access_time;

static ssize_t procfile_read(struct file *file_pointer, char __user *buffer, size_t buffer_length, loff_t* offset)
{
    unsigned long current_time = jiffies;
    char s[64];  
    ssize_t ret;
    
    if (current_time - last_access_time > 10 * HZ) { ret = sprintf(s, "1 File accessed more than 10 seconds ago\n");} 
    else { ret = sprintf(s, "0 File accessed less than 10 seconds ago\n");}
    last_access_time = current_time;
    
    if (*offset >= ret || copy_to_user(buffer, s, ret)) { ret = 0; } 
    else { *offset += ret; }
    return ret;
}
#if LINUX_VERSION_CODE >= KERNEL_VERSION(5, 6, 0)
static const struct proc_ops proc_file_fops = 
{
    .proc_read = procfile_read,
};
#else
static const struct file_operations proc_file_fops = 
{
    .read = procfile_read,
};
#endif

static int __init procfs1_init(void) 
{
    our_proc_file = proc_create(procfs_name, 0644, NULL, &proc_file_fops);
    pr_info("proc %s was created\n", procfs_name);
    return 0;
}

static void __exit procfs1_exit(void) 
{
    proc_remove(our_proc_file);
    pr_info("proc %s was removed\n", procfs_name);
}

module_init(procfs1_init);
module_exit(procfs1_exit);
MODULE_LICENSE("GPL");
