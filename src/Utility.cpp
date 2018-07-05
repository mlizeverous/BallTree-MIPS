#include <cstdio>
#include <cmath>
#include <cstdlib>
#include <ctime>
#include <algorithm>
#include <cstring>
#include <stdio.h>

#include "Utility.h"

using namespace std;

void writeInt(ofstream &out, int num);
void writeFloatArr(ofstream &out, float * arr, int len);
void writeFloat(ofstream &out, float data);

ballTreeNode::ballTreeNode() {
	index = indexRight = indexData = -1;
	radius = 0;
	tableSize = 0;
	mean = NULL;
	data = NULL;
	left = right = NULL;
	dimension = 0;
}

ballTreeNode::ballTreeNode(float r, float *m, int d) {
	index = indexRight = indexData = -1;
	radius = r;
	tableSize = 0;
	mean = new float[d];
	memcpy(mean, m, (d) * sizeof(float));
	dimension = d;
	data = NULL;
	left = right = NULL;
}

ballTreeNode::~ballTreeNode() {
	if (mean != NULL) {
		delete[] mean;
	}
	if (data != NULL) {
		for (int i = 0; i < tableSize; i++) {
			delete[] data[i];
		}
		delete[] data;
	}
}

void ballTreeNode::storeIndexTraversally(ofstream &out, const char* dir_path, int indexSlotNum) {
	if (getSlotId(indexSlotNum) == 0 && getPageId(indexSlotNum) != 0) {
		out.close();
		out.open(dir_path + to_string(getPageId(indexSlotNum)) + ".txt", std::ios::binary | std::ios::out);
	}
	writeInt(out, indexRight);
	writeInt(out, indexData);
	writeFloatArr(out, mean, dimension);
	writeFloat(out, radius);
	if (left != NULL) {
		left->storeIndexTraversally(out, dir_path, indexSlotNum);
		right->storeIndexTraversally(out, dir_path, indexSlotNum);
	}
}

int ballTreeNode::getPageId(int slotNum) {
	return index / slotNum;
}

int ballTreeNode::getSlotId(int slotNum) {
	return index % slotNum;
}

void writeInt(ofstream &out, int num) {
	out.write((char*)&num, sizeof(num));
}

void writeFloatArr(ofstream &out, float * arr, int len) {
	out.write((char*)arr, len * sizeof(arr[0]));
}
void writeFloat(ofstream &out, float data) {
	out.write((char*)&data, sizeof(data));
}

bool read_data(
	int n,
	int d,
	float** &data,
	const char* file_name)
{
	FILE* fin = fopen(file_name, "r");
	if (!fin) {
		printf("%s doesn't exist!\n", file_name);
		return false;
	}

	data = new float*[n];
	for (int i = 0; i < n; i++) {
		data[i] = new float[d];
		for (int j = 0; j < d; j++) {
			fscanf(fin, "%f", &data[i][j]);
		}
	}

	printf("Finish reading %s\n", file_name);
	fclose(fin);

	return true;
}