#include "Connection.h"

#define FAIL 1
#define SUCCESS 0

#define MALLOC(x) HeapAlloc(GetProcessHeap(), 0, (x))
#define FREE(x) HeapFree(GetProcessHeap(), 0, (x))


struct connInfo* Connection::getLocalIP() {
    struct connInfo* outConn = (connInfo*)malloc(sizeof(struct connInfo*));
    outConn->result = FAIL;

    PIP_ADAPTER_INFO pAdapterInfo;
    PIP_ADAPTER_INFO pAdapter = NULL;
    DWORD dwRetVal = 0;

    ULONG ulOutBufLen = sizeof(IP_ADAPTER_INFO);
    pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(sizeof(IP_ADAPTER_INFO));
    if (pAdapterInfo == NULL) {
        std::printf("Error allocating memory needed to call GetAdaptersinfo\n");
        return outConn;
    }

    // Make an initial call to GetAdaptersInfo to get
    // the necessary size into the ulOutBufLen variable
    if (GetAdaptersInfo(pAdapterInfo, &ulOutBufLen) == ERROR_BUFFER_OVERFLOW) {
        FREE(pAdapterInfo);
        pAdapterInfo = (IP_ADAPTER_INFO*)MALLOC(ulOutBufLen);
        if (pAdapterInfo == NULL) {
            std::printf("Error allocating memory needed to call GetAdaptersinfo\n");
            return outConn;
        }
    }

    if ((dwRetVal = GetAdaptersInfo(pAdapterInfo, &ulOutBufLen)) == NO_ERROR) {

        char* ipAddress = NULL;
        pAdapter = pAdapterInfo;
        while (pAdapter) {

            ipAddress = pAdapter->IpAddressList.IpAddress.String;
            if (strcmp(ipAddress, "0.0.0.0") == 1) {
                outConn->private_addr = ipAddress;
                outConn->result = SUCCESS;
                return outConn;
            }
            pAdapter = pAdapter->Next;
        }
    }
    else {
        std::printf("GetAdaptersInfo failed with error: %d\n", dwRetVal);
    }
    if (pAdapterInfo)
        FREE(pAdapterInfo);

    return outConn;

}

struct connInfo* Connection::checkIfIPChanged(struct connInfo coninfo) {

    struct connInfo* outConn = (connInfo*)malloc(sizeof(struct connInfo*));

    outConn->private_addr = coninfo.private_addr;
    outConn->result = FAIL;

    std::ifstream myfile;
    myfile.open("portAssignments.txt");

    if (myfile.is_open() == 1)
    {
        std::string line;

        int found = FAIL;
        int lineCount = 4;

        for (int x = 0; x < lineCount; x++) {

            std::getline(myfile, line);
            //first find private IP
            if (x == 0) {
                if (line.find(coninfo.private_addr) != std::string::npos) {
                    found = SUCCESS;
                    outConn->result = SUCCESS;
                }
            }

            //second find public IP
            if (x == 1) {

                const char* c = line.c_str();
                int sze = std::strlen(c) + 1;
                char* cc = new char[sze];

                strcpy_s(cc, sze, c);

                outConn->public_addr = cc;
            }

            //first find private Port
            if (x == 2) {
                outConn->privatePort = std::stoi(line);
            }

            //second find public Port
            if (x == 3) {
                outConn->publicPort = std::stoi(line);
            }
        }

        //remove current entry within the NAT
        if (found == FAIL) {

            std::printf("Found a current opening in the nat, closing now...\n");

            LPSTARTUPINFOA si = { };
            PROCESS_INFORMATION pi = { };

            char command[50];
            sprintf_s(command, 50, "./upnpc -i -d %s UDP", std::to_string(coninfo.publicPort).c_str());
            std::printf(command);

            if (CreateProcessA("Resources/bin/upnpc.exe", command, NULL, NULL, TRUE, 0, NULL, NULL, si, &pi) != 0) {
                std::printf("closed\n");
            }

            // Wait until child process exits.
            WaitForSingleObject(pi.hProcess, INFINITE);

            // Close process and thread handles. 
            CloseHandle(pi.hProcess);
            CloseHandle(pi.hThread);

            myfile.close();
        }
    }
    else {
        std::printf("found no entries\n");
    }

     return outConn;
}

struct connInfo* Connection::getForwardedPort(char* localIP) {

    // int publicPortAssignment = (rand() % (65535 - 1024)) + 1024;
    // int privatePortAssignment = (rand() % (65535 - 1024)) + 1024;
    int publicPortAssignment = 6557;
    int privatePortAssignment = 6557;

    struct connInfo* outConn = (connInfo*)malloc(sizeof(struct connInfo*));
    outConn->result = FAIL;
    outConn->private_addr = localIP;
    //Create handles that will allow for the creading of the child proceses' command prompt
    HANDLE hStdInPipeRead = NULL;
    HANDLE hStdInPipeWrite = NULL;
    HANDLE hStdOutPipeRead = NULL;
    HANDLE hStdOutPipeWrite = NULL;

    int ok = 1;

    // Create two pipes.
    SECURITY_ATTRIBUTES sa = { sizeof(SECURITY_ATTRIBUTES), NULL, TRUE };
    ok = CreatePipe(&hStdInPipeRead, &hStdInPipeWrite, &sa, 0);
    if (ok == FALSE) return outConn;
    ok = CreatePipe(&hStdOutPipeRead, &hStdOutPipeWrite, &sa, 0);
    if (ok == FALSE) return outConn;

    //binding these values to its respective variables
    STARTUPINFOA si = { };
    si.cb = sizeof(STARTUPINFOA);
    si.dwFlags = STARTF_USESTDHANDLES;
    si.hStdError = hStdOutPipeWrite;
    si.hStdOutput = hStdOutPipeWrite;
    si.hStdInput = hStdInPipeRead;

    //Must be closed after process has been ended
    PROCESS_INFORMATION pi = { };
    char command[50];

    printf("%s\n", localIP);

    sprintf_s(command, 50, "./upnpc -i -a %s %d %d UDP", localIP, publicPortAssignment, privatePortAssignment);

    printf("%s\n", command);

    std::printf("attempting to assign...\n");


    //Make this point within the project
    if (CreateProcessA("C:/upnpc.exe", command, NULL, NULL, TRUE, 0, NULL, NULL, &si, &pi) != 0)
    {
        //Wait for results
        std::this_thread::sleep_for(std::chrono::seconds(8));

        CloseHandle(hStdOutPipeWrite);
        CloseHandle(hStdInPipeRead);

        // The main loop for reading output from the DIR command.
        char buf[1024] = { };
        DWORD dwRead = 0;
        DWORD dwAvail = 0;
        ok = ReadFile(hStdOutPipeRead, buf, 1024, &dwRead, NULL);
        while (ok == TRUE)
        {
            buf[dwRead] = '\0';
            ok = ReadFile(hStdOutPipeRead, buf, 1024, &dwRead, NULL);
        }

        // Clean up and exit.
        CloseHandle(hStdOutPipeRead);
        CloseHandle(hStdInPipeWrite);
        DWORD dwExitCode = 0;

        short success = FAIL;

        std::string str = std::string(buf);
        std::string success_marker = std::string("is redirected");
        std::string publicIP_marker = std::string("external ");


        if ((str.find(success_marker) != std::string::npos)) {

            int index = str.find(publicIP_marker);

            std::string stringMarker;
            stringMarker.append(":")
                .append(std::to_string(publicPortAssignment).c_str())
                .append(" UDP");

            int index2 = str.find(stringMarker);

            if (index != std::string::npos && index2 != std::string::npos)
            {
                std::printf("found public IP address!\n");

                std::string publicIP = str.substr(index + 9, index2 - (index + 9));

                const char* c = publicIP.c_str();
                int sze = std::strlen(c) + 1;
                char* cc = new char[sze];

                strcpy_s(cc, sze, c);

                std::printf("%s\n", cc);

                outConn->public_addr = cc;
                success = SUCCESS;
            }

        }
        else {
            std::printf("could not find public IP address\n");
        }

        if (success == SUCCESS) {
            outConn->result = SUCCESS;
            outConn->privatePort = privatePortAssignment;
            outConn->publicPort = publicPortAssignment;
        }
    }
    else {
        std::printf("Error Executing Command!\n");

        CloseHandle(hStdOutPipeWrite);
        CloseHandle(hStdInPipeRead);
        CloseHandle(hStdOutPipeRead);
        CloseHandle(hStdInPipeWrite);
    }

    // Wait until child process exits.
    WaitForSingleObject(pi.hProcess, INFINITE);

    // Close process and thread handles. 
    CloseHandle(pi.hProcess);
    CloseHandle(pi.hThread);

    std::printf("returning\n");
    return outConn;
}

int Connection::savePorts(struct connInfo coninfo) {

    printf("Saving:");
    printf(" %s", coninfo.private_addr);
    printf(" %s", coninfo.public_addr);
    printf(" %d", coninfo.privatePort);
    printf(" %d\n", coninfo.publicPort);

    printf("Does an entry alreaady exist?\n");
    std::ifstream infile("portAssignments.txt");
    if (infile.is_open())
    {
        printf("yes\n");

        printf("Invalid forward no longer in the NAT\n");

        if (!remove("portAssignments.txt")) {
            std::cout << "file not found.\n";
            return FAIL;
        }

        std::cout << "Deleted\n";
    }
    else {
        printf("naa\n");
    }
    infile.close();


    std::ofstream outfile ("portAssignments.txt");
    outfile << coninfo.private_addr << "\n";
    outfile << coninfo.public_addr << "\n";
    outfile << coninfo.privatePort << "\n";
    outfile << coninfo.publicPort << "\n";

    outfile.close();

    //free(&infile);
    //free(&outfile);

    return SUCCESS;
}

bool Connection::start() {
    IPInfo = getLocalIP();

    struct connInfo* status = IPInfo;
    if (status->result == FAIL) {
        std::printf("Get local IP FAILURE!!");
        return 1;
    }

    status = checkIfIPChanged(*IPInfo);
    if (status->result == FAIL) {
        std::printf("IP address changed!!\n\n\n\n\n");
        //create new port assignments
        std::printf("assigning ports...\n");

        int retryCount = 3;
        for (int x = 0; x < retryCount; x++) {
            status = getForwardedPort(IPInfo->private_addr);
            if (status->result == SUCCESS) {
                std::printf("Successfully added ports: pub %d, pri %d\n", status->publicPort, status->privatePort);
                std::printf("Successfully added addr: pub %s\n", status->public_addr);

                IPInfo->public_addr = status->public_addr;
                IPInfo->privatePort = status->privatePort;
                IPInfo->publicPort = status->publicPort;
                break;
            }

            if (x == 2) {
                std::printf("Could not forward port.\n");
                return 1;
            }

            std::printf("retrying to assign ports\n");
        }

        //over-write all exist entries and create new ones that are good  
        if (savePorts(*IPInfo) == FAIL) {
            std::printf("could not save ports, oh well.\n");
        }

        std::printf("Succesfully saved ports\n");
    }
    else {
        std::printf("IP address hasn't been changed moving forward!\n");
        //use existing port assignments
        IPInfo = status;
    }

    connectionStatus = READY;
}

bool Connection::ready() {
    return connectionStatus == READY;
}
