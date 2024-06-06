#pragma once

#include "ZSRACISHeads.h"

//#include "json/json.h"
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string>
#include <json/json.h>
#include <cassert>
#include <errno.h>

using namespace std;

class ZSRReadInfoInJsonfile
{
public:
	ZSRReadInfoInJsonfile(void);
	~ZSRReadInfoInJsonfile(void);

//function
public:
	int ReadJsonFromFile(const char* filename);

private:


//data
public:

private:


};

