#include <iostream>
#include <cstring>
#include <time.h>

//Parsers
#include "Parsers/fastkv/fastkv.h"
#include "Parsers/QuickKV/KVParser/quickkv.h"


char* ReadFile(const char* path, int& len)
{

	FILE* f;

        /* Use the "safe" crt functions for windows and normal ones for POSIX systems */
#ifdef _WIN32 
        fopen_s(&f, path, "rb");
#else
        f = fopen(path, "rb");
        if(!f) abort();
#endif 

        fseek(f, 0, SEEK_END);
	len = ftell(f);
	rewind(f);
	char* str = (char*)malloc(len + 1);
	str[len] = 0;
	fread(str, len, 1, f);
	fclose(f);

	return str;
}

class CTimer
{
public:
	CTimer() {}
	CTimer(const char* name) { m_name = name; }

	void Reset()
	{
		m_parseTotalTime = 0;
		m_parseStartTime = 0;
		m_parseCount = 0;


		m_deleteTotalTime = 0;
		m_deleteStartTime = 0;
		m_deleteCount = 0;
	}



	inline void StartParseTimer()
	{
		m_parseStartTime = clock();
	};

	inline void StopParseTimer()
	{
		clock_t stop = clock();
		m_parseTotalTime += stop - m_parseStartTime;
		m_parseCount++;
	};

	// returns in ms
	float GetAverageParseTime()
	{
		return ( (float)m_parseStartTime ) / m_parseCount * CLOCKS_PER_SEC / 1000.0f;
	}



	inline void StartDeleteTimer()
	{
		m_deleteStartTime = clock();
	};

	inline void StopDeleteTimer()
	{
		clock_t stop = clock();
		m_deleteTotalTime += stop - m_deleteStartTime;
		m_deleteCount++;
	};

	// returns in ms
	float GetAverageDeleteTime()
	{
		return ((float)m_deleteStartTime) / m_deleteCount * CLOCKS_PER_SEC / 1000.0f;
	}


	void Print()
	{
		std::cout << "===" << m_name << "===\n";
		std::cout << GetAverageParseTime() << " ms to parse\n\n";
		std::cout << GetAverageDeleteTime() << " ms to delete\n";
		std::cout << "===" << m_name << "===\n";
	}

	const char* m_name = 0;
private:


	clock_t m_parseTotalTime = 0;
	clock_t m_parseStartTime = 0;
	clock_t m_parseCount = 0;


	clock_t m_deleteTotalTime = 0;
	clock_t m_deleteStartTime = 0;
	clock_t m_deleteCount = 0;


};

int main(int argc, char* argv[])
{
	if (argc != 2)
	{
		std::cout << "Failed!\nExpected keyvalue file path as first argument!";
		return 1;
	}

	char* path = argv[1];

	int len = 0;
	char* buf = ReadFile(path, len);
	std::cout.flush();


	CTimer fastkv("fastkv");
	CTimer quickkv("QuickKV");

	
	int count = 100;
	for (int i = 0; i < count; i++)
	{

		std::cout << "\n" << i << "/" << count << "\n";

		//QuickKV
		{
			std::cout << i << "/" << count << " - " << quickkv.m_name << "\n";

			//time parse
			quickkv.StartParseTimer();
			CKeyValueRoot* kv = CKeyValueRoot::Parse(buf, len);
			quickkv.StopParseTimer();

			//time delete
			quickkv.StartDeleteTimer();
			delete kv; 
			quickkv.StopDeleteTimer();
		}


		//fastkv
		{
			std::cout << i << "/" << count << " - " << fastkv.m_name << "\n";


			//fastkv modifies its file buffer so we have to make a copy
			char* fastkvBuf = new char[len];
			memcpy(fastkvBuf, buf, len);


			uint64_t index = 0;

			vars_t defines;
			defines.length = 3;
			defines.vars = (char**)new const char* [3]{ "X64", "X86_64", "LINUX" };

			//time parse
			fastkv.StartParseTimer();
			item_t parsed = kv_parse(fastkvBuf, &index, len, defines);
			fastkv.StopParseTimer();

			//time delete
			fastkv.StartDeleteTimer();
			kv_freeitem(parsed);
			fastkv.StopDeleteTimer();



			delete[] fastkvBuf;
		}


	}

	delete[] buf;

	char* shortPath = strrchr(path, '/');
	if(shortPath == nullptr)
		shortPath = strrchr(path, '\\');
	if (shortPath == nullptr)
		shortPath = path;
	else
		shortPath++;

	std::cout << "\n\n==times taken on " << shortPath  << " for " << count << " parses==\n\n\n";

	fastkv.Print();
	std::cout << "\n";
	quickkv.Print();

	std::cout << "\n\n==times taken on " << shortPath << " for " << count << " parses==\n\n\n";



	return 0;
}
