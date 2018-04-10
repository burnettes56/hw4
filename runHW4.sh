#!/bin/bash
if(ls makefile >> /dev/null 2>&1)
then
  {
    make clean >> /dev/null 2>&1
    make >> /dev/null 2>&1
    printf "\nA clean build has been made!\n"    
  }
else
   printf "\nThe makefile does not exist. Please check for makefile inside current directory.\n" 
fi
if(ls hw4 >> /dev/null 2>&1)
then
  {
     printf "\nExecuting Hw4\n"
    ./hw4 $@
  }
else
  printf "\nExecuteable \"hw4\" was not found\n"   
fi