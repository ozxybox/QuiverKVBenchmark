#include <iostream>
#include <cstring>
#include <time.h>

#ifndef _WIN32
#include <climits>
#endif

//Parsers

/* This is C source code */
extern "C" {
#include "Parsers/fastkv/fastkv.h"
} 
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
		m_parseMaxTime = LONG_MIN;
		m_parseMinTime = LONG_MAX;


		m_deleteTotalTime = 0;
		m_deleteStartTime = 0;
		m_deleteCount = 0;
		m_deleteMaxTime = LONG_MIN;
		m_deleteMinTime = LONG_MAX;
	}



	inline void StartParseTimer()
	{
		m_parseStartTime = clock();
	};

	inline clock_t StopParseTimer()
	{
		clock_t stop = clock();
		clock_t timeTaken = stop - m_parseStartTime;
		m_parseTotalTime += timeTaken;
		m_parseCount++;

		if (timeTaken > m_parseMaxTime)
			m_parseMaxTime = timeTaken;
		else if (timeTaken < m_parseMinTime)
			m_parseMinTime = timeTaken;

		return timeTaken;
	};

	// returns in ms
	float GetAverageParseTime()
	{
		return ( (float)m_parseTotalTime) / (float)m_parseCount * CLOCKS_PER_SEC / 1000.0f;
	}

	// returns in ms
	float GetMaxParseTime()
	{
		return ((float)m_parseMaxTime) * CLOCKS_PER_SEC / 1000.0f;
	}

	// returns in ms
	float GetMinParseTime()
	{
		return ((float)m_parseMinTime) * CLOCKS_PER_SEC / 1000.0f;
	}



	inline void StartDeleteTimer()
	{
		m_deleteStartTime = clock();
	};

	inline clock_t StopDeleteTimer()
	{
		clock_t stop = clock();
		clock_t timeTaken = stop - m_deleteStartTime;
		m_deleteTotalTime += timeTaken;
		m_deleteCount++;

		if (timeTaken > m_deleteMaxTime)
			m_deleteMaxTime = timeTaken;
		else if (timeTaken < m_deleteMinTime)
				m_deleteMinTime = timeTaken;

		return timeTaken;
	};

	// returns in ms
	float GetAverageDeleteTime()
	{
		return ((float)m_deleteTotalTime) / m_deleteCount * CLOCKS_PER_SEC / 1000.0f;
	}

	// returns in ms
	float GetMaxDeleteTime()
	{
		return ((float)m_deleteMaxTime) * CLOCKS_PER_SEC / 1000.0f;
	}

	// returns in ms
	float GetMinDeleteTime()
	{
		return ((float)m_deleteMinTime) * CLOCKS_PER_SEC / 1000.0f;
	}




	void Print()
	{
		std::cout << "===" << m_name << "===\n";
		std::cout << "Parsing:\n";
		std::cout << "\tBest Time: " << GetMinParseTime() << " ms\n";
		std::cout << "\tWorst Time: " << GetMaxParseTime() << " ms\n";
		std::cout << "\tAverage Time: " << GetAverageParseTime() << " ms\n";
		std::cout << "\n";
		std::cout << "Deleting:\n";
		std::cout << "\tBest Time: " << GetMinDeleteTime() << " ms\n";
		std::cout << "\tWorst Time: " << GetMaxDeleteTime() << " ms\n";
		std::cout << "\tAverage Time: " << GetAverageDeleteTime() << " ms\n";
		std::cout << "===" << m_name << "===\n";
	}

	const char* m_name = 0;
private:


	clock_t m_parseTotalTime = 0;
	clock_t m_parseStartTime = 0;
	clock_t m_parseCount = 0;
	clock_t m_parseMaxTime = LONG_MIN;
	clock_t m_parseMinTime = LONG_MAX;


	clock_t m_deleteTotalTime = 0;
	clock_t m_deleteStartTime = 0;
	clock_t m_deleteCount = 0;
	clock_t m_deleteMaxTime = LONG_MIN;
	clock_t m_deleteMinTime = LONG_MAX;


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
