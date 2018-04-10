/*                                          */
/* Author: Hayden Burnette                  */
/* Date  : 2/04/2018                        */
/*                                          */
/*                                          */
#include <iostream>
#include <unistd.h>
#include <stdlib.h>
#include <sys/wait.h>
#include <string.h>
#include <fstream>
#include <vector>
#include <semaphore.h>
#include <queue>

#ifndef LOG_H

#include "Log.h"

#endif
#ifndef SETUPDATA_H

#include "SetupData.h"

#endif
using namespace std;

#define STRLEN 32

struct Message {
    int id;            // Unique identification numbered
    char command;       // Which command   
    char key[16];       // Key value for the note   
    char payload[128];  // The short note
}; //end Message
struct parentArgs {
    Log *log;
    SetupData *setup;
    vector<Message> allMessages;
};

//
//functions
//
void OpenCommandFile(fstream &, const char *);

void ReadCommandFile(fstream &, vector<Message> &);

void CloseCommandFile(fstream &);

void PrintMessage(Message);

void writeLog(string, Log &);

void ProcessSetupFile(SetupData &, Log &);

void ProcessCommandFile(fstream &, const char *, vector<Message> &);

void LogCommand(Message, Log &);

void LogStart(Log &, SetupData &);

void LogEnd(Log &);

void initalizePipes();

bool CreateFile(fstream &, const char *, Message);

void CommandHandler(Log &, SetupData &, vector<Message> &);

Message loadMessage(int, vector<Message> &);

void pushPutStore(const Message &msg);

Message getReturnMessage();

void pushSearch(const Message &msg);

void pushNumber(const Message &msg);

void pushReturn(const Message &msg);

Message getPutStoreMessage();

Message getNumberMessage();

Message getSearchMessage();

//
//global variables
//
int pipeLogger[2];
sem_t putStoreSem;
sem_t searchSem;
sem_t numberSem;
sem_t returnSem;
sem_t processTask;
pthread_mutex_t putStoreLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t searchLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t numberLock = PTHREAD_MUTEX_INITIALIZER;
pthread_mutex_t returnLock = PTHREAD_MUTEX_INITIALIZER;
pthread_t pidSearch;
pthread_t pidNumber;
pthread_t pidPutStore;
pthread_t pidReturnProcess;
queue<Message> putStoreQueue;
queue<Message> searchQueue;
queue<Message> numberQueue;
queue<Message> returnMessageQueue;
fstream f;

//
// main( )
//
int main(int argc, char **argv) {
    Log log;                                                                                //new log for recording
    char dashP[STRLEN] = "", dashS[STRLEN] = "";                                           // getopt( ) requires char* arguments
    char c;                                                                                // For return value of getopt( )
    vector<Message> messages;                                                              //vector for holding messages read from command fil
    //there are no commands then display a message
    if (argc <= 1) {
        cout << "\nUsage: hw1 –p <pathame> [-s <setupfilename>]" << endl;
        exit(0);
    } // if

    //loop through -p, -s flags
    while ((c = getopt(argc, argv, "p:s:")) != -1) {
        switch (c) {
            case 'p': {
                strncpy(dashP, optarg,
                        STRLEN - 1);                                              //copy data from console into strings
            }
                break;
            case 's': {
                strncpy(dashS, optarg,
                        STRLEN - 1);                                          //copy data from console into strings
            }
                break;
        }; // switch
    } // while

    SetupData data(dashP,
                   dashS);                                                          //make new data object to store data
    ProcessSetupFile(data, log);
    ProcessCommandFile(f, data.getCommandfilename().c_str(), messages);
    string commandFilename = data.getCommandfilename();
    system("mkdir \"./keys\" >> /dev/null 2>&1");                                                                         //make directory for keys
  
    ////////////////////////////////////////////////////
    //Here is where we start our forking and piping process
    ////////////////////////////////////////////////////

    //initalize pipes
    initalizePipes();
    //call the main function
    CommandHandler(log, data, messages);
    cout << "\nProgram over" << endl;
} // main( )
//
//process a putStore command
//
void *runPutStore(void *) {
    Message msg;
    string newPayload;
    while (true) {
        sem_wait(&putStoreSem);
        msg = getPutStoreMessage();
        if(msg.command == 'q' || msg.command == 'Q')
            break;
        cout << "\nPut Store server has received msg #" << msg.id << " key: " << msg.key << ", Payload: "
             << msg.payload;

        string str = "./keys/" + string(msg.key);
        const char *k = str.c_str();
        bool suc = CreateFile(f, k, msg);
        if (suc == false) {
            newPayload = "(PUT_STORE) DUPLICATE key: " + string(msg.key);
            strcpy(msg.payload, newPayload.c_str());
        } else {
            newPayload = "(PUT_STORE) OKAY";
            strcpy(msg.payload, newPayload.c_str());
        }
        pushReturn(msg);
        sem_post(&returnSem);
    }

    cout << "\nPut Store server has received 'quit' command, killing process " << endl;

    pthread_exit(0);
}
//
//process a search command
//
void *runSeach(void *) {
    Message m;
    string str;
    string newPayload;
    while (true) {
        sem_wait(&searchSem);
        m = getSearchMessage();
        if(m.command == 'q' || m.command == 'Q')
            break;
        cout << "\nSearch server has received msg #" << m.id << " key: " << m.key << ", Payload: " << m.payload;

        system(string("find ./keys -name '" + string(m.key) + "' >> temp").c_str());
        f.open("temp", fstream::in);
        getline(f, newPayload);
        system("rm temp");
        f.close();
        if (newPayload == "")
            newPayload = "(SEARCH) FILE NOT FOUND WITH KEY: " + string(m.key);
        else {
            //get payload from key
            f.open(string("./keys/" + string(m.key)).c_str(), fstream::in);
            getline(f, str);
            f.close();
            newPayload = "(SEARCH) PAYLOAD AT KEY " + string(m.key) + " IS " + str;
        }
        strcpy(m.payload, newPayload.c_str());
        pushReturn(m);
        sem_post(&returnSem);
    }

    cout << "\nSearch server has received 'quit' command, killing process " << endl;

    pthread_exit(0);
}
//
//process a number command
//
void *runNumber(void *) {
    Message m;
    string newPayload;
    while (true) {
        sem_wait(&numberSem);
        m = getNumberMessage();
        if(m.command == 'q' || m.command == 'Q')
            break;
        //system("ls ./keys | wc -l");
        cout << "\nNumber server has received msg #" << m.id << " key: " << m.key << ", Payload: " << m.payload;
        system("ls ./keys | wc -l >> temp");
        f.open("temp", fstream::in);
        getline(f, newPayload);
        newPayload = "(NUMBER) THE NUMBER OF FILES STORED IS " + newPayload;
        strcpy(m.payload, newPayload.c_str());
        f.close();
        system("rm temp");
        pushReturn(m);
        sem_post(&returnSem);
    }

    cout << "\nNumber server has received 'quit' command, killing process " << endl;
    pthread_exit(0);
}
//
//this will handler everything
//
void CommandHandler(Log &log, SetupData &data, vector<Message> &messages) {
    Message m;                                                                             //
    Message msg;
    int counter = 0;
    struct parentArgs a;
    string newPayload;
    //fork logger process
    int logValue = fork();
    if (logValue < 0) {
        cout << "\nlogger fork failed to initalize. Please try again!" << endl;
        exit(0);
    } else if (logValue == 0) {
        LogStart(log, data);
        //fork success
        while (m.command != 'q' && m.command != 'Q'){
            read(pipeLogger[0], (char *) &m, sizeof(Message));
            LogCommand(m, log);

            cout << "\nLog server has logged msg #" << m.id << " key: " << m.key << ", Payload: " << m.payload;

        }
        //q or Q has been received so quit and close pipe
        cout << "\nLog server has received 'quit' command, killing process " << endl; 
        close(pipeLogger[0]);
        close(pipeLogger[1]);
        LogEnd(log);
        exit(0);
    } else {
        /////////////////////////////////////////////////////////
        //                   PARENT CODE                       //
        /////////////////////////////////////////////////////////
        //Spin off new processes with pthreads put store first
        a.log = &log;
        a.setup = &data;
        a.allMessages = messages;
      //initalize all of the semaphores
        sem_init(&returnSem, 0, 0);
        sem_init(&putStoreSem, 0, 0);
        sem_init(&numberSem, 0, 0);
        sem_init(&searchSem, 0, 0);
        sem_init(&processTask, 0, 1);
      //create all of the pthreads for each command
        pthread_create(&pidPutStore, NULL, runPutStore, NULL);
        pthread_create(&pidSearch, NULL, runSeach, NULL);
        pthread_create(&pidNumber, NULL, runNumber, NULL);

        while (msg.command != 'q' && msg.command != 'Q') {
            sem_wait(&processTask);
            //need to dequeue the return queue
            //log it
            msg = loadMessage(counter, messages);
            counter++;
            //log it
            write(pipeLogger[1], (char *) &msg, sizeof(Message));
            switch (msg.command) {
                //send message to put store server
                case 'P':
                case 'p': {
                  //process putStore and return message 
                    pushPutStore(msg);
                    sem_post(&putStoreSem);
                    sem_wait(&returnSem);
                    msg = getReturnMessage();
                    cout << "\nMost Recent Message from servers: " << msg.payload << endl;
                    sem_post(&processTask);
                }
                    break;
                    //send message to search server
                case 'S':
                case 's': {
                  //process search and return message 
                    pushSearch(msg);
                    sem_post(&searchSem);
                    sem_wait(&returnSem);
                    msg = getReturnMessage();
                    cout << "\nMost Recent Message from servers: " << msg.payload << endl;
                    sem_post(&processTask);
                }
                    break;
                    //send message to put number server
                case 'N':
                case 'n': {
                  //process number and return message 
                    pushNumber(msg);
                    sem_post(&numberSem);
                    sem_wait(&returnSem);
                    msg = getReturnMessage();
                    cout << "\nMost Recent Message from servers: " << msg.payload << endl;
                    sem_post(&processTask);
                }
                    break;
                    //tell all processes to quit
                case 'q':
                case 'Q': {
                  //end all
                    pushPutStore(msg);
                    sem_post(&putStoreSem);
                    pushSearch(msg);
                    sem_post(&searchSem);
                    pushReturn(msg);
                    sem_post(&returnSem);
                    pushNumber(msg);
                    sem_post(&numberSem);
                    msg = getReturnMessage();
                    cout << "\nShutting down return message server\n" << endl;
                }
                    break;
                    //if the command read is invalid, it just skips it
                default: {

                    cout << "\n\'" << msg.command << "\'"
                         << " IS NOT A ACCEPTED. (ACCEPTED VALUES: p(put_Store), n(number), s(search), q(quit))"
                         << endl;

                }
                    break;
            }
        }
        //wait for all to end 
        pthread_join(pidPutStore, NULL);
        pthread_join(pidSearch, NULL);
        pthread_join(pidNumber, NULL);
        pthread_join(pidReturnProcess, NULL);
        wait(&logValue);
    }
}
//
//gets a message from the search queue to be processed. 
//
Message getSearchMessage() {
    Message m;
    pthread_mutex_lock(&searchLock);
    m = searchQueue.front();
    searchQueue.pop();
    pthread_mutex_unlock(&searchLock);
    return m;
}
//
//gets a message from the number queue to be processed. 
//
Message getNumberMessage() {
    Message m;
    pthread_mutex_lock(&numberLock);
    m = numberQueue.front();
    numberQueue.pop();
    pthread_mutex_unlock(&numberLock);
    return m;
}
//
//gets a message from the putStore queue to be processed. 
//
Message getPutStoreMessage() {
    Message msg;
    pthread_mutex_lock(&putStoreLock);
    msg = putStoreQueue.front();
    putStoreQueue.pop();
    pthread_mutex_unlock(&putStoreLock);
    return msg;
}
//
//pushes message onto the return queue
//
void pushReturn(const Message &msg) {
    pthread_mutex_lock(&returnLock);
    returnMessageQueue.push(msg);
    pthread_mutex_unlock(&returnLock);
}
//
//pushes message onto the number queue
//
void pushNumber(const Message &msg) {
    pthread_mutex_lock(&numberLock);
    numberQueue.push(msg);
    pthread_mutex_unlock(&numberLock);
}
//
//pushes message onto the search queue
//
void pushSearch(const Message &msg) {
    pthread_mutex_lock(&searchLock);
    searchQueue.push(msg);
    pthread_mutex_unlock(&searchLock);
}
//
//gets a message from the return queue.
//
Message getReturnMessage() {
    Message msg;
    pthread_mutex_lock(&returnLock);
    msg = returnMessageQueue.front();
    returnMessageQueue.pop();
    pthread_mutex_unlock(&returnLock);
    return msg;
}
//
//pushes message onto the putStore queue
//
void pushPutStore(const Message &msg) {
    pthread_mutex_lock(&putStoreLock);
    putStoreQueue.push(msg);
    pthread_mutex_unlock(&putStoreLock);
}

//
//initalizes all pipes
//
void initalizePipes() {
    if (pipe(pipeLogger) == -1) {
        cout << "logger pipe failed to initalize. Please try again!" << endl;
        exit(0);
    }
}

//
//creates a file
//
bool CreateFile(fstream &f, const char *filename, Message msg) {
    if (ifstream(filename)) {
        return false;
    }
    ofstream file(filename);
    if (!file) {
        return false;
    }
    f.open(filename, fstream::out);
    f << msg.payload << endl;
    f.close();
    return true;
}

//
//loads a message struct to be sent out
//
Message loadMessage(int counter, vector<Message> &msg) {
    Message m = msg.at(counter);
    return m;
}

//
//opens the log file for logging
//
void LogStart(Log &l, SetupData &s) {
    int success = l.open();
    if (success != 0) {
        cout << "Could not open set up file! Please check data and try again." << endl;
        exit(0);
    } else {
        string stringData =
                "\nLog File: " + s.getLogfilename()
                + "\nCommand File : " + s.getCommandfilename()
                + "\nUsername: " + s.getUsername();
        l.writeLogRecord(stringData);
    }
}

//
//log command file data
//
void LogCommand(Message m, Log &l) {
    string stringData =
            string("\nCommand: ") + m.command
            + "\nKey: " + m.key
            + "\nPayload: " + m.payload;
    l.writeLogRecord(stringData);
}

//
//closes the log file
//
void LogEnd(Log &l) {
    int success = l.close();
    if (success != 0) {
        cout << "Could not close set up file! Please check data and try again." << endl;
        exit(0);
    }
}

//
//Refactor of data.read(), data.close();
//
void ProcessSetupFile(SetupData &d, Log &log) {
    string stringData;
    //attempt to open file
    if (d.open() == -1) {
        log.open();
        stringData = "START\nbad setupfile name\nEND\n";         //open log and record data
        log.writeLogRecord(stringData);
        log.close();
        cout << d.error(-1) << endl;
        exit(0);
    }
        //if open succeeded!
    else {
        d.read();                                                                           //read the file and store values
        d.close();                                                                          //close the file
    }
}

//
//Refactor of OpenCommandFile,ReadCommandFile, and CloseCommandFile
//
void ProcessCommandFile(fstream &f, const char *filename, vector<Message> &msg) {
    OpenCommandFile(f, filename);
    ReadCommandFile(f, msg);
    CloseCommandFile(f);
}

//
//opens a command file for reading
//
void OpenCommandFile(fstream &f, const char *filename) {
    f.open(filename, fstream::in);
    if (!f) {
        cout << "\nbad command file\n";
        exit(0);
    }
}

//
//Reads all commands from the command file
//
void ReadCommandFile(fstream &f, vector<Message> &msg) {
    string strs[3];
    int count = 0;
    int idcount = 0;
    Message m;
    string str;
    if (!f) {
        cout << "\nError! Please try again!\n" << endl;
        exit(0);
    }
    while (true) {
        if (f.eof())
            break;
        while (count < 3) {
            getline(f, str);
            strs[count] = str;
            count++;
        }
        m.id = ++idcount;
        m.command = strs[0][0];
        strcpy(m.key, strs[1].c_str());
        strcpy(m.payload, strs[2].c_str());
        msg.push_back(m);
        count = 0;
    }

}

//
//Close the open command file
//
void CloseCommandFile(fstream &f) {
    if (f)
        f.close();
}

//
//Print the data in the message struct
//
void PrintMessage(Message m) {
    cout << "id: " << m.id << endl;
    cout << "command: " << m.command << endl;
    cout << "key: " << m.key << endl;
    cout << "payload: " << m.payload << endl;
}
