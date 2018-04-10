/*                                          */
/* Author: Hayden Burnette                  */
/* Date  : 2/04/2018                        */
/*                                          */
/*                                          */

// Uses C++ string class for convenience
#include <string>
// For the log file output
#include <fstream>
#include <time.h>
#ifndef LOG_H
#include "Log.h"
#endif

const string Log::DEFAULT_LOG_FILE_NAME = "log.txt";
//------------
//constructors
//------------
//
//default constructor
//
Log::Log()
{
  logfilename = Log::DEFAULT_LOG_FILE_NAME;
}
//
//char* constructor
//
Log::Log(char* lname)
{
  string str(lname);
  Log::setLogfileName(lname);
}
//
//string contructor
//
Log::Log(string lname)
{
  logfilename = lname;
}
//------------
//setters
//------------
//
//set the log file name
//
 void Log::setLogfileName(string cname)
 {
   if(cname == "")
   {
     logfilename = Log::DEFAULT_LOG_FILE_NAME;
   }
   else
   {
     logfilename = cname; 
   }
 }
//------------
//getters
//------------
//
//get the log file name
//
string Log::getLogfileName()
{
  return logfilename;
}
//
//returns the default log file name
//
string Log::getDefaultLogfileName( )
{
  return Log::DEFAULT_LOG_FILE_NAME;
}
//------------
// actual functions that do cool things
//------------
//
//open the file
//
int Log::open()
{
  int success = -1;
  if(logF)
  {
     const char *path = logfilename.c_str();
     logF.open( path, fstream::out | ios_base::app);
     success = 0;
     logF << "BEGIN";
  }
  return success;
}
//
//close the file
//
int Log::close()
{
  int success = -1;
  if(logF)
  {
     logF << "END " << Log::getTimeStamp() << endl;
     logF.close();
     success = 0;
  }
  return success;
}
//
//write the log file
//
int Log::writeLogRecord(string s)
{
  int success = -1;
  if(logF)
  {
     logF << s << endl;
     success = 0;
  }
  return success;
}
//
//returns a string that is the time of the system
//
string Log::getTimeStamp()
{
  time_t t;
  time(&t);
  string currtime = ctime(&t);
  return currtime;
}
