
#include <iostream>
#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <string>

using namespace std;
int main(int argc, char** argv){
	string str="\n";
    getline(std::cin,str);                                                                                                                                                                                                                   
    string::iterator first = str.begin();
    string::iterator last = str.end();
    while((first != last)&&(first != (last-2))) {
      std::iter_swap(first+1,--last);                                                                                                                                                                                                    
      std::iter_swap(first++,--last);
      ++first;
    } 
	if(argv[1]!=NULL){
 	  int len = strlen(argv[1]);
 	  char *p=(char*)malloc(len+1);
	  memset(p,0,len);
	  strcpy(p,argv[1]);
 	  p[len]='\0';
	  if(p[0]=='4'){
	   first = str.begin();
	   last = str.end();
	   while((first != last)&&(first != (last-8))) {
   	     std::iter_swap(first,last-8);
   		 std::iter_swap(first+1,last-7);
   		 std::iter_swap(first+2,last-6);
    	 std::iter_swap(first+3,last-5);
	     std::iter_swap(first+4,last-4);
    	 std::iter_swap(first+5,last-3);
	     std::iter_swap(first+6,last-2);
    	 std::iter_swap(first+7,last-1);
	     first += 8;
    	 last -= 8;
       }	
      }
	free(p);
	}
	std::cout << str << endl;	
	return 0;
}
