#pragma once
#include "Common.h"

#include <curl/curl.h>
#include <iphlpapi.h>
#include <filesystem>
#include <chrono>
#include <thread>
#include <fstream>
#include <processthreadsapi.h>

#pragma comment(lib, "IPHLPAPI.lib")

#define READY 0
#define DISCONNECTED 1
#define CONNECTED 2
#define CONNECTING 3
#define NOTREADY 4

struct connInfo {
	int result = FAIL;
	int privatePort = -1;
	int publicPort = -1;
	char* private_addr = NULL;
	char* public_addr = NULL;
};

class Connection
{
	public:
		struct connInfo* IPInfo;

		bool ready();
		bool start();
		bool open();

	private:
		int connectionStatus; 
		struct connInfo* getLocalIP();
		struct connInfo* checkIfIPChanged(struct connInfo);
		struct connInfo* getForwardedPort(char*);
		int savePorts(struct connInfo);
};

