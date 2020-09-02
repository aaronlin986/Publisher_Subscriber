#include <linux/netlink.h>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <sys/socket.h>
#include <unistd.h>
#include <pthread.h>

#define NETLINK_USER 31

#define MAX_PAYLOAD 1024 /* maximum payload size*/
//Publisher variables
struct sockaddr_nl src_addr, dest_addr;
struct nlmsghdr *nlh = NULL;
struct iovec iov;
int sock_fd;
struct msghdr msg;
char *buffer;
pthread_t pub, sub;

//Subscriber variables
struct sockaddr_nl src_addr2, dest_addr2;
struct nlmsghdr *nlh2 = NULL;
struct iovec iov2;
int sock_fd2;
struct msghdr msg2;

void *publisher(void *ptr){
	while (1) {                    
		 buffer = malloc(MAX_PAYLOAD);
		*buffer = 'P';
		buffer++;

		
		sock_fd = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
		if (sock_fd < 0)
			return -1;

		memset(&src_addr, 0, sizeof(src_addr));
		src_addr.nl_family = AF_NETLINK;
		src_addr.nl_pid = getpid(); 
		bind(sock_fd, (struct sockaddr *)&src_addr, sizeof(src_addr));	

		    memset(&dest_addr, 0, sizeof(dest_addr));
		    dest_addr.nl_family = AF_NETLINK;
		    dest_addr.nl_pid = 0; 
		    dest_addr.nl_groups = 0;

		    nlh = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
		    memset(nlh, 0, NLMSG_SPACE(MAX_PAYLOAD));
		    nlh->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
		    nlh->nlmsg_pid = getpid();
		    nlh->nlmsg_flags = 0;

			fgets(buffer, MAX_PAYLOAD, stdin);               
			strcpy(NLMSG_DATA(nlh), --buffer);

		    iov.iov_base = (void *)nlh;
		    iov.iov_len = nlh->nlmsg_len;
		    msg.msg_name = (void *)&dest_addr;
		    msg.msg_namelen = sizeof(dest_addr);
		    msg.msg_iov = &iov;
		    msg.msg_iovlen = 1;

		    printf("[Publisher Thread] : Sending message to kernel\n");
		    sendmsg(sock_fd, &msg, 0);
			free(nlh);
		free(buffer);
    }
	close(sock_fd);
}

void *subscriber(void *ptr){
		sock_fd2 = socket(PF_NETLINK, SOCK_RAW, NETLINK_USER);
	    if (sock_fd2 < 0)
		return -1;

	    memset(&src_addr2, 0, sizeof(src_addr2));
	    src_addr2.nl_family = AF_NETLINK;
	    src_addr2.nl_pid = pthread_self(); 

	    bind(sock_fd2, (struct sockaddr *)&src_addr2, sizeof(src_addr2));

	    memset(&dest_addr2, 0, sizeof(dest_addr2));
	    dest_addr2.nl_family = AF_NETLINK;
	    dest_addr2.nl_pid = 0; 
	    dest_addr2.nl_groups = 0; 

	    nlh2 = (struct nlmsghdr *)malloc(NLMSG_SPACE(MAX_PAYLOAD));
	    memset(nlh2, 0, NLMSG_SPACE(MAX_PAYLOAD));
	    nlh2->nlmsg_len = NLMSG_SPACE(MAX_PAYLOAD);
	    nlh2->nlmsg_pid = pthread_self();
	    nlh2->nlmsg_flags = 0;

	    strcpy(NLMSG_DATA(nlh2), "SRegister");

	    iov2.iov_base = (void *)nlh2;
	    iov2.iov_len = nlh2->nlmsg_len;
	    msg2.msg_name = (void *)&dest_addr2;
	    msg2.msg_namelen = sizeof(dest_addr2);
	    msg2.msg_iov = &iov2;
	    msg2.msg_iovlen = 1;

	    printf("Registering with Kernel...\n");
	    sendmsg(sock_fd2, &msg2, 0);

	    recvmsg(sock_fd2, &msg2, 0);
		printf("%s\n\n", NLMSG_DATA(nlh2));
		free(nlh2);

		while(1){
			memset(nlh2, 0, NLMSG_SPACE(MAX_PAYLOAD));
			printf("Fetching...\n");
			recvmsg(sock_fd2, &msg2, 0);
			printf("[Subscriber Thread] Message received from Kernel : %s\n\n", NLMSG_DATA(nlh2));
		}

	    close(sock_fd2);
}

int main()
{
	int i1, i2;
	i1 = pthread_create(&pub, NULL, &publisher, NULL);
	i2 = pthread_create(&sub, NULL, &subscriber, NULL);
	pthread_join(pub, NULL);
	pthread_join(sub, NULL);

}
