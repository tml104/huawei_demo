#include "stdafx.h"
#include "test.h"
#include "muilthreadHandle.h"


void *connectAndFillTask(void *val) {
	ConnectFillArgs* args = (ConnectFillArgs*)val;
	vector<Ohm_slice::Cell *> &cell_list = args->cell_list;
	printf("vector size: %d\n", cell_list.size());
	for (int index = 0; index < args->size; index++) {
		printf("order %d\n", args->cell_list[0]->patch_list[0]->pixel_list[0][0]->order);
		setCellAttr(args->cell_list[index]);
		handlePartialConnect(args->cell_list[index]);
		geometryFillWithSide(args->cell_list[index]);
		printf("index:%d\n", index);
	}
	return NULL;
}
void muilHandleConnectAndFill(int thread_num, std::vector<Ohm_slice::Cell *> cell_list) {
	std::vector<pthread_t> thread_list(thread_num);
	int task_num = cell_list.size();
	int thread_handle_num = task_num / thread_num;
	if (task_num % thread_num != 0) {
		thread_handle_num++;
	}
	printf("order %d\n", cell_list[0]->patch_list[0]->pixel_list[0][0]->order);
	int index = 0;
	printf("muilthread\n");
	while (index * thread_handle_num < cell_list.size()) {
		printf("%d %d\n", index * thread_handle_num, (index + 1) * thread_handle_num);
		printf("task: %d [%ld->%ld]\n", index, index * thread_handle_num, std::min((index + 1) * thread_handle_num, (int)cell_list.size()));
		std::vector<Ohm_slice::Cell *>::const_iterator begin = cell_list.begin() + index * thread_handle_num;
		std::vector<Ohm_slice::Cell *>::const_iterator end = cell_list.begin() + std::min((index + 1) * thread_handle_num, (int)cell_list.size());
		std::vector<Ohm_slice::Cell *> cell_part(begin, end);
		printf("cell_part size %d\n", cell_part.size());
		ConnectFillArgs *args = new ConnectFillArgs(cell_part,cell_part.size());
		printf("cell_part size %d\n", args->cell_list.size());
		pthread_create(&thread_list[index], NULL, (void* (*)(void*))connectAndFillTask, (void*)args);
		index++;
	}
	for (int i = 0; i < thread_list.size(); i++) {
		pthread_join(thread_list[i], NULL);
	}
	printf("end handle\n");
}
