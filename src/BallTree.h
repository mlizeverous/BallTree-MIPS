#ifndef __BALL_TREE_H
#define __BALL_TREE_H

#include "Utility.h"

#include <ctime>
#include <math.h>
#include <stack>
#include <list>
#include <string>
#include <iostream>
#include <fstream>

using namespace std;

#define N0 20

class BallTree {
private:
	int dimension; // dimension作为一个私有数据，即全局变量
	ballTreeNode *root;
	int indexSlotNum; // 每一页里面槽的数量
	int dataSlotNum;
	int indexSlotSize; // 每一槽的大小
	int dataSlotSize;
	ifstream indexPage; // one page 数据
	ifstream dataPage;
	int curIndexPageId;
	int curDataPageId;
	int index;
	int indexData;
	const char* PATH;

	void getCenter(float *&center, float **dataSet, int numberOfData);

	void getRadius(float &radius, float **dataSet, int numberOfData, float *center);

	float getDistance(float *x, float *y);

	bool makeBallTreeSplit(int numberOfData, float **dataSet, float *&A, float *&B);

	bool makeBallTree(ballTreeNode*& node, int numberOfData, float **dataSet);

	float* getFarthestPoint(float *x, float **dataSet, int n);

	bool storeIndexToFile(const char* index_path);

	void getLeaveNodePreOrder(list<ballTreeNode*> &leave, ballTreeNode* root);
	void pharseDataFilePath(int &curSlot, int &fileIndex,
		ofstream &fout, string filePath, const char* index_path);
	bool storeDataToFile(const char* index_path);

	void getNode(int index, ballTreeNode*& node);

	void getData(int indexData, int &TableSize, float** &data);

	void readOneIndexPage(int pageId);

	void readOneDataPage(int pageId);

	float length_of(float* vector);

	float MIP(float* query, ballTreeNode*& T);

	float innerProduct(float* vector1, float* vector2);

	void treeSearch(float* query, ballTreeNode* T, float &highestInnerProduct, float* &mipQuery);

	void linearSearch(float* query, ballTreeNode*&T, float &highestInnerProduct, float* &mipQuery);

public:
	BallTree();
	~BallTree();
	//清除所有在内存中的节点
	void clear();
	void recursive_clear(ballTreeNode *&node);

	// 任务1：建树
	bool buildTree(
		int n,
		int d,
		float** data);

	// 任务2：写进外存
	bool storeTree(
		const char* index_path); // index_path: index文件夹路径

								 // 任务3：从外存读进来
	bool restoreTree(
		const char* index_path); // 同上

								 // 任务4：用最大内积搜索
	int mipSearch(
		int d,
		float* query);

};

#endif