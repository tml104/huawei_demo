#ifndef ZSRACISNOTIFY_H 
#define ZSRACISNOTIFY_H 
//#include<windows.h>
#include<tchar.h>
#include <iostream>
#include <fstream>
#include <sstream>
#include <stdio.h>
#include <string>
using namespace std;

template<class T>
string convertToString(const T val)
{
	string s;
	std::stringstream ss;
	ss << val;
	ss >> s;
	return s;
}

FILE *ZSRCreateFile(const string &FilePath);
void ZSRWrite(FILE *_File,const string &strMessage);
void ZSRWriteDouble(FILE *_File,const string &head, double d);
void ZSRWriteDoubles(FILE *_File,const string &head, const double *d,const int &n);
void ZSRWriteInt(FILE *_File,const string &head, const int &i);
void ZSRWriteString(FILE *_File,const string &head, const string &istr);
FILE *ZSRCreateFile(const string &FilePath,const string &FileName);
//void ZSRReadInfoInJsonfile();

#endif