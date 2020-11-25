#include <stdio.h>
#include <strings.h>
#include <string.h>
#include <stdlib.h>
#include <pthread.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h> 

#define PORT 15000
#define max_user 5
#define MAXSIZE 1024

struct user{
	char name[1024];
	int connect_to;
};

struct user users[100];

int fdt[max_user] = {0};
char mes[1024] = {'\0'};

int win_dis[8][3] = {{0, 1, 2}, {3, 4, 5}, {6, 7, 8}, {0, 3, 6}, {1, 4, 7}, {2, 5, 8}, {0, 4, 8}, {2, 4, 6}};

int find_fd (char *name) {
    for (int i=0;i<100;i++)
        if (strcmp(name,users[i].name)==0)
            return i;
    return -1;
}


void Command_response(char *mes,int sender){

	int command = 0;
	sscanf(mes,"%d",&command);
	
	if(command == 1){	//新增使用者
		char name[100];
		char buf[100] = {'\0'};
		sscanf(mes,"1 %s",name);
		strncpy (users[sender].name,name,100);
                strcat(buf,"1 ");
                strcat(buf,name);
                send(sender,buf,strlen(buf),0);
	        printf("1:%s\n",name);
	}	
	else if(command == 2){	//顯示目前玩家
		char buf[MAXSIZE],tmp[100],str_num[100] = {'\0'};
            	int p = sprintf(buf,"2 ");
            	int num = 0;
            	for (int i=0;i<100;i++)
                	if (strcmp(users[i].name,"")!=0){
                    		sscanf(users[i].name,"%s",tmp);
                    		p = sprintf(buf+p,"%s ", tmp) + p;
                    		num++;
                	}
            	sprintf(str_num," \n當前玩家人數/最大玩家人數: %d/%d", num,max_user);
            	strcat(buf,str_num);
            	printf("2:%s\n",buf);
                send(sender,buf,strlen(buf),0);

	}
	else if(command == 3){	//請求對戰
		char a[100],b[100];	
   		char buf[MAXSIZE];
   		sscanf (mes,"3 %s %s",a,b);
            	int b_fd = find_fd(b);
            	sprintf(buf, "4 %s 邀請你對戰 打ㄇ?\n", a);
            	send(b_fd, buf, strlen(buf), 0);
            	printf("3:%s", buf);

        }
	else if(command == 5){	//接受對戰與否
		int state;
                char inviter[100];
                sscanf(mes, "5 %d %s", &state, inviter);
                if (state==1){
                    send(sender, "6\n", 2, 0);
                    send(find_fd(inviter), "6\n", 2, 0);
                    int fd = find_fd(inviter);
		    /*把雙方玩家的connect_to都改成彼此的fd*/
                    users[sender].connect_to = fd;
                    users[fd].connect_to = sender;
                    printf("6:\n");
                }
        }
	else if(command == 7){	//確認勝負
		int board[9];
                char state[100];
    	        char buf[MAXSIZE];

		sscanf(mes, "7  %d %d %d %d %d %d %d %d %d",&board[0],&board[1],&board[2],&board[3],&board[4],&board[5],&board[6],&board[7],&board[8]);
		for (int i=0;i<100;i++){
	                state[i] = '\0';
		}
		memset(buf,'\0',MAXSIZE);
                memset(state,'\0',sizeof(state));
		strcat(state, users[sender].name);
		/*八種勝利條件判定是否勝利*/
		
		for(int i=0;i<8;i++){	
			if (board[win_dis[i][0]]==board[win_dis[i][1]] && board[win_dis[i][1]]==board[win_dis[i][2]]) {
	                    if (board[win_dis[i][0]]!=0) {

        	            strcat(state, "獲得勝利！！！！!遊戲結束！！！！！\n");
       	        	    sprintf (buf,"8  %d %d %d %d %d %d %d %d %d %s\n",board[0],board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8],state);
                       		printf ("7:%s",buf);
      		            send(sender,buf,sizeof(buf),0);
                	    send(users[sender].connect_to,buf,sizeof(buf),0);
                    	    return;
                    	    }
                	}
		}
		/*若沒分出勝負 再輪到另一個人的回合*/
		memset(state,'\0',sizeof(state));
		memset(buf,'\0',MAXSIZE);

		for (int i = 0; i < 9;i++) {
                	if (i==8) {
                    		strcat(state, "平手了!別再打了！\n");
                                sprintf (buf,"8  %d %d %d %d %d %d %d %d %d %s\n",board[0],board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8],state);
                    		printf ("7:%s",buf);
                    		send(sender,buf,sizeof(buf),0);
                    		send(users[sender].connect_to,buf,sizeof(buf),0);
                    		return;
                	}
                	if (board[i]== 0)
                    		break;
            	}

		strcat(state, users[users[sender].connect_to].name);
                strcat(state, "輪到你惹!\n");
		sprintf (buf,"8  %d %d %d %d %d %d %d %d %d %s\n",board[0],board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8],state);
		printf("7:%s",buf);
		send(sender,buf,sizeof(buf),0);
		send(users[sender].connect_to,buf,sizeof(buf),0);
        }
}




void *pthread_service(void* fdtt){
	int fd = *(int *)fdtt;
	
	while(1){
		int i;
		int rev;
		
		rev = recv(fd,mes,MAXSIZE,0);
		printf("\n\n\nrecv_mes = %s\n\n\n",mes);
		
		/*client中斷連線時 關閉thread 清除使用者資料*/
		if(rev <= 0){
			for(i=0;i<max_user;i++){
				if(fd==fdt[i]){
					fdt[i] = 0;
				}
			}
			memset(users[fd].name,'\0',sizeof(users[fd].name));
            		users[fd].connect_to = -1;
  			break;
	
		}

		Command_response(mes,fd);
		bzero(mes,MAXSIZE);
	}
	close(fd);
}


int main(){
	int socket_fd,confd;
	int i,j,k;
	struct sockaddr_in server;
    	struct sockaddr_in client;
	int number=0;
	int sin_size = sizeof(struct sockaddr_in);	

	/*初始化*/
	for(i=0;i<100;i++){
		for(j=0;j<1024;j++){
			users[i].name[j] = '\0';
			users[i].connect_to = -1;
		}
	}

	/*建立socket*/
	socket_fd = socket(AF_INET, SOCK_STREAM, 0);
	int opt = SO_REUSEADDR;
        setsockopt(socket_fd, SOL_SOCKET, SO_REUSEADDR, &opt, sizeof(opt));
	bzero(&server,sizeof(server));

	server.sin_family=AF_INET;
    	server.sin_port=htons(PORT);
    	server.sin_addr.s_addr = htonl (INADDR_ANY);

	bind(socket_fd, (struct sockaddr *)&server, sizeof(struct sockaddr));
	listen(socket_fd,1);
	printf("OX伺服器已成功建立 等待玩家連線...\n");
	
	/*等待client連線*/
	while(1){

		/*accept client*/
		confd = accept(socket_fd,(struct sockaddr *)&client,&sin_size);
		
		/*檢查人數有無超過上限*/
		if(number >= max_user){
			printf("超過連線人數 將關閉連線\n");
			close(confd);
		}
		/*找出空的fdt*/
		for(i=0;i<max_user;i++){
	            if(fdt[i]==0){
        	        fdt[i]=confd;
                	break;
           	 	}
		}
		/*pthread*/
		pthread_t t;
        	pthread_create(&t,NULL,(void*)pthread_service,&confd);
	        number++;

	}	
	close(confd);
}
