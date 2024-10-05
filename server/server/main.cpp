#include <iostream>
#include <WinSock2.h>
#include <WS2tcpip.h>
#include <tchar.h>
#include <thread>
#include <vector>

using namespace std;

#pragma comment(lib, "ws2_32.lib")


/*

1. Initialize winsock library
2. Create socket
3. Get ip and port
4. Bind the ip/port with the socket
5. Listen on the socket
6. Accept
6. Receive and send
7. CLose the socket
8. Cleanup the winsock

*/
bool Initialize() {
	WSADATA data;
	return WSAStartup(MAKEWORD(2, 2), &data) == 0;
}

void InteractWithCLient(SOCKET clientSocket, vector<SOCKET> &clients) {
	// send/ recv client 
	cout << "Client connected" << endl;
	char buffer[4096];

	while (1) {

		int bytesrecvd = recv(clientSocket, buffer, sizeof(buffer), 0);

		if (bytesrecvd <= 0) {
			cout << "Client Disconnect" << endl;
			break;
		}

		string message(buffer, bytesrecvd);
		cout << " Message from the client : " << message << endl;

		for (auto client : clients) {
			if (client != clientSocket) {
				send(client, message.c_str(), message.length(), 0);
			}
		}
	}
	
	auto it = find(clients.begin(), clients.end(), clientSocket);
	if (it != clients.end()) {
		clients.erase(it);
	}


	closesocket(clientSocket);

}

int main() {
	if (!Initialize()) {
		cout << "Winsock initialization failed" << endl;
	}
	cout << "Server program" << endl;

	SOCKET listenSocket = socket(AF_INET, SOCK_STREAM, 0);

	if (listenSocket == INVALID_SOCKET) {
		cout << "Socket connection failed" << endl;
		return 1;
	}

	// create address structure
	int port = 12345;
	sockaddr_in serveraddr;
	serveraddr.sin_family = AF_INET;
	serveraddr.sin_port = htons(port);

	// COnvert the ipaddress (0.0.0.0) put it inside the sin_family in binary format

	if (InetPton(AF_INET, _T("0.0.0.0"), &serveraddr.sin_addr) != 1) {
		cout << "Setting address structure failed" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Bind

	if (bind(listenSocket, reinterpret_cast<sockaddr*>(&serveraddr), sizeof(serveraddr)) == SOCKET_ERROR ){
		cout << "Bind failed" << endl;

		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	// Listen
	if (listen(listenSocket, SOMAXCONN) == SOCKET_ERROR) {
		cout << "Listen Error" << endl;
		closesocket(listenSocket);
		WSACleanup();
		return 1;
	}

	cout << "Server has started listening on port : "<< port  << endl;
	vector<SOCKET> clients;

	
	while (1) {
		// Accept

		SOCKET clientSocket = accept(listenSocket, nullptr, nullptr);
		if (clientSocket == INVALID_SOCKET) {
			cout << "Invalid client socket" << endl;
		}
		clients.push_back(clientSocket);

		thread t1(InteractWithCLient, clientSocket, std::ref(clients));
		t1.detach();
	}

	


	closesocket(listenSocket);


	WSACleanup();
	return 0;

}