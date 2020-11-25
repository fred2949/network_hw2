#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <strings.h>
#include <unistd.h>
#include <sys/types.h>
#include <sys/socket.h>
#include <netinet/in.h>
#include <netdb.h>
#include <pthread.h>
#include <arpa/inet.h>

#define MAXSIZE 100
#define PORT 15000

char name[100];
int fd;
char sendbuf[1024];
char recvbuf[1024];
int board[9];

void show_board(int *board){
    printf("===========================================\n");
    printf("號碼位置：        當前棋盤：\n");
    printf("┌───┬───┬───┐        ┌───┬───┬───┐\n");
    printf("│ 0 │ 1 │ 2 │        │ %d │ %d │ %d │\n", board[0], board[1], board[2]);
    printf("├───┼───┼───┤        ├───┼───┼───┤\n");
    printf("│ 3 │ 4 │ 5 │        │ %d │ %d │ %d │\n", board[3], board[4], board[5]);
    printf("├───┼───┼───┤        ├───┼───┼───┤\n");
    printf("│ 6 │ 7 │ 8 │        │ %d │ %d │ %d │\n", board[6], board[7], board[8]);
    printf("└───┴───┴───┘        └───┴───┴───┘\n");
}

int choose_user_turn(int *board){
    int i=0;
    int inviter=0, invitee=0;
    for(i=0; i<9; i++){
        if(board[i] == 1){
            inviter++;
        }
        else if(board[i] == 2){
            invitee++;
        }
    }
    if(inviter > invitee)
        return 2;
    else
        return 1;
}



void command_menu(){
	 printf("┌─────────────────────────────────────────────┐\n");
	 printf("│改變使用者名稱(1 名子)                       │\n");
	 printf("│顯示所有玩家名稱,以及上線人數(2)             │\n");
	 printf("│邀請別的玩家對戰(3 你的名稱 對方的名稱)      │\n");
	 printf("│登出(ex)                                     │\n");
	 printf("└─────────────────────────────────────────────┘\n\n");
}

void chess_on_board(int *board,int chess){
	show_board(board);
	int user_choice = choose_user_turn(board);
	board[chess] = user_choice;
	sprintf(sendbuf, "7  %d %d %d %d %d %d %d %d %d\n", board[0], \
        board[1],board[2],board[3],board[4],board[5],board[6],board[7],board[8]);

}
/*接收server到client*/
void pthread_recv(void *ptr){
	int command;
	while(1){
		memset(sendbuf,0,sizeof(sendbuf));
		command = 0;

		recv(fd,recvbuf,MAXSIZE,0);
		sscanf(recvbuf,"%d",&command);

		if(command == 1){	//改名or創名
			printf("你現在的名子是...%s\n",&recvbuf[2]);
		}
		else if(command == 2){	//顯示所有名稱
			printf("%s\n",&recvbuf[2]);
		}
		else if(command == 4){	//問對方接不接受
			char inviter[100];
	                sscanf(recvbuf,"%d %s",&command, inviter);
        	        printf("%s\n", &recvbuf[2]); // 
                	printf("打！ (5 1 對手的名稱) %s\n", inviter);
                	printf("窩不敢！ （5 0 ）%s\n", inviter);
		}
		else if(command == 6){
			printf("^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^^\n");
			printf("決鬥吧！！\n");
			printf("數字象徵 0:空白 1:邀請玩家 2:被邀請玩家 \n");
			printf("由邀請者先攻\n");
			printf("請下（-下的位置）\n");
			show_board(board);
		}
		else if(command == 8){
			char word[100];
			sscanf(recvbuf,"%d %d%d%d%d%d%d%d%d%d %s",&command,&board[0],&board[1],&board[2],&board[3],&board[4],&board[5],&board[6],&board[7],&board[8], word);
			show_board(board);
			printf("%s\n",word);
			printf("請下（-下的位置）\n");
		}
		memset(recvbuf,0,sizeof(recvbuf));
	}
}


int main(int argc,char *argv[]){
	

	int rev;
	struct hostent *he;
	struct sockaddr_in server;
	char buf[MAXSIZE];
	char mes[100] = {'\0'};

	if(argc!=2){
		printf("請輸入正確格式\n");
		exit(1);
	}
	/*建立封包*/
	he = gethostbyname(argv[1]);
	fd=socket(AF_INET, SOCK_STREAM, 0);
	bzero(&server,sizeof(server));

    	server.sin_family = AF_INET;
    	server.sin_port = htons(PORT);
    	server.sin_addr = *((struct in_addr *)he->h_addr);
	/*連線*/
	connect(fd,(struct sockaddr *)&server,sizeof(struct sockaddr));
	printf("連線成功！\n");
	/*輸入玩家名稱*/
	printf("請輸入你的名子:");
	fgets(name,sizeof(name),stdin);
	strcat(mes,"1 ");
	strcat(mes,name);
	send(fd,mes,strlen(mes),0);
	/*顯示選單*/
	command_menu();
	/*pthread*/
	pthread_t tid;
    	pthread_create(&tid, NULL, (void*)pthread_recv, NULL);
	while(1){
		memset(sendbuf,0,sizeof(sendbuf));
		fgets(sendbuf,sizeof(sendbuf),stdin);
		int chess;
		/*下棋*/
		if(sendbuf[0] == '-'){
			sscanf(&sendbuf[1], "%d", &chess);
			chess_on_board(board, chess);
		}
		send(fd,sendbuf,strlen(sendbuf),0);	
		/*登出*/
		if(strcmp(sendbuf,"ex\n")==0){
            	    memset(sendbuf,0,sizeof(sendbuf));
        	    printf("你已經登出惹\n");
        	    return 0;
	        }

	}

	pthread_join(tid,NULL);
	close(fd);
	return 0;
}
