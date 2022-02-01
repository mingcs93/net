#include<iostream>
#include "timestamp.h"
#include "async_log.h"

using namespace base;

int main(int argc, char** argv)
{
	Timestamp now = Timestamp::now();
	std::string timeStr = now.toFormattedString();
	//LOGF("test....");
	return 0;
}