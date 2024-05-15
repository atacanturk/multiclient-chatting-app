#include <stdio.h>
#include <stdlib.h>
#include <unistd.h>
#include <string.h>
#include <fcntl.h>
#include <pthread.h>
#include <netinet/in.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <arpa/inet.h>

void sendUserIdToServer(int clientSocket, int userID, int config_flag);

void *doRecieving(void *sockID)
{

	int clientSocket = *((int *)sockID);

	while (1)
	{

		char data[1024];
		int read = recv(clientSocket, data, 1024, 0);
		data[read] = '\0';
		printf("%s\n", data);
	}
}

int main(int argc, char *argv[])
{

	int userID = atoi(argv[1]);
	int config_flag = 0;
	char user_name[12];
	char user_surname[12];
	char user_telephone[12];
	int clientSocket = socket(PF_INET, SOCK_STREAM, 0);

	struct sockaddr_in serverAddr;

	serverAddr.sin_family = AF_INET;
	serverAddr.sin_port = htons(8080);
	serverAddr.sin_addr.s_addr = htonl(INADDR_ANY);

	if (connect(clientSocket, (struct sockaddr *)&serverAddr, sizeof(serverAddr)) == -1)
		return 0;

	printf("Connection established ............\n");

	char input[1024]; // buffer for inputs
	pthread_t thread;
	pthread_create(&thread, NULL, doRecieving, (void *)&clientSocket);
	while (1)
	{

		if (config_flag == 0 && userID != 0)
		{
			// send CONFIG to server
			char conf[8] = "CONFIG";
			send(clientSocket, conf, 1024, 0);

			// send userID to server
			char userID_str[12];

			sprintf(userID_str, "%d", userID);
			send(clientSocket, userID_str, 1024, 0);

			printf("Your name:");
			scanf("%s", user_name);
			send(clientSocket, user_name, 1024, 0);

			printf("\nYour surname:");
			scanf("%s", user_surname);
			send(clientSocket, user_surname, 1024, 0);

			printf("\nYour telephone:");
			scanf("%s", user_telephone);
			send(clientSocket, user_telephone, 1024, 0);

			config_flag = 1;
			printf("config flag is %d\n", config_flag);
		}

		int expression;
		// menu for client to operations
		printf("Enter your choice : \n");
		printf("1 FOR ADD CONTACT\n");
		printf("2 FOR SEND MESSAGE\n");
		printf("3 FOR HISTORY\n");
		printf("4 FOR ONLINE USERS\n");
		scanf("%d", &expression);
		switch (expression)
		{
		case 1:;
			char add_opt[10] = "ADD";
			send(clientSocket, add_opt, 1024, 0);

			printf("type userID of contact\n");
			scanf("%s", input);
			send(clientSocket, input, 1024, 0);
			break;
		case 2:;
			char send_opt[10] = "SEND";
			send(clientSocket, send_opt, 1024, 0);

			printf("type dest_id message\n");
			scanf("%s", input);
			send(clientSocket, input, 1024, 0);

			scanf("%[^\n]s", input);
			send(clientSocket, input, 1024, 0);
			break;
		case 3:;
			char history_opt[10] = "HISTORY";
			send(clientSocket, history_opt, 1024, 0);

			printf("type userID of contact\n");
			scanf("%s", input);
			send(clientSocket, input, 1024, 0);
			break;
		case 4:;
			char list_opt[10] = "LIST";
			send(clientSocket, list_opt, 1024, 0);

		default:
			break;
		}
		
	}

	pthread_cancel(thread);
}
