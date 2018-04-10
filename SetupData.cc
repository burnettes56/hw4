
/*                                          */
/* Author: Hayden Burnette                  */
/* Date  : 2/04/2018                        */
/*                                          */
/*                                          */

#include <string>
#include <fstream>
#include <map>
#include<unistd.h>
#include<iostream>
//define
#ifndef SETUPDATA_H
#include "SetupData.h"
#endif
//
//constructors
//
//
//default constructor
//
SetupData::SetupData()
{
  SetupData::setPathname("");
  SetupData::setSetupfilename("");
}
SetupData::SetupData(string pname, string sname)
{
  SetupData::setPathname(pname);
  SetupData::setSetupfilename(sname);
}
//
//getters and setters
//
//
//set path name
//
void SetupData::setPathname(string pname)
{
  values["pname"] = pname; 
}
//
//get path name
//
string SetupData::getPathname()
{
   return values["pname"];
}
//
//set setupfile name
//
void SetupData::setSetupfilename(string sname)
{
  if(sname == "")
  {
     values["sname"] = "setup"; 
  }
  else
  {
     values["sname"] = sname;  
  }
}
//
//get setupfile name
//
string SetupData::getSetupfilename()
{
  return values["sname"];
}
//
//set logfile name
//
void SetupData::setLogfile(string lname)
{
  values["lname"] = lname;
}
//
//get logfile name
//
string SetupData::getLogfilename()
{
  return values["lname"];
}
//
//set commandfile name
//
void SetupData::setCommandfilename(string cname)
{
  values["cname"] = cname;
}
//
//get commandfile name
//
string SetupData::getCommandfilename()
{
  return values["cname"];
}
//
//set user name
//
void SetupData::setUsername(string uname)
{
  values["uname"] = uname;
}
//
//get user name
//
string SetupData::getUsername()
{
  return values["uname"];
}
//
//actual functions that do cool things
//
//
//opens a file!
//
int SetupData::open()
{
  int success = 0;
  string str1 = values["pname"];
  const char *path = str1.c_str();
  string str2 = values["sname"];
  const char *setup = str2.c_str();
  
  if(chdir(path) != -1)
  {    
    f.open(setup, fstream::in);
    if(!f)
    {
      success = -1;
    }
  }
  else
  {
    //path failed return bad path
    cout << "bad Path" << endl;
  }
  return success;
}
//
//reads a file and stores the values in a object
//
void SetupData::read( )
{
  string strs [3];
  int count = 0;
  string str;
  string str2;
  int strlength;
  while(true)
  {
    if(f.eof())
      break;
    getline(f, str);
    int posOfdata = str.find(' ') + 1;
    strlength = (sizeof str - 1);
    str2 = str.substr(posOfdata, (strlength - posOfdata));
    strs[count] = str2;
    count++;
  }
  //store values
  const char *conv0 = strs[0].c_str();
  SetupData::setLogfile(conv0);
  const char *conv1 = strs[1].c_str();
  SetupData::setCommandfilename(conv1);
  const char *conv2 = strs[2].c_str();
  SetupData::setUsername(conv2);
}
//
//prints the data stored in the object to the screen
//
void SetupData::print( )
{
  cout << "\nContents of setup file:\n" << endl;
  cout << "logfile: " << getLogfilename() << endl;
  cout << "commandfile: " << getCommandfilename() << endl;
  cout << "username: " << getUsername() << endl;
  cout << "\nEND OF FILE\n" << endl;
}
//
//closes a file!
//
void SetupData::close( )
{
  if(f)
    f.close();
}

// Error stringifier
//
//returns a string based on passed error message
//
string SetupData::error(int e)
{
  if(e == 0)
    return "success";
  else if(e == -1 || e == -2)
    return "Please check path and file name!";
}
