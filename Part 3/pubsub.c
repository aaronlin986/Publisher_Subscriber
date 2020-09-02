#include <linux/module.h>
#include <net/sock.h> 
#include <linux/netlink.h>
#include <linux/skbuff.h> 
#define NETLINK_USER 31

struct node {
	int tid;
	struct list_head listItem;
};

LIST_HEAD(myLinked);
struct sock *nl_sk = NULL;
char *buffer;
struct node *curr;

static void hello_nl_recv_msg(struct sk_buff *skb)
{
    	struct nlmsghdr *nlh;
	struct sk_buff *skb_out;
	int msg_size;
	char *msg;
	char *registerMsg = "Registered!";
	int res;
	int selector = 1;

	struct node *newItem = kmalloc(sizeof(struct node), GFP_KERNEL);
	INIT_LIST_HEAD(&newItem->listItem);

	printk(KERN_INFO "Entering: %s\n", __FUNCTION__);

    	nlh = (struct nlmsghdr *)skb->data;
	msg = nlmsg_data(nlh) + 1;
	msg_size = strlen(msg + 1);
	printk(KERN_INFO "Netlink received msg payload: %s\n", (char *)nlmsg_data(nlh));

	if(*(char *)nlmsg_data(nlh) == 'S'){
		selector = 0;
	}

	if(selector == 0){
		msg_size = strlen(registerMsg);
		msg = registerMsg;
		newItem->tid = nlh->nlmsg_pid;
		list_add(&newItem->listItem, &myLinked);
		
		skb_out = nlmsg_new(msg_size, 0);
		if (!skb_out) {
			printk(KERN_ERR "Failed to allocate new skb\n");
		      	return;
		}

		nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
		NETLINK_CB(skb_out).dst_group = 0; /* not in mcast group */
		strncpy(nlmsg_data(nlh), msg, msg_size);
		
		res = nlmsg_unicast(nl_sk, skb_out, newItem->tid);
		if (res < 0)
			printk(KERN_INFO "Error while sending bak to user\n");
	}

	if(selector == 1){
		curr = NULL;
		list_for_each_entry(curr, &myLinked, listItem){
			skb_out = nlmsg_new(msg_size, 0);
			if (!skb_out) {
				printk(KERN_ERR "Failed to allocate new skb\n");
			      	return;
			}

			nlh = nlmsg_put(skb_out, 0, 0, NLMSG_DONE, msg_size, 0);
			NETLINK_CB(skb_out).dst_group = 0; 
			strncpy(nlmsg_data(nlh), msg, msg_size);
			res = nlmsg_unicast(nl_sk, skb_out, curr->tid);
			if (res < 0)
				printk(KERN_INFO "Error while sending bak to user\n");
		}
	}
}

static int __init hello_init(void)
{
    printk("Entering: %s\n", __FUNCTION__);
    struct netlink_kernel_cfg cfg = {
        .input = hello_nl_recv_msg,
    };

    nl_sk = netlink_kernel_create(&init_net, NETLINK_USER, &cfg);
    if (!nl_sk) {
        printk(KERN_ALERT "Error creating socket.\n");
        return -10;
    }

    return 0;
}

static void __exit hello_exit(void)
{
	struct node *temp = NULL;
	curr = NULL;
	list_for_each_entry_safe(curr, temp, &myLinked, listItem){
		list_del(&curr->listItem);
		kfree(curr);
	}

    printk(KERN_INFO "exiting hello module\n");
    netlink_kernel_release(nl_sk);
}

module_init(hello_init); module_exit(hello_exit);

MODULE_LICENSE("GPL");
