#include <cstdlib>
#include <cstring>
#include <curl/curl.h>
#include <iostream>
#include <vector>
#include <regex>
#include <tinyxml2.h>

using namespace std;
using namespace tinyxml2;

struct MemoryStruct {
  char *memory;
  size_t size;
};

struct ChannelStruct{
	string name;
	string url;
	string srvref;
	string epgtitle;
};


struct EnigmaServices{
	string name;
	string bouquetref;
	
};

class Enigma2{
public:
	string enigma2ip;
	vector<EnigmaServices> parseBouquet(char * data);
	vector<ChannelStruct> m3uParser(char * url);
	void curlDownload(char * url ,MemoryStruct * chunk);
	
	bool getServices();
	
	vector<EnigmaServices> e2services;
	
	
};

