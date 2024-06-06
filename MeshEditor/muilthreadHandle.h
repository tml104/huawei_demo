#pragma once
#include "pthread.h"
#include "Cell.h"
#include "ohmConnection.h"
#include "GeometricFill.h"
#include "vector"
struct ConnectFillArgs {
	std::vector<Ohm_slice::Cell *> cell_list;
	int size;
	ConnectFillArgs(std::vector<Ohm_slice::Cell *> cell_list_,int size_) :
		cell_list(cell_list_),size(size_) {}
};

void *connectAndFillTask(void *val);
void muilHandleConnectAndFill(int thread_num, std::vector<Ohm_slice::Cell *> cell_list);
