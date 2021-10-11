#include "Enigma2.h"


static size_t
WriteMemoryCallback(void *contents, size_t size, size_t nmemb, void *userp)
{
  size_t realsize = size * nmemb;
  struct MemoryStruct *mem = (struct MemoryStruct *)userp;
 
  char *ptr = (char *)realloc(mem->memory, mem->size + realsize + 1);
  if(!ptr) {
    /* out of memory! */
    printf("not enough memory (realloc returned NULL)\n");
    return 0;
  }
 
  mem->memory = ptr;
  memcpy(&(mem->memory[mem->size]), contents, realsize);
  mem->size += realsize;
  mem->memory[mem->size] = 0;
 
  return realsize;
}

string urlencode(string s)
{
    const char *str = s.c_str();
    vector<char> v(s.size());
    v.clear();
    for (size_t i = 0, l = s.size(); i < l; i++)
    {
        char c = str[i];
        
        if (c == ' ')
        {
            v.push_back('%');
			v.push_back('2');
			v.push_back('0');
        }
        else
        {
			v.push_back(c);
        }
    }

    return string(v.cbegin(), v.cend());
}


void Enigma2::curlDownload(char * url ,MemoryStruct * chunk){
	
	CURL *curl_handle;
	CURLcode res;
	chunk->memory = (char *)malloc(1);  
	chunk->size = 0; 
	curl_global_init(CURL_GLOBAL_ALL);
	curl_handle = curl_easy_init();
	curl_easy_setopt(curl_handle, CURLOPT_URL, url);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEFUNCTION, WriteMemoryCallback);
	curl_easy_setopt(curl_handle, CURLOPT_WRITEDATA, (void *)chunk);
	curl_easy_setopt(curl_handle, CURLOPT_USERAGENT, "libcurl-agent/1.0");
	res = curl_easy_perform(curl_handle);
	if(res != CURLE_OK) {
		fprintf(stderr, "curl_easy_perform() failed: %s\n",
        curl_easy_strerror(res));
	}
	else {
		
		
	}
	curl_easy_cleanup(curl_handle);
	curl_global_cleanup();
}


vector<EnigmaServices> Enigma2::parseBouquet(char * data){
	
	vector<EnigmaServices> tmpret;
	XMLDocument doc;
	doc.Parse( data );
	XMLElement * pRootElement = doc.RootElement();
	if (NULL != pRootElement) {
			XMLElement * pe2service = pRootElement->FirstChildElement("e2service");
			while(pe2service){
				EnigmaServices tmpele;
				
				XMLElement * pe2serviceName = pe2service->FirstChildElement("e2servicename");
				XMLElement * pe2serviceRef = pe2service->FirstChildElement("e2servicereference");
				tmpele.name = pe2serviceName->GetText();
				tmpele.bouquetref = pe2serviceRef->GetText();
				tmpret.push_back(tmpele);
				pe2service = pe2service->NextSiblingElement("e2service");
			}
	}else{
		
	}
	return tmpret;
}

vector<ChannelStruct> Enigma2::m3uParser(char * url){
	string m3uurl = "http://";
	m3uurl.append(enigma2ip);
	m3uurl.append("/web/services.m3u?bRef=");
	m3uurl.append(urlencode(url));
	struct MemoryStruct chunk;
	curlDownload((char *)m3uurl.c_str(),&chunk);
	
	vector<ChannelStruct> tmpret;
	string s = chunk.memory;
	std::regex rgx("#EXTINF:(?:)[[:print:]]*,(.+)\n[[:print:]]*\n(http[[:print:]]*)");
    std::smatch matches;
    std::smatch sm;
	while (regex_search(s, sm, rgx))
	{
		ChannelStruct tmpchannel;
		tmpchannel.name = sm[1];
		tmpchannel.url = "enigma2://";
		tmpchannel.url.append(sm[2]); 
		tmpret.push_back(tmpchannel);
		s = sm.suffix();
	}
	free(chunk.memory);
	return tmpret;
}

bool Enigma2::getServices(){
	struct MemoryStruct chunk;
	string downurl = "http://";
	downurl.append(enigma2ip);
	downurl.append("/web/getservices?sRef=");
	curlDownload((char *)downurl.c_str(),&chunk);

	e2services =  parseBouquet(chunk.memory);
	free(chunk.memory);
}
 