//#pragma once
#include "Connection.h"

#include <hash-library/sha1.h>

#pragma comment(lib, "IPHLPAPI.lib")
#pragma comment (lib, "Ws2_32.lib")

class Server
{
	public:
		bool initServer(Connection& connection);
		bool runServerV2(ServerData*, bool*);
		bool SendServerData(ServerData*);
		bool endServer();

		int connectionStat = NOTREADY;
		int frame = 0;

		void sendWithID(char*, int, char*);
		void sendWithoutID(char*, int);

		void messageRecvLoop();

		struct CHAR {
			char* data = nullptr;
			int dataSize = 0;
			int otherData = 0;
		};

		struct Message {
			char* data = { new char[15] };
			int messageID = -1;
			struct Message* next = nullptr;
		};

		int groupSize = 12;
		int pktsize = 1000;

		double resendRate = 0;

	private:
		char** messageBuffer = new char* [5];
		int messageCounter = 0;

		fd_set readfds, masterfds;
		struct timeval timeout;

		std::vector<char> dataOUT;
		char messageID[15] = {};

		int ID;
		int sizeLength;

		SHA1 sha1;
		void waitTillConnect();
		bool CreateOutputMessageString(ServerData*, Server::CHAR*, char*);
		void SendCharData(Server::CHAR*, char*);
		bool VerifyFrameWasSentSuccessfully(char*);
		bool AwaitData(char*);
		bool AddSignature(char*, int, Server::CHAR*);
		void print(const char*);

		void sendFullPacketWithID(char* data);

		void sendPartPacketWithID(char* data, int size);

		void processMessage(char*);
		bool readNextMessage(Message*);

		struct Message* finalEntry = nullptr;
		struct Message* currentEntry = nullptr;

		int LOCKED = 0;
		int UNLOCKED = 1;

		int* writability = &UNLOCKED;

};

