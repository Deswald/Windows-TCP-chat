#include "chat.hpp"
#include "user.hpp"

char buffer[MESSAGE_BUFFER]; 
char message[MESSAGE_BUFFER];
int message_size;
const char *endSession_string = "endSession";
const char *signUp_string = "signUp";
const char *signIn_string = "signIn";
const char *checkingUserMessages_string = "checkingUserMessages";
const char *logOut_string = "logOut";
const char *messageSomeOne_string = "messageSomeOne";
const char *messageAll_string = "messageAll";
char bufferName[MESSAGE_BUFFER];
char bufferLogin[MESSAGE_BUFFER]; 
char bufferPassword[MESSAGE_BUFFER]; 
char bufferAddressee[MESSAGE_BUFFER];
char bufferReceivedMessage[MESSAGE_BUFFER];

int chat()
{
	WSADATA wsaData;
	int iResult;

	SOCKET ListenSocket = INVALID_SOCKET;
	SOCKET ClientSocket = INVALID_SOCKET;

	struct addrinfo* result = NULL;
	struct addrinfo hints;

	int iSendResult;
	char recvbuf[MESSAGE_BUFFER];
	int recvbuflen = MESSAGE_BUFFER;

	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return 1;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = AF_INET;
	hints.ai_socktype = SOCK_STREAM;
	hints.ai_protocol = IPPROTO_TCP;
	hints.ai_flags = AI_PASSIVE;

	// Resolve the server address and port
	iResult = getaddrinfo(NULL, PORT, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return 1;
	}

	// Create a SOCKET for the server to listen for client connections.
	ListenSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ListenSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return 1;
	}

	// Setup the TCP listening socket
	iResult = bind(ListenSocket, result->ai_addr, (int)result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	freeaddrinfo(result);

	iResult = listen(ListenSocket, SOMAXCONN);
	if (iResult == SOCKET_ERROR) {
		printf("listen failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	// Accept a client socket
	ClientSocket = accept(ListenSocket, NULL, NULL);
	if (ClientSocket == INVALID_SOCKET) {
		printf("accept failed with error: %d\n", WSAGetLastError());
		closesocket(ListenSocket);
		WSACleanup();
		return 1;
	}

	std::cout << "The server has created a socket and accept a client socket!" << std::endl;

	// No longer need server socket
	closesocket(ListenSocket);

	const int usersCount = 3;
	User users[usersCount];
	User userRegOrAuth;
	static int index = 0;
	char usersFull[MESSAGE_BUFFER] = "usersFull";
	char usersNotFull[MESSAGE_BUFFER] = "usersNotFull";
	char successfulUserAuthorization[MESSAGE_BUFFER] = "successfulUserAuthorization";
	char failedUserAuthorization[MESSAGE_BUFFER] = "failedUserAuthorization";

	do {

		std::cout << "The server is waiting for a message.." << std::endl;
		iResult = recv(ClientSocket, recvbuf, MESSAGE_BUFFER, 0);
		
		if (iResult > 0) {
			/*if (strcmp(recvbuf, endSession_string) == 0)
			{
				std::cout << "End Session String" << std::endl;
				// shutdown the connection since we're done
				iResult = shutdown(ClientSocket, SD_SEND);
				if (iResult == SOCKET_ERROR) {
					printf("shutdown failed with error: %d\n", WSAGetLastError());
					closesocket(ClientSocket);
					WSACleanup();
					return 1;
				}
				closesocket(ClientSocket);
				WSACleanup();
				return 0;
			} */
			recvbuf[iResult - 1] = '\0';
			if (strcmp(recvbuf, signUp_string) == 0)
			{
				if (index < usersCount)
				{
					iSendResult = send(ClientSocket, usersNotFull, MESSAGE_BUFFER, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
					}
					std::cout << "Server sent usersNotFull and wait for data" << std::endl;

					message_size = recv(ClientSocket, bufferName, recvbuflen, 0);
					bufferName[message_size] = '\0';
					std::cout << "Message Received from Client (name): " << bufferName << std::endl;

					message_size = recv(ClientSocket, bufferLogin, MESSAGE_BUFFER, 0);
					bufferLogin[message_size] = '\0';
					std::cout << "Message Received from Client (login): " << bufferLogin << std::endl;

					message_size = recv(ClientSocket, bufferPassword, MESSAGE_BUFFER, 0);
					bufferPassword[message_size] = '\0';
					std::cout << "Message Received from Client (password): " << bufferPassword << std::endl;

					users[index] = userRegOrAuth.sign_up(bufferName, bufferLogin, bufferPassword);
					std::cout << "The server has created a new user!: " << users[index]._my_name << std::endl;
					index++;
				}
				else
				{
					std::cout << "Maximum number of users reached: " << usersCount << std::endl;
					iSendResult = send(ClientSocket, usersFull, MESSAGE_BUFFER, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
					}

				}
			}
			if (strcmp(recvbuf, signIn_string) == 0)
			{
				std::cout << "The server is authenticating.." << std::endl;

				message_size = recv(ClientSocket, bufferLogin, MESSAGE_BUFFER, 0);
				bufferLogin[message_size] = '\0';
				std::cout << "Message Received from Client (login): " << bufferLogin << std::endl;

				message_size = recv(ClientSocket, bufferPassword, MESSAGE_BUFFER, 0);
				bufferPassword[message_size] = '\0';
				std::cout << "Message Received from Client (password): " << bufferPassword << std::endl;

				userRegOrAuth = userRegOrAuth.sign_in(users, usersCount, bufferLogin, bufferPassword);
				if (userRegOrAuth._auth)
				{
					std::cout << "Successful authorization!" << std::endl;

					iSendResult = send(ClientSocket, successfulUserAuthorization, MESSAGE_BUFFER, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
					}

					while (userRegOrAuth._auth)
					{
						std::cout << "Waiting for an authorized user to select an action.." << std::endl;

						iResult = recv(ClientSocket, recvbuf, MESSAGE_BUFFER, 0);
						recvbuf[iResult - 1] = '\0';

						if (strcmp(recvbuf, logOut_string) == 0)
						{
							std::cout << "Logging out.." << std::endl;
							userRegOrAuth._auth = 0;
						}

						if (strcmp(recvbuf, messageSomeOne_string) == 0)
						{
							std::cout << "Waiting for a message.." << std::endl;

							message_size = recv(ClientSocket, bufferAddressee, MESSAGE_BUFFER, 0);
							bufferAddressee[message_size] = '\0';
							std::cout << "Message Received from Client (message recipient): " << bufferAddressee << std::endl;

							message_size = recv(ClientSocket, bufferReceivedMessage, MESSAGE_BUFFER, 0);
							bufferReceivedMessage[message_size] = '\0';
							std::cout << "Message Received from Client (message): " << bufferReceivedMessage << std::endl;

							userRegOrAuth.sendAMessage(users, usersCount, bufferAddressee, bufferReceivedMessage);
						}

						if (strcmp(recvbuf, messageAll_string) == 0)
						{
							std::cout << "Waiting for a message to be sent to all users.." << std::endl;

							message_size = recv(ClientSocket, bufferReceivedMessage, MESSAGE_BUFFER, 0);
							bufferReceivedMessage[message_size] = '\0';
							std::cout << "Message Received from Client (message to all): " << bufferReceivedMessage << std::endl;

							userRegOrAuth.SendAMessageToEveryone(users, usersCount, bufferReceivedMessage);

						}

						if (strcmp(recvbuf, checkingUserMessages_string) == 0)
						{
							for (int i = 0; i < usersCount; i++)
							{
								if (users[i]._user_data.first == userRegOrAuth._user_data.first)
								{
									std::cout << "User " << users[i]._my_name << " messages:" << std::endl;
									char* charMessage;
									std::string stringMessage;
									for (int k = 0; k < 5; k++)
									{
										std::cout << "Message #" << k + 1 << ": " << users[i]._my_messages[k] << std::endl;
										stringMessage = users[i]._my_messages[k];
										charMessage = &stringMessage[0];
										iSendResult = send(ClientSocket, charMessage, sizeof(charMessage), 0);
										if (iSendResult == SOCKET_ERROR) {
											printf("send failed with error: %d\n", WSAGetLastError());
										}
									}
									break;
								}
							}
						}
					}
				}
				else
				{
					std::cout << "Incorrect login or password" << std::endl;

					iSendResult = send(ClientSocket, failedUserAuthorization, MESSAGE_BUFFER, 0);
					if (iSendResult == SOCKET_ERROR) {
						printf("send failed with error: %d\n", WSAGetLastError());
					}
				}
			}
		}
		else if (iResult == 0)
			printf("Connection closing...\n");
		else {
			printf("recv failed with error: %d\n", WSAGetLastError());
			closesocket(ClientSocket);
			WSACleanup();
			return 1;
		}
	} while (iResult > 0);

	std::cout << "Server is Quitting" << std::endl;

	// shutdown the connection since we're done
	iResult = shutdown(ClientSocket, SD_SEND);
	if (iResult == SOCKET_ERROR) {
		printf("shutdown failed with error: %d\n", WSAGetLastError());
		closesocket(ClientSocket);
		WSACleanup();
		return 1;
	}

	// cleanup
	closesocket(ClientSocket);
	WSACleanup();
	return 0;
}