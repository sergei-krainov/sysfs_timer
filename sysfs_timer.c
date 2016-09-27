#include <linux/module.h>
#include <linux/init.h>
#include <linux/workqueue.h>
#include <linux/delay.h>

static int tv = 30;
static struct kobject *timer_object;

static ssize_t tv_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t tv_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);

static struct kobj_attribute tv_attribute = __ATTR(tv, 0660, tv_show, tv_store);

static struct workqueue_struct *my_wq;
static struct work_struct my_work;

static void timer_routine(struct work_struct *work)
{
	do {
		printk(KERN_INFO "Tick Tick\n");
		msleep(tv * 1000);
	} while (tv != 0);
}

static ssize_t tv_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf)
{
	return sprintf(buf, "%d\n", tv);
}

static ssize_t tv_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count)
{
	sscanf(buf, "%du", &tv);
	return count;
}

static int __init timer_init(void)
{
	int error = 0;
	printk(KERN_INFO "Module initialized successfully\n");
	
	timer_object = kobject_create_and_add("timer_object", kernel_kobj);
	if(!timer_object)
		return -ENOMEM;
	
	error = sysfs_create_file(timer_object, &tv_attribute.attr);
	if (error) {
		printk(KERN_INFO "failed to create the tv file in /sys/kernel/timer_object \n");
	}
	else {
		printk(KERN_INFO "Created tv file in /sys/kernel/timer_object \n");
	}
	
	my_wq = create_workqueue("CWQ");
	
	INIT_WORK(&my_work, timer_routine);
	queue_work(my_wq, &my_work);

	return error;
}

static void __exit timer_exit(void)
{
	tv = 0;
	kobject_put(timer_object);
	destroy_workqueue(my_wq);
	printk(KERN_INFO "Module uninitialized successfully \n");
}

module_init(timer_init);
module_exit(timer_exit);
MODULE_LICENSE("GPL");
