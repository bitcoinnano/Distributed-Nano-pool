#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <err.h>
#include <errno.h>
#include <unistd.h>

#include <iostream>
#include <string>

#include <boost/interprocess/sync/file_lock.hpp>
#include <glog/logging.h>
#include <libconfig.h++>
#include <event2/thread.h>
#include "primitives/block.h"
#include "zmq.hpp"

#include "Utils.h"
#include "StratumServer.h"

using namespace std;

CBlockHeader header_;
int handle(char*);

int main(){
	cout << "input header data" << endl;
	header_.hashReserved = uint256S("00000000000000000000000000000000");	
	char pSol[2689];
	FILE* pFile;
	pFile = fopen("solution.txt","r+");
	if (pFile == NULL) cout << "Error opening file" << endl;
	else{
    	if(fgets(pSol, 2689, pFile) == NULL )
    	fclose(pFile);
	}
	vector<unsigned char> nSolution(ParseHex(pSol));
	while(true){
		int i =	handle(pSol);
		if(i){break;}
	}
	return 0;
}
int handle(char* pSol){
    string str = "\n";
    getline(cin,str);
	if(str == "hash"){
	    uint256 blkHash = header_.GetHash();
	    cout << blkHash.ToString() << endl;
		return 0;
	}
	if(str == "break"){
		return 1;	
	}
	if(str == "print"){
		cout << "prevhash: " << header_.hashPrevBlock.ToString() << endl;
		cout << "mekleRoo: " << header_.hashMerkleRoot.ToString() << endl;
		cout << "Reseved:  " << header_.hashReserved.ToString() << endl; 
		cout << "nNonce:   " << header_.nNonce.ToString() << endl;
		cout << "nTime:    " << to_string(header_.nTime) << endl;
		cout << "nBits:    " << to_string(header_.nBits) << endl;
		cout << "nVersion: " << to_string(header_.nVersion) << endl; 	 
     	cout << "nSolution:" << pSol << endl;
		return 0;
	}
    size_t found = str.find("=");
    if(found == string::npos){
		return 0;
   	}
	string preStr = str.substr(0,found);
 //   cout << preStr << endl;
	string postStr = str.substr(found+1,str.size());
 //	cout << postStr << endl;
	if(preStr == "hashPrevBlock"){ header_.hashPrevBlock = uint256S(postStr);}
    else if(preStr == "hashMerkleRoot"){ header_.hashMerkleRoot = uint256S(postStr); }
    else if(preStr == "nNonce"){ header_.nNonce = uint256S(postStr);}
    else if(preStr == "nTime"){	header_.nTime = strtoul(postStr.c_str(),nullptr,16); }
	else if(preStr == "nBits"){ header_.nBits = strtoul(postStr.c_str(),nullptr,16); }
	else if(preStr == "nVersion"){ header_.nVersion = strtoul(postStr.c_str(),nullptr,16);}
	else if(preStr == "nSolution"){ header_.nSolution = ParseHex(postStr.c_str());}
	else {}
	return 0;
}









