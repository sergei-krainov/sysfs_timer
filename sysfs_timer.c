#include <linux/module.h>
#include <linux/init.h>
#include <linux/time.h>
#include <linux/rtc.h>
#include <linux/workqueue.h>
#include <linux/sched.h>

static struct kobject *timer_object;
static int tv = 60;
static struct timer_list exp_timer;
struct timespec ts;

static ssize_t tv_show(struct kobject *kobj, struct kobj_attribute *attr, char *buf);
static ssize_t tv_store(struct kobject *kobj, struct kobj_attribute *attr, const char *buf, size_t count);


static struct kobj_attribute tv_attribute = __ATTR(tv, 0660, tv_show, tv_store);
//static struct work_struct Task;
//static DECLARE_WORK(Task, &timer_routine);

static void timer_routine(struct work_struct *irrelevant)
{
	printk(KERN_INFO "Tick Tick\n");
}

static struct workqueue_struct *my_workqueue;
static struct work_struct Task;
//static struct work_struct *Task_ref;
//struct delayed_work *dwork = to_delayed_work(Task_ref);
struct delayed_work *dwork;
static DECLARE_WORK(Task, timer_routine);

static void write_info(unsigned long data)
{
		
	struct timeval time;
	unsigned long local_time;
	struct rtc_time tm;
	do_gettimeofday(&time);
	local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
	rtc_time_to_tm(local_time, &tm);
	printk(KERN_INFO "%02d:%02d:%02d\n", tm.tm_hour, tm.tm_min, tm.tm_sec);
	
        printk(KERN_INFO "Timer's tick, %d\n", tv);
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
	struct timeval time;
	unsigned long local_time;
	struct rtc_time tm;
	printk(KERN_INFO "Module initialized successfully\n");
	
	timer_object = kobject_create_and_add("timer_object", kernel_kobj);
	if(!timer_object)
		return -ENOMEM;
	
	error = sysfs_create_file(timer_object, &tv_attribute.attr);
	if (error) {
		printk(KERN_INFO "failed to create the tv file in /sys/kernel/timer_object \n");
	}
	
	init_timer_on_stack(&exp_timer);
	
	exp_timer.expires = jiffies + tv * HZ;
	exp_timer.data = 0;
	exp_timer.function = write_info;
	
	add_timer(&exp_timer);	

	do_gettimeofday(&time);
	local_time = (u32)(time.tv_sec - (sys_tz.tz_minuteswest * 60));
	rtc_time_to_tm(local_time, &tm);

	printk(KERN_INFO " @ (%04d-%02d-%02d %02d:%02d:%02d)\n", tm.tm_year + 1900, tm.tm_mon + 1, tm.tm_mday, tm.tm_hour, tm.tm_min, tm.tm_sec);
	
	printk(KERN_INFO "%02d:%02d:%02d == %lu\n", tm.tm_hour, tm.tm_min, tm.tm_sec, jiffies);
	
	//my_workqueue = create_workqueue("CSched.c");
	//queue_delayed_work(my_workqueue, dwork, tv * HZ);
	//INIT_DELAYED_WORK(&work, disk_events_workfn);
	dwork = to_delayed_work(&Task);
	INIT_DELAYED_WORK(dwork, timer_routine);

	return error;
}

static void __exit timer_exit(void)
{
	kobject_put(timer_object);
	del_timer(&exp_timer);
	printk(KERN_INFO "Module uninitialized successfully \n");
}

module_init(timer_init);
module_exit(timer_exit);
MODULE_LICENSE("GPL");
