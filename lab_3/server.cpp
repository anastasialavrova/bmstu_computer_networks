#include <stdlib.h>
#include <stdio.h>
#include <sys/socket.h>
#include <sys/types.h>
#include <netinet/in.h>
#include <arpa/inet.h>
#include <unistd.h>
#include "general.h"
#include <string.h>

#include <chrono>

#include <stdio.h>
#include <string>
#include <iostream>
#include <fstream>
#include <thread>
#include <cstdlib> // For exit() and EXIT_FAILURE
#include <iostream> // For cout
#include <unistd.h> // For read
#include <thread> // std::thread
#include <vector> // std::vector
#include <queue> // std::queue
#include <mutex> // std::mutex
#include <condition_variable> // std::condition_variable



using namespace std;

struct statusCodes
{
  char* statusCode;
  char* message;
};

struct existingURLS
{
  char* url;
  char* htmlPage;
};


static statusCodes OKCode = {"200", "OK"};
static statusCodes NotFoundCode = {"404", "Not Found"};
static statusCodes IternalErrorCode = {"500", "Iternal error"};



static existingURLS existingURI[3] = {{"/test.html", "test.html"},{"/mm.html", "mm.html"},{"klimat05.html", "<div><h1> hello world</h1></div>"}};



string clientHandler(char *message);
// This class manages a thread pool that will process requests
class ThreadPool {
 public:
  ThreadPool() : done(false) {
    // This returns the number of threads supported by the system. If the
    // function can't figure out this information, it returns 0. 0 is not good,
    // so we create at least 1
    auto numberOfThreads = std::thread::hardware_concurrency();
    if (numberOfThreads == 0) {
      numberOfThreads = 1;
    }

    for (unsigned i = 0; i < numberOfThreads; ++i) {
      // The threads will execute the private member `doWork`. Note that we need
      // to pass a reference to the function (namespaced with the class name) as
      // the first argument, and the current object as second argument
      threads.push_back(std::thread(&ThreadPool::doWork, this));
    }
  }

  // The destructor joins all the threads so the program can exit gracefully.
  // This will be executed if there is any exception (e.g. creating the threads)
  ~ThreadPool() {
    // So threads know it's time to shut down
    done = true;

    // Wake up all the threads, so they can finish and be joined
    workQueueConditionVariable.notify_all();
    for (auto& thread : threads) {
      if (thread.joinable()) {
        thread.join();
      }
    }
  }

  // This function will be called by the server, every time there is a request
  // that needs to be processed by the thread pool
  void queueWork(int fd, char* request) {
    // Grab the mutex
    std::lock_guard<std::mutex> g(workQueueMutex);

    // Push the request to the queue
    workQueue.push(std::pair<int, char*>(fd, request));

    // Notify one thread that there are requests to process
    workQueueConditionVariable.notify_one();
  }

 private:
  // This condition variable is used for the threads to wait until there is work
  // to do
  std::condition_variable_any workQueueConditionVariable;

  // We store the threads in a vector, so we can later stop them gracefully
  std::vector<std::thread> threads;

  // Mutex to protect workQueue
  std::mutex workQueueMutex;

  // Queue of requests waiting to be processed
  std::queue<std::pair<int, char*>> workQueue;

  // This will be set to true when the thread pool is shutting down. This tells
  // the threads to stop looping and finish
  bool done;

  // Function used by the threads to grab work from the queue
  void doWork() {
    // Loop while the queue is not destructing
    while (!done) {
      std::pair<int, char*> request;

      // Create a scope, so we don't lock the queue for longer than necessary
      {
        std::unique_lock<std::mutex> g(workQueueMutex);
        workQueueConditionVariable.wait(g, [&]{
          // Only wake up if there are elements in the queue or the program is
          // shutting down
          return !workQueue.empty() || done;
        });

        request = workQueue.front();
        workQueue.pop();
      }

      processRequest(request);
    }
  }

  void processRequest(const std::pair<int, char*> item) {

    string s = clientHandler(item.second);
    
    send(item.first, s.c_str(), s.size(), 0);

    // Close the connection
    close(item.first);
  }
};


string getInfoFromFile(char* fileName)
{
  string fileRes = "";
  string line;
  std:ifstream in(fileName);
  if (in.is_open())
  {
    while (getline(in, line))
    {
        fileRes.append(line);
    }

    in.close();
  }
  return fileRes;
}

void saveUserStatistic(string userId, char* url)
{
  std:ofstream of;
  of.open(userId + ".txt", ios_base::app);
  if (of.is_open())
  {
    of << userId + " visited page: " + url << endl;

    of.close();
  }
}

void perror_and_exit(char *s, int exit_code)
{
	perror(s);
	exit(exit_code);
}

string generateResponseMessage(char* headers, int pageId)
{
  string resultMsg = "";
  statusCodes statusCode;
  if (pageId != -1)
  {
    statusCode = OKCode;
  }
  else
  {
    statusCode = NotFoundCode;
  }
  resultMsg.append(HttpVersion);
  resultMsg.append(" ");
  resultMsg.append(statusCode.statusCode);
  resultMsg.append(" ");
  resultMsg.append(statusCode.message);
  resultMsg.append("\r\n");
  resultMsg.append("Connection: closed\r\n");
  resultMsg.append("Content-Type: text/html; charset=UTF-8\r\n");
  resultMsg.append("\r\n");
  if (pageId != -1)
  {
    string fileRes = getInfoFromFile(existingURI[pageId].htmlPage);
    if (fileRes != "")
      resultMsg.append(fileRes);
  }
  return resultMsg;
}


string handleRequestMessage(char * innerMessage)
{
  char* method = strtok(innerMessage, " ");
  char* url = strtok(NULL, " ");
  char* httpVersion = strtok(NULL, "\r\n");
  char* userName = strtok(NULL, "\r\n");
  char* HostName = strtok(NULL, "\r\n");
  cout << "+++" << userName << "+++" << endl;
  char *Name = strstr(userName, ": ");
  
  if (strcmp(method, "GET") == 0)
  {
    int pageId = -1;
    for (int i = 0; i < 3; i++)
    {
      if (strcmp(url, existingURI[i].url) == 0)
      {
        saveUserStatistic((string)(Name + 2), url);
        pageId = i;
        break;
      }
    }
    char res[MESSAGE_LEN];
    //return "asd";
    
    return generateResponseMessage(innerMessage, pageId);
  }
  else
  {
    return "It's not Get";
  }
  
}


string clientHandler(char *message)
{
  printf("Client's message: %s\n", message);
  string res = handleRequestMessage(message);
        //cout <<"test;" << res << endl;
  const char * msg = res.c_str();
  printf("====================================\n");
  printf("Server's message:\n %s", msg);
  return msg;
  //send(sock, msg, strlen(msg), 0);
  //cout << "ok!" << endl;
  //close(sock);
}


int main()
{
    int listener;
    struct sockaddr_in addr, client_addr;
  
    listener = socket(AF_INET, SOCK_STREAM, 0);
    if(listener < 0)
		perror_and_exit("socket()", 1);
    
    addr.sin_family = AF_INET;
    addr.sin_port = htons(Port);
    addr.sin_addr.s_addr = htonl(INADDR_ANY);
    if(bind(listener, (struct sockaddr *)&addr, sizeof(addr)) < 0)
		perror_and_exit("bind()", 2);

    listen(listener, 10);
  	printf("Server is listening on %s:%d...\n", ServIp, Port);
    ThreadPool tp;
    while(1)
    {

      int sock;
      int bytes_read;
      char *buf = (char*)malloc(MESSAGE_LEN);
      socklen_t cli_addr_size = sizeof(client_addr);
      sock = accept(listener, (struct sockaddr*) &client_addr, &cli_addr_size);
      if(sock < 0)
		    perror_and_exit("accept()", 3);

      bytes_read = recv(sock, buf, MESSAGE_LEN, 0);
      buf[bytes_read] = '\0';
      char tst[MESSAGE_LEN];
      strcpy(tst, buf);

      tp.queueWork(sock, tst);
      free(buf);
      //std::thread thread(clientHandler, client_addr, listener);
      //clientHandler(client_addr,listener);
		  //thread.join();
    }
    close(listener);
    return 0;
}