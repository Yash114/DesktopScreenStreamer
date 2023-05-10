#undef UNICODE
#define WIN32_LEAN_AND_MEAN

#include "Server.h"
#include "Control.h"
#include <random>

int True = 1;
int False = 0;

//number of packets sent before verification is needed

int c = False;

int next = 0;

char buffer[1024];

struct sockaddr_in server_addr;
int server_struct_length = sizeof(server_addr);

const char* key = "ggboiconn";

long long s;

char* capt;

SOCKET ClientSocket = INVALID_SOCKET;

void Server::print(const char* message) {
	printf("%d : %s\n", frame, message);
}

void Server::waitTillConnect() {

	connectionStat = CONNECTING;
	std::memset(buffer, '\0', sizeof(buffer));


	//Wait for the key message for connection
	while (std::strcmp(buffer, key) != 0) {
		recvfrom(ClientSocket, buffer, 1024, 0, (struct sockaddr*)&server_addr, &server_struct_length);
	}

	//continuously send "ok" 
	std::thread send_ok_loop([this] {
		while (connectionStat == CONNECTING) {
			sendto(ClientSocket, key, strlen(key), 0, (struct sockaddr*)&server_addr, server_struct_length);
			std::this_thread::sleep_for(std::chrono::seconds(1));
		}
		});

	memset(buffer, '\0', sizeof(buffer));

	//Wait until you recieve annother message from this host
	while (connectionStat == CONNECTING) {
		recvfrom(ClientSocket, buffer, 1024, 0, (struct sockaddr*)&server_addr, &server_struct_length);
		if (strlen(buffer) > 0) {
			printf("connnected!\n");
			connectionStat = CONNECTED;
		}
	}

	send_ok_loop.join();

	return;
}

bool Server::initServer(Connection& connection) {

	connInfo* conninfo = connection.IPInfo;

	printf("creating server... w/ ");
	printf("addr: %s, ", conninfo->public_addr);
	printf("port: %d\n", conninfo->publicPort);

	WSADATA wsaData;
	int iResult = NULL;
	struct addrinfo* result = NULL,
		* ptr = NULL, hints;


	// Initialize Winsock
	iResult = WSAStartup(MAKEWORD(2, 2), &wsaData);
	if (iResult != 0) {
		printf("WSAStartup failed with error: %d\n", iResult);
		return FAIL;
	}

	ZeroMemory(&hints, sizeof(hints));
	hints.ai_family = PF_INET;
	hints.ai_socktype = SOCK_DGRAM;
	hints.ai_protocol = IPPROTO_UDP;

	std::string n = std::to_string(conninfo->privatePort);
	const char* p = n.c_str();


	iResult = getaddrinfo(conninfo->private_addr, p, &hints, &result);
	if (iResult != 0) {
		printf("getaddrinfo failed with error: %d\n", iResult);
		WSACleanup();
		return FAIL;
	}


	// Create a SOCKET for the server to listen for client connections.
	ClientSocket = socket(result->ai_family, result->ai_socktype, result->ai_protocol);
	if (ClientSocket == INVALID_SOCKET) {
		printf("socket failed with error: %ld\n", WSAGetLastError());
		freeaddrinfo(result);
		WSACleanup();
		return FAIL;
	}


	iResult = ::bind(ClientSocket, result->ai_addr, result->ai_addrlen);
	if (iResult == SOCKET_ERROR) {
		printf("bind failed with error: %d\n", WSAGetLastError());
		freeaddrinfo(result);
		closesocket(ClientSocket);
		WSACleanup();
		return FAIL;
	}

	//You do not listen for a connection on UDP 
	//You simply wait for a string of data
	//using
	//bytes_received = recvfrom(serverSocket, serverBuf, serverBufLen, 0 /* no flags*/, (SOCKADDR *) & SenderAddr, &SenderAddrSize);

	if (ClientSocket != INVALID_SOCKET) {

		//What I'll do is wait for a particular string of information.
		//Then send back annother special key
		//This will establish the "connection"
		printf("waiting... \n");

		////Blocking call that will resume once a connection is made.
		waitTillConnect();

	}
	else {
		printf("couldn't open socket!!\n");
	}


	return SUCCESS;
}
//closesocket(ClientSocket);

char ss[20];
char* i;
int y;

char* sha;

std::vector<uint8_t> dataBuffer;
int counter = 0;

int p = 0;

std::string packetedSign1("xcof");
std::string unpacketedSign1("p7vej");
std::string verificationSignal("bvuiwo");

std::string frameCompletionSignal("s5drcy");
std::string resendSuccessSignal("ygwbeihv");
std::string signatureVerifiedSignal("vndsio");
std::string signatureNotVerifiedSignal("aubielew");
std::string sendDataToDecoder("gwnieo");
std::string dataFinished("fgbewui");

bool Server::CreateOutputMessageString(ServerData* data, CHAR* output, char* signature) {

	int totalSize = data->size;
	int sizeLeft = data->size;

	char* i = reinterpret_cast<char*>(data->data);


	if (i == nullptr) { return false; }


	std::string sigString = sha1(data->data, data->size);
	std::memcpy(signature, (char*) sigString.c_str(), 40);

	//Send data in packets
	int u = 0;
	int packetCount = 0;

	dataBuffer.clear();

	//printf("%d\n", totalSize);

	while (sizeLeft >= pktsize) {

		//data marker
		dataBuffer.insert(dataBuffer.begin() + dataBuffer.size(),
			packetedSign1.c_str(), packetedSign1.c_str() + packetedSign1.length());

		//actual data
		dataBuffer.insert(dataBuffer.begin() + dataBuffer.size(),
			i + u, i + u + pktsize - packetedSign1.length());

		//Move to next data []
		sizeLeft -= pktsize - packetedSign1.length(); //total packsize minus signal
		u += pktsize - packetedSign1.length();

		packetCount++;
	}

	//Sends data that is not in a packet
	//flag for !defualt size
	dataBuffer.insert(dataBuffer.begin() + dataBuffer.size(),
		unpacketedSign1.c_str(), unpacketedSign1.c_str() + unpacketedSign1.length());

	//puts size info
	int sizelength = std::to_string(sizeLeft).size();
	sprintf_s(ss, "0000000%d", sizeLeft);
	dataBuffer.insert(dataBuffer.begin() + dataBuffer.size(),
		&ss[sizelength], &ss[sizelength + 7]);

	//puts data
	dataBuffer.insert(dataBuffer.begin() + dataBuffer.size(),
		i + u, i + u + sizeLeft);

	output->data = (char*)dataBuffer.data();
	output->dataSize = dataBuffer.size();
	output->otherData = sizeLeft + 7 + unpacketedSign1.length();

	print("Created Frame");

	return true;
}

void Server::messageRecvLoop() {
	while (connectionStat == CONNECTED) {

		memset(buffer, '\0', sizeof(buffer));
		recvfrom(ClientSocket, buffer, sizeof(buffer), 0, (struct sockaddr*)&server_addr, &server_struct_length);

		//writability = &LOCKED;
		processMessage(buffer);
		//writability = &UNLOCKED;

	}
}

bool Server::readNextMessage(Server::Message* output) {

	if (currentEntry == nullptr) { return false; }

	memcpy(&output->data[0], currentEntry->data, 15);
	output->messageID = currentEntry->messageID;

	Server::Message* newFirst = currentEntry;

	//while (writability == &LOCKED) {}

	currentEntry = newFirst->next;

	delete newFirst;

	return true;
}

void Server::processMessage(char* message) {

	//If the first digit is a number (indicating an ID)
	if (message[0] >= 48 && message[0] <= 57) {

		int val = 0;
		for (int jk = 0; jk < 5; jk++) {
			val = val * 10 + *(message + jk) - '0';
		}

		//If the linked list is empty
		if (currentEntry == nullptr) {


			std::string stringMessage(message);

			currentEntry = new Message;
			memcpy(currentEntry->data, stringMessage.data(), stringMessage.size());

			currentEntry->messageID = val;
			currentEntry->next = nullptr;

			finalEntry = currentEntry;

		}
		else { /* If the linked list is not empty */

			struct Message* tempEntry = new Message;

			std::string stringMessage(message);

			memcpy(tempEntry->data, stringMessage.data(), stringMessage.size());

			tempEntry->messageID = val;
			tempEntry->next = nullptr;

			finalEntry->next = tempEntry;

			finalEntry = tempEntry;

		}
	}
	else {

		if (message[0] == -56) {
			Control().sendMouse(message[1], message[2]);

		}
		else if (message[0] == -64) {
			Control().sendMouseButton(message[1]);
		}
		else if (message[0] == -96) {
			Control().sendInput(message[1]);
		}
	}


}

void Server::SendCharData(Server::CHAR* data, char* signature) {

	int dataPointer = 0;
	bool verified = false;

	do {

		if (data->dataSize >= pktsize) {
			for (int packet = 0; packet < floor(data->dataSize / pktsize); packet++) {

				print(" - Sending Packet");
				sendWithoutID((char*)(data->data + dataPointer), pktsize);
				dataPointer += pktsize;
			}
		}

		print(" - Sending Unpacketed Data");
		sendWithoutID((char*)(data->data + dataPointer), data->dataSize - dataPointer);

		print("Verifying");

 		verified = VerifyFrameWasSentSuccessfully(signature);
		print(verified ? "Group Verified" : "Group Not Verified Sending Again...");

		dataPointer = 0;

	}  while (!verified);


}


bool Server::SendServerData(ServerData* data) {

	struct Server::CHAR output;

	char signature[41] = {};

	if (!CreateOutputMessageString(data, &output, &signature[0])) { return false; }

	SendCharData(&output, &signature[0]);

	print("Frame Done\n");

	frame++;

}

bool Server::AwaitData(char* output) {

	struct Server::Message recievedMessage;

	//printf("Waiting for: %d\n", ID);

	do {

		if (!readNextMessage(&recievedMessage)) { return false; }

	} while (recievedMessage.messageID != ID);

	memcpy(output, recievedMessage.data + 5, 15);

	return true;
}

bool Server::AddSignature(char* data, int dataSize, Server::CHAR* output) {

	ID++;
	ID % 99999;

	//create the message ID (randomly generated)
	sizeLength = std::to_string(ID).size();
	sprintf_s(messageID, "00000%d", ID);

	dataOUT.clear();

	//insert the new message into the buffer
	dataOUT.insert(dataOUT.begin(), &messageID[sizeLength], &messageID[5 + sizeLength]);
	dataOUT.insert(dataOUT.begin() + 5, data, data + dataSize);

	output->data = dataOUT.data();
	output->dataSize = dataOUT.size();

	return true;

}


int messageNumber = 0;


//Send without ID because most Data doesnt need to be verified
void Server::sendWithoutID(char* data, int dataSize) {

	y = sendto(ClientSocket, data, dataSize + 25, 0, (struct sockaddr*)&server_addr, server_struct_length);
	if (y == SOCKET_ERROR) {			
		printf("error: %d\n", WSAGetLastError());
	}

	print(" - Sent");
}


//Send with ID to ensure the message was actually sent
void Server::sendWithID(char* data, int dataSize, char* result) {

	bool success = true;
	Server::CHAR output;
	AddSignature(data, dataSize, &output);

	//continuously send the same thing until it receives a response with the same message ID
	do {

		//send the message out to the client
		y = sendto(ClientSocket, output.data, output.dataSize, 0, (struct sockaddr*)&server_addr, server_struct_length);
		if (y == SOCKET_ERROR) {
			printf("error: %d\n", WSAGetLastError());
		}

		print(" - Sent");

		success = AwaitData(result);

		if (!success)
			std::this_thread::sleep_for(std::chrono::milliseconds(1));

	} while (!success);

	print(" - Data Received");
}

bool Server::VerifyFrameWasSentSuccessfully(char* signature) {

	const int headerSize = 40 + verificationSignal.length();
	char shaOutput[50] = {};

	sprintf_s(shaOutput, "%s%s", verificationSignal.c_str(), signature);

	char result[15] = {};
	sendWithID(shaOutput, headerSize, result);

	//Sends the signature and outputs if the returned signal was verified
	return memcmp(result,
		signatureVerifiedSignal.c_str(),
		signatureVerifiedSignal.length()) == 0;


}