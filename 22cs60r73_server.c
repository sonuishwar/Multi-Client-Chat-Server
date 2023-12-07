/*We have just provided a skeleton code. Feel free to add/remove functiolaties as you like. The function definitions are not complete. Please provide appropriate parameters and return types to a function in case you decide to use it in your submission.*/

#include <stdio.h>
#include <stdlib.h>
#include <sys/types.h> 
#include <sys/socket.h>
#include <netinet/in.h>
#include <fcntl.h>
#include <unistd.h>
#include <sys/wait.h>
#include <sys/ipc.h>
#include <sys/sem.h>	/* Include this to use semaphores */
#include <time.h>
#include <string.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/shm.h>
#include <errno.h>
#include<sys/select.h>
#define mx_id 100
#define MAX_CONNECTIONS 11      //max client -1 limit
#define g_size 5      //max group limit
#define mx_groups 10     // max groups

#define P(s) semop(s, &pop, 1)  /* pop is the structure we pass for doing the P(s) operation */
#define V(s) semop(s, &vop, 1)  /* vop is the structure we pass for doing the V(s) operation */

/*Define all the desired data structures as asked in the question*/

int ran(int id){
    srand(time(0));
    int t = rand()%1000;
    if(t<100) t+=100;
    return t*100+id;
}
/*Function to handle ^C*/
void sigCHandler() 
{ 
	//WRITE YOUR CODE HERE
   signal(SIGINT, sigCHandler);
} 

/*Function to handle ^Z*/
void sigZhandler() 
{ 
	//WRITE YOUR CODE HERE
    signal(SIGTSTP, sigZhandler);
    // printf("Cannot execute Ctrl+Z");
}

/*Function to handle errors*/
void error(char *msg)
{
	//WRITE YOUR CODE HERE
    printf("%s", msg);
    exit(0);
}


int fid[mx_id];
int count=0;
int all_connections[MAX_CONNECTIONS];

typedef struct gschema{
int admin[g_size],mem[g_size],invited[g_size],adreq[g_size], size,broad;
}gschema;

gschema groups[mx_groups];

void printg(int gid){
    for(int j=0; j<g_size; j++){
        printf("%d  %d  %d\n",groups[gid].admin[j],groups[gid].mem[j],groups[gid].invited[j]);
    }
}

void gwp(int id, char *line){
    int i=0;
    while(groups[i].size!=0) i++;
    for(int j=0; j<g_size; j++){
        groups[i].admin[j]=-1;
        groups[i].invited[j]=-1;
        groups[i].mem[j]=-1;
        groups[i].adreq[i]=-1;
    }
    groups[i].admin[0]=fid[id];
    groups[i].mem[0]=fid[id];
    int flag=1,icount=1;
    char *token=NULL;
    token=strtok(line," ");
    while(token!=NULL){
        if(icount==g_size){
            write(id, "group limit exceeded",20);
            flag=0;
            break;
        }
        int gmember = atoi(token);
        if(fid[gmember%100]<0){
            write(id, "offline member can't be included",32);
            flag=0;
            break;
        }
        groups[i].mem[icount++] = gmember;
        // groups[i].size++;
        token=strtok(NULL," ");
    }
    if(flag==0){
        groups[i].size=0;
        return;
    } 
    groups[i].size=icount;
    char msg[256];
    sprintf(msg,"Group created successfully with id %d",i);
    write(id, msg, sizeof(msg));
    sprintf(msg,"You are added in group id %d ",i);
    for(int j=1;j<g_size; j++){
        int t=groups[i].mem[j];
        if(t<0) continue;
        write(t%100,msg,sizeof(msg));
    }
}


void gp(int id, char* line){
    int i=0;
    while(groups[i].size!=0) i++;
    for(int j=0; j<g_size; j++){
        groups[i].admin[j]=-1;
        groups[i].invited[j]=-1;
        groups[i].mem[j]=-1;
        groups[i].adreq[i]=-1;
    }
    groups[i].admin[0]=fid[id];
    groups[i].mem[0]=fid[id];
    int flag=1,icount=1;
    char *token=NULL;
    token=strtok(line," ");
    while(token!=NULL){
        if(icount==g_size){
            write(id, "group limit exceeded",20);
            flag=0;
            break;
        }
        int gmember = atoi(token);
        if(fid[gmember%100]<0){
            write(id, "offline member can't be included",32);
            flag=0;
            break;
        }
        groups[i].invited[icount++] = gmember;
        token=strtok(NULL," ");
    }
    if(flag==0){
        groups[i].size=0;
        return;
    } 
    groups[i].size=1;
    char msg[256];
    sprintf(msg,"Group created successfully with id %d",i);
    write(id, msg, sizeof(msg));
    sprintf(msg,"You have a request from group id %d ",i);
    for(int j=1;j<g_size; j++){
        int t=groups[i].invited[j];
        if(t<0) continue;
        write(t%100,msg,sizeof(msg));
    }
    // for(int j=1;j<g_size; j++)
    //     groups[i].invited[j]=-1;
}

void join_gp(int id, char *line){
    char* token=NULL;
    int gid=atoi(token=strtok(line," "));
    char msg[256];
    if(groups[gid].size==0){
        write(id, "No such group exist", 19);
        return;
    }
    int flag=0;
    for(int j=1; j<g_size; j++){
        if(groups[gid].invited[j]==fid[id]){
            flag=1;
            break;
        }
    }
    if(flag==0){
        write(id, "You are not invited", 19);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].mem[j]<0){
            groups[gid].mem[j]=fid[id];
            write(id, "You are added", 13);
            break;
        }
    }
    groups[gid].size++;
}

void decline_gp(int id, char*line){
    char* token=NULL;
    int gid=atoi(token=strtok(line," "));
    char msg[256];
    if(groups[gid].size==0){
        write(id, "No such group exist", 19);
        return;
    }
    int flag=0;
    int j=1;
    for(; j<g_size; j++){
        if(groups[gid].invited[j]==fid[id]){
            flag=1;
            break;
        }
    }
    if(flag==0){
        write(id, "You already had not invited", 27);
        return;
    }
    groups[gid].invited[j]=-1;
    write(id, "Reject accepted", 15);
}

void send_gp(int id, char* line){
    char* token;
    int gid=atoi(token=strtok(line," "));
    char *rem = strtok(NULL,"\n");
    if(groups[gid].size==0){
        write(id, "No such group exist", 19);
        return;
    }
    int ismember=0, isadmin=0;
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]==fid[id]) isadmin=1;
    }
    if(isadmin==0 && groups[gid].broad!=0){
        write(id, " Only admin can send the message", 32);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].mem[j]==fid[id]) ismember=1;
    }
    if(ismember==0){
        write(id, "You are not the member of this group", 36);
        return;
    }
    char msg[256];
    strcpy(msg,rem);
    for(int j=0; j<g_size; j++){
        if(groups[gid].mem[j]<0) continue;
        if(groups[gid].mem[j]==fid[id]) continue;
        write((groups[gid].mem[j])%100, msg, sizeof(msg));
    }

}

void make_admin(int id, char * line){
    char* token;
    int gid=atoi(token=strtok(line," "));
    int cid = atoi(token=strtok(NULL," "));
    int isadmin=0,ismember=0;
    char msg[256];
    if(groups[gid].size==0){
        write(id, "No such group exist", 19);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]==fid[id]) isadmin=1;
    }
    if(isadmin==0){
        write(id, " You are not an admin", 21);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].mem[j]==cid) ismember=1;
    }
    if(ismember==0){
        sprintf(msg,"%d is not a member of requested group", cid);
        write(id, msg, sizeof(msg));
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]<0){
           groups[gid].admin[j]=cid;
           break; 
        }
    }
    sprintf(msg,"%d is now admin for group %d", cid,gid);
    write(id, msg, sizeof(msg));
    // printg(gid);
}

void add_togroup(int id, char * line){
    char* token;
    int gid=atoi(token=strtok(line," "));
    token=strtok(NULL," ");
    int isadmin=0;
    if(groups[gid].size==0){
        write(id, "No such group exist", 19);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]==fid[id]) isadmin=1;
    }
    if(isadmin==0){
        write(id, " You are not an admin", 21);
        return;
    }
    int clist[20],i=0;
    while(token!=NULL){
        clist[i++]=atoi(token);
        if(fid[clist[i-1]%100]<0){
            write(id, "some client doesn't exist", 25);
            return;
        }
        token=strtok(NULL," ");
    }
    if(i>g_size-groups[gid].size){
         write(id, " group size exceeded", 20);
        return;
    }
    int k=0;
    for(int j=0; j<g_size && k<i; j++){
        if(groups[gid].mem[j]<0){
            groups[gid].mem[j]=clist[k++];
        }
    }
    // for(int k=0; k<i;k++){
    //     if()
    //        groups[gid].mem[j++]=clist[k];
    // }
    write(id, " clients added", 14);
    groups[gid].size+=i;
}

void remove_fromgroup(int id, char * line){
    char* token;
    int gid=atoi(token=strtok(line," "));
    token=strtok(NULL," ");
    int isadmin=0;
    if(groups[gid].size==0){
        write(id, "No such group exist", 19);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]==fid[id]) isadmin=1;
    }
    if(isadmin==0){
        write(id, " You are not an admin", 21);
        return;
    }
    int clist[20],i=0;
    while(token!=NULL){
        clist[i++]=atoi(token);
        token=strtok(NULL," ");
    }
    int k=0;
    for(int k=0; k<i; k++){
        for(int j=0; j<g_size; j++){
            if(groups[gid].mem[j]==clist[k]){
            groups[gid].mem[j]=-1;
            }
       }
    }
    
    write(id, " Clients removed", 16);
}

void make_gbroadcast(int id, char* line){
    char* token;
    int gid=atoi(token=strtok(line," "));
    int isadmin=0;
    if(groups[gid].size==0){
        write(id, "No such group exist", 19);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]==fid[id]) isadmin=1;
    }
    if(isadmin==0){
        write(id, " You are not an admin", 21);
        return;
    }
    groups[gid].broad=1;
    write(id,"Group modified for broadcast only",33);
}

void active_gps(int id){
    char msg[256];
    for(int i=0; i<mx_groups; i++){
        if(groups[i].size==0) continue;
        int flag=0;
        bzero(msg,256);
        for(int j=0; j<g_size; j++){
            if(groups[i].mem[j]==fid[id]) flag=1;
        }
        int clients[g_size], admins[g_size];
        int c=0,a=0;
        if(flag==1){
            for(int j=0; j<g_size; j++){
                if(groups[i].mem[j]>0) clients[c++]=groups[i].mem[j];
                if(groups[i].admin[j]>0) admins[a++]= groups[i].admin[j];
            }
            char temp[256];
            sprintf(temp, "Group id: %d, ",i);
            strcat(msg,temp);
            strcat(msg,"Admins are: ");
            for(int k=0; k<a; k++){
               sprintf(temp," %d,",admins[k]);
               strcat(msg,temp);
            }
            strcat(msg," Clients are: ");
            for(int k=0; k<c; k++){
               sprintf(temp," %d,",clients[k]);
               strcat(msg,temp);
            }
            write(id,msg,sizeof(msg));
        }
    }
}

void quit(int id){
    for(int i=0; i<mx_groups; i++){
        if(groups[i].size==0) continue;
        for(int j=0; j<g_size; j++){
            if(groups[i].mem[j]==fid[id]) groups[i].mem[j]=-1;
            if(groups[i].admin[j]==fid[id]) groups[i].admin[j]=-1;
        }
    }
}

void make_adminreq(int id, char * line){
    char* token;
    int gid=atoi(token=strtok(line," "));
    int isadmin=0,ismember=0;
    char msg[256];
    if(groups[gid].size==0){
        write(id, "No such group exist", 19);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]==fid[id]) isadmin=1;
    }
    if(isadmin==1){
        write(id, " You are already an admin", 25);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].mem[j]==fid[id]) ismember=1;
    }
    if(ismember==0){
        sprintf(msg,"%d is not a member of requested group", fid[id]);
        write(id, msg, sizeof(msg));
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].adreq[j]<0){
            groups[gid].adreq[j]=fid[id];
            break;
        }
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]>0){
           sprintf(msg,"%d has requested for admin in group id %d",fid[id],gid);
           write(groups[gid].admin[j]%100, msg, sizeof(msg)); 
        }
    }
}

void approve_adminreq(int id, char * line){
    char* token;
    int gid=atoi(token=strtok(line," "));
    int cid = atoi(token=strtok(NULL," "));
    int isadmin=0,ismember=0;
    char msg[256];
    if(groups[gid].size==0){
        write(id, "No such group exist", 19);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]==fid[id]) isadmin=1;
    }
    if(isadmin==0){
        write(id, " You are not an admin", 21);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].mem[j]==cid) ismember=1;
    }
    if(ismember==0){
        sprintf(msg,"%d is not a member of requested group", cid);
        write(id, msg, sizeof(msg));
        return;
    }
    isadmin=0;
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]==cid) isadmin=1;
    }
    if(isadmin==1){
        sprintf(msg,"%d is already an admin", cid);
        write(id, msg, sizeof(msg));
        return;
    }
    int reqstatus=0;
    for(int j=0; j<g_size; j++){
        if(groups[gid].adreq[j]==cid) reqstatus=1;
    }
    if(reqstatus==0){
        write(id, "No admin request available", 26);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]<0){
           groups[gid].admin[j]=cid;
           break; 
        }
    }
    sprintf(msg,"%d is now admin for group %d", cid,gid);
    write(id, msg, sizeof(msg));
    sprintf(msg, " your admin request for group %d accepted", gid);
    write(cid%100, msg, sizeof(msg));
    // printg(gid);
}

void decline_adminreq(int id, char * line){
    char* token;
    int gid=atoi(token=strtok(line," "));
    int cid = atoi(token=strtok(NULL," "));
    int isadmin=0,ismember=0;
    char msg[256];
    if(groups[gid].size==0){
        write(id, "No such group exist", 19);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]==fid[id]) isadmin=1;
    }
    if(isadmin==0){
        write(id, " You are not an admin", 21);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].mem[j]==cid) ismember=1;
    }
    if(ismember==0){
        sprintf(msg,"%d is not a member of requested group", cid);
        write(id, msg, sizeof(msg));
        return;
    }
    isadmin=0;
    for(int j=0; j<g_size; j++){
        if(groups[gid].admin[j]==cid) isadmin=1;
    }
    if(isadmin==1){
        sprintf(msg,"%d is already an admin", cid);
        write(id, msg, sizeof(msg));
        return;
    }
    int reqstatus=0;
    for(int j=0; j<g_size; j++){
        if(groups[gid].adreq[j]==cid) reqstatus=1;
    }
    if(reqstatus==0){
        write(id, "No admin request available", 26);
        return;
    }
    for(int j=0; j<g_size; j++){
        if(groups[gid].adreq[j]==cid){
           groups[gid].adreq[j]=-1;
           break; 
        }
    }
    write(id, "request declined",16);
    sprintf(msg,"admin request for group id %d is declined by %d", gid,fid[id]);
    write(cid%100, msg, sizeof(msg));
    // printg(gid);
}


int main(int argc, char *argv[])
{
	srand(time(0)); /*seeding the rand() function*/
	memset(fid, -1, sizeof(fid));
    for(int j=0;j<mx_groups; j++){
        groups[j].size=0;
        groups[j].broad=0;
    }
	// signal(**FILL SIGNAL VALUE**, sigCHandler);  // handles ^C
	// signal(**FILL SIGNAL VALUE**, sigZhandler);    //handles ^Z
	
	// if (argc < 2) {
	// 	fprintf(stderr,"ERROR, no port provided\n");
	// 	exit(1);
	// }

	int sockfd, newsockfd, portno=5002, clilen, pid,client_id,flags;
    char buffer[256];
	struct sockaddr_in serv_addr, cli_addr;
	  
	sockfd = socket(AF_INET, SOCK_STREAM, 0);  /*getting a sockfd for a TCP connection*/
	if (sockfd < 0)  error("ERROR opening socket");

	int optval = 1;
	setsockopt(sockfd, SOL_SOCKET, SO_REUSEADDR, &optval, sizeof(optval));

	if ((flags = fcntl(sockfd, F_GETFL, 0)) < 0) 
	{ 
		error("can't get flags to SCOKET!!");
	} 


	if (fcntl(sockfd, F_SETFL, flags | O_NONBLOCK) < 0) 
	{ 
		error("fcntl failed.");
	}

	bzero((char *) &serv_addr, sizeof(serv_addr));

	// portno = atoi(argv[1]);
	serv_addr.sin_family = AF_INET; /*symbolic constant for IPv4 address*/
	serv_addr.sin_addr.s_addr = INADDR_ANY; /*symbolic constant for holding IP address of the server*/
	serv_addr.sin_port = htons(portno);

	if (bind(sockfd, (struct sockaddr *) &serv_addr, sizeof(serv_addr)) < 0){
		error("ERROR on binding");
 	}
 	
 	/*Initialize all the data structures and the semaphores you will need for handling the client requests*/
 	socklen_t addrlen;
    int ret_val = listen(sockfd, 5);
     if (ret_val != 0) {
         fprintf(stderr, "listen failed [%s]\n", strerror(errno));
         close(sockfd);
         return -1;
     }

    for (int i=0;i < MAX_CONNECTIONS;i++) {
         all_connections[i] = -1;
     }
     all_connections[0] = sockfd;
     fid[sockfd]=ran(sockfd);

     int activity;


 	printf("\nServer Started...\n");
    // FILE *fp = fopen("log.txt", "a+");
    // printf(fp,"%s","sadca");
    // fclose(fp);
    


    // signal(SIGINT, sigCHandler);
    signal(SIGTSTP, sigZhandler);
	while (1) {
		
		/*Write appropriate code to set up fd_set to be used in the select call. Following is a rough outline of what needs to be done here	
			-Calling FD_ZERO
			-Calling FD_SET
			-identifying highest fd for using it in select call
		*/
        fd_set readfds;
        // fd_set writefds;
        // fd_set exceptfds;
        FD_ZERO(&readfds);
         /* Set the fd_set before passing it to the select call */
         for (int i=0;i < MAX_CONNECTIONS;i++) {
             if (all_connections[i] >= 0) {
                 FD_SET(all_connections[i], &readfds);
             }
         }

         /* Invoke select() and then wait! */
        //  printf("\nUsing select() to listen for incoming events\n");
        

		activity = select(FD_SETSIZE, &readfds, NULL, NULL, NULL); //give appropriate parameters 
		
		if((activity<0)&&(errno!=EINTR)) //fill appropriate parameters here
		{
			error("!!ERROR ON SELECT!!");
		}
		
		/*After successful select call you can now monitor these two scenarios in a non-blocking fashion:
			- A new connection is established, and
			- An existing client has made some request
		  You have to handle both these scenarios after line 191.
		*/
        if (activity >= 0 ) {
            //  printf("Select returned with %d\n", activity);
             if (FD_ISSET(sockfd, &readfds)) {                       // Check if the fd with event is the serverfd
                //  printf("Returned fd is %d (server's fd)\n", sockfd);
                 
                 newsockfd = accept(sockfd, (struct sockaddr*)&cli_addr, &addrlen);
                 if (newsockfd>= 0) {
                    if(count==MAX_CONNECTIONS-1){
                        char le[50];
                        bzero(le,50);
                        sprintf(le,"Client limit exceeded");
                        write(newsockfd, le, sizeof(le));
                        close(newsockfd);
                    }
                    else{
                        fid[newsockfd]=ran(newsockfd);
                        char wl[256]; // for wl msg
                    bzero(wl,256);
                    sprintf(wl, "Welcome with id %d", fid[newsockfd]);
                    write(newsockfd, wl, sizeof(wl));
                    count++;
                     printf("New connection with fd: %d\n", fid[newsockfd]);
                    //  bzero(buffer,256);
                    //  char con_msg[256];
                    //  sprintf(con_msg, "id %d get connected", newsockfd);
                     for (int i=1;i < MAX_CONNECTIONS;i++) {
                         if (all_connections[i] < 0) {
                             all_connections[i] = newsockfd; 
                             break;
                         }
                        //  write(all_connections[i],con_msg,sizeof(con_msg));
                     }
                    }
                    
                 }
                 else {
                     fprintf(stderr, "accept failed [%s]\n", strerror(errno));
                 }
                 activity--;
                 if (!activity) continue;
             }

             /* Check if the fd with event is a non-server fd */
             for (int i=1;i < MAX_CONNECTIONS;i++) {
                 if ((all_connections[i] > 0) && (FD_ISSET(all_connections[i], &readfds))) { 
                    //  printf("Returned fd is %d [index, i: %d]\n", all_connections[i], i);
                     bzero(buffer,256);
                     ret_val = recv(all_connections[i], buffer, 256, 0);
                     if (ret_val == 0) {
                        count--;
                        int dis_id = fid[all_connections[i]];
                         printf("Closing connection for fd:%d\n", dis_id);
                         close(all_connections[i]);
                         fid[all_connections[i]]=-1;
                         all_connections[i] = -1;                     // Connection is now closed 
                         
                         for(int j=i; j<MAX_CONNECTIONS-1; j++){
                            all_connections[j] = all_connections[j+1];
                         }
                         char dis_msg[256];
                         bzero(dis_msg,256);
                         sprintf(dis_msg, "id %d got disconnected", dis_id);
                         for(int j=1; j<MAX_CONNECTIONS; j++){
                            if(all_connections[j]<0) break;
                            write(all_connections[j],dis_msg,sizeof(dis_msg));
                         }
                        // printf("%s", dis_msg);
                     } 
                     if (ret_val > 0) { 
                        if(strncmp(buffer,"/activegroups",13)==0){
                            active_gps(all_connections[i]);
                        }
                        else if(strncmp(buffer,"/active",7)==0){
                                bzero(buffer,256);
                                strcat(buffer, "Active Ids: ");
                                for (int i=1;i < MAX_CONNECTIONS;i++) {
                                    if (all_connections[i] < 0) 
                                        break;
                                    char temp[256];
                                    sprintf(temp, "%d; ", fid[all_connections[i]]);
                                    strcat(buffer, temp);
                                }
                                write(all_connections[i],buffer,sizeof(buffer));
                        }
                        else if(strncmp(buffer,"/sendgroup",10)==0){
                            send_gp(all_connections[i],buffer+11);
                        }
                        else if(strncmp(buffer,"/send",5)==0){
                            // printf("%s",buffer);
                            char *line=buffer+6,*token=NULL;
                            char msg[256];
                            int dest_id = (atoi(token=strtok(line, " ")))%100;
                            token=strtok(NULL," ");
                            strcpy(msg,token);
                            // printf("%d %s", dest_id,buffer+8);
                            // strcat(msg,"\0");
                            if(dest_id==all_connections[0]){
                                printf("%s", msg);
                                continue;
                            }
                            int avl=0;
                            for(int j=1; j<MAX_CONNECTIONS; j++){
                                if(all_connections[j]<0) break;
                                if(all_connections[j]==dest_id) avl=1;
                            }
                            if(avl==1)
                                write(dest_id,msg,sizeof(msg));
                            else
                                write(all_connections[i],"Message discarded as dest_id is down",36);
                            FILE *fp = fopen("log.txt", "a+");
                            fprintf(fp,"\nid %d sent to id %d : %s" ,fid[all_connections[i]],fid[dest_id],msg);
                            fclose(fp);
                        }
                        else if(strncmp(buffer,"/quit",5)==0){
                            quit(all_connections[i]);
                            int dis_id = fid[all_connections[i]];
                            count--;
                            printf("Connection Closed for fd:%d\n", dis_id);
                            write(all_connections[i],"CONNECTION TERMINATED",21);
                            close(all_connections[i]);
                            fid[all_connections[i]]=-1;
                            all_connections[i] = -1;                     // Connection is now closed 
                            for(int j=i; j<MAX_CONNECTIONS-1; j++){
                                all_connections[j] = all_connections[j+1];
                            }
                            char dis_msg[256];
                            bzero(dis_msg,256);
                            sprintf(dis_msg, "id %d got disconnected", dis_id);
                            for(int j=1; j<MAX_CONNECTIONS; j++){
                                if(all_connections[j]<0) break;
                                write(all_connections[j],dis_msg,sizeof(dis_msg));
                            }
                            // printf("%s", dis_msg);
                        }
                        else if(strncmp(buffer,"/broadcast",10)==0){
                            int curr_id = all_connections[i];
                            for(int j=1; j<MAX_CONNECTIONS; j++){
                                if(all_connections[j]<0) break;
                                if(j==i) continue;
                                write(all_connections[j],buffer+11,sizeof(buffer+11));
                            }
                            FILE *fp = fopen("log.txt", "a+");
                            fprintf(fp,"\nid %d broadcast %s ",fid[all_connections[i]],buffer+11);
                            fclose(fp);
                        }
                        else if(strncmp(buffer,"/makegroupbroadcast",19)==0){
                            make_gbroadcast(all_connections[i],buffer+20);
                        }
                        else if(strncmp(buffer,"/makegroupreq",13)==0){
                            gp(all_connections[i],buffer+14);
                        }
                        else if(strncmp(buffer,"/makegroup",10)==0){
                            gwp(all_connections[i],buffer+11);
                        }
                        else if(strncmp(buffer,"/joingroup",10)==0){
                            join_gp(all_connections[i],buffer+11);
                        }
                        else if(strncmp(buffer,"/declinegroup",13)==0){
                            decline_gp(all_connections[i],buffer+14);
                        }
                        else if(strncmp(buffer,"/makeadminreq",13)==0){
                            make_adminreq(all_connections[i],buffer+14);
                        }
                        else if(strncmp(buffer,"/approveadminreq",16)==0){
                            approve_adminreq(all_connections[i],buffer+17);
                        }
                        else if(strncmp(buffer,"/declineadminreq",16)==0){
                            decline_adminreq(all_connections[i],buffer+17);
                        }
                        else if(strncmp(buffer,"/makeadmin",10)==0){
                            make_admin(all_connections[i],buffer+11);
                        }
                        else if(strncmp(buffer,"/addtogroup",11)==0){
                            add_togroup(all_connections[i],buffer+12);
                        }
                        else if(strncmp(buffer,"/removefromgroup",16)==0){
                            remove_fromgroup(all_connections[i],buffer+17);
                        }
                        
                        else{
                            bzero(buffer,256);
                            sprintf(buffer, "%s", "Invalid Command");
                            // printf("%s",buffer);
                            write(all_connections[i],buffer,sizeof(buffer));
                        }
                     } 
                     if (ret_val == -1) {
                         printf("recv() failed for fd: %d [%s]\n", all_connections[i], strerror(errno));
                         break;
                     }
                 }
                //  ret_val--;
                //  if (!ret_val) continue;
             }                       // fend of for-loop 
        }

	}            //end of while 
	
	return 0; 
}
