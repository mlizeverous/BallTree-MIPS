#include "Utility.h"
#include "BallTree.h"

bool BallTree::makeBallTreeSplit(int numberOfData, float **dataSet, float* &A, float* &B) {
	srand((unsigned)time(NULL));
	int randomNum = rand() % numberOfData;
	float *x = dataSet[randomNum];
	A = getFarthestPoint(x, dataSet, numberOfData);
	B = getFarthestPoint(A, dataSet, numberOfData);
	return true;
}

float* BallTree::getFarthestPoint(float *x, float **dataSet, int n) {
	float *farthestPoint = NULL;
	float farthest = 0;
	float distance = 0;
	for (int i = 0; i < n; i++) {
		distance = getDistance(x, dataSet[i]);
		if (farthest < distance) {
			farthest = distance;
			farthestPoint = dataSet[i];
		}
	}
	return farthestPoint;
}

void BallTree::getCenter(float *&center, float **dataSet, int numberOfData) {
	for (int i = 1; i < dimension; i++) {
		float mean = 0;
		for (int j = 0; j < numberOfData; j++) {
			mean += dataSet[j][i];
		}
		mean = mean / numberOfData;
		center[i] = mean;
	}
}

void BallTree::getRadius(float &radius, float **dataSet, int numberOfData, float *center) {
	float max = 0;
	for (int i = 0; i < numberOfData; i++) {
		float distance = getDistance(center, dataSet[i]);
		if (distance > max) {
			max = distance;
		}
	}
	radius = max;
}

float BallTree::getDistance(float *x, float *y) {
	float sum = 0;
	for (int i = 1; i < dimension; i++) {
		sum += (x[i] - y[i]) * (x[i] - y[i]);
	}
	return sqrt(sum);
}

bool BallTree::makeBallTree(ballTreeNode *&node, int numberOfData, float **dataSet) {
	index++;
	float *center = new float[dimension];
	float radius;
	getCenter(center, dataSet, numberOfData);
	getRadius(radius, dataSet, numberOfData, center);
	node = new ballTreeNode(radius, center, dimension);
	delete[]center;
	node->index = index;
	//叶子节点
	if (numberOfData <= N0) {
		node->indexData = indexData;
		node->data = new float*[numberOfData];
		for (int i = 0; i < numberOfData; i++) {
			node->data[i] = new float[dimension];
			for (int j = 0; j < dimension; j++) {
				node->data[i][j] = dataSet[i][j];
			}
		}
		node->tableSize = numberOfData;
		indexData = indexData + numberOfData + 1;
		for (int i = 0; i < numberOfData; i++) {
			if (dataSet[i] != NULL) {
				delete[] dataSet[i];
			}
		}
		if (dataSet != NULL) {
			delete[] dataSet;
		}
		return false;
	}
	//得到A、B节点
	float *A;
	float *B;
	makeBallTreeSplit(numberOfData, dataSet, A, B);
	//得到分开的数据集
	float **leftDataSet = new float*[numberOfData];
	float **rightDataSet = new float*[numberOfData];
	int leftDataSize = 0;
	int rightDataSize = 0;
	for (int i = 0; i < numberOfData; i++) {
		float ADistance = getDistance(dataSet[i], A);
		float BDistance = getDistance(dataSet[i], B);
		if (ADistance <= BDistance) {
			leftDataSet[leftDataSize] = dataSet[i];
			leftDataSize++;
		}
		else {
			rightDataSet[rightDataSize] = dataSet[i];
			rightDataSize++;
		}
	}
	//重新分配数据集
	float **leftData = new float*[leftDataSize];
	float **rightData = new float*[rightDataSize];
	for (int i = 0; i < leftDataSize; i++) {
		leftData[i] = new float[dimension];
		for (int j = 0; j < dimension; j++) {
			leftData[i][j] = leftDataSet[i][j];
		}
	}
	for (int i = 0; i < rightDataSize; i++) {
		rightData[i] = new float[dimension];
		for (int j = 0; j < dimension; j++) {
			rightData[i][j] = rightDataSet[i][j];
		}
	}
	for (int i = 0; i < numberOfData; i++) {
		if (dataSet[i] != NULL) {
			delete[] dataSet[i];
		}
	}
	if (dataSet != NULL) {
		delete[] dataSet;
	}
	delete[] leftDataSet;
	delete[] rightDataSet;
	//为左右节点赋值
	makeBallTree(node->left, leftDataSize, leftData);
	makeBallTree(node->right, rightDataSize, rightData);
	node->indexRight = node->right->index;
	return true;
}

BallTree::BallTree() {
	dimension = 0;
	root = NULL;
	index = -1;
	indexData = 0;
	curIndexPageId = -1;
	curDataPageId = -1;
}

BallTree::~BallTree() {
	if (root == NULL) {
		dataPage.close();
		indexPage.close();
		return;
	}
	clear();
	dataPage.close();
	indexPage.close();
}

//读进去之后删除整棵树
void BallTree::clear() {
	if (root == NULL) {
		return;
	}
	recursive_clear(root);
}

//递归删除节点
void BallTree::recursive_clear(ballTreeNode *&node) {
	if (node == NULL) {
		return;
	}
	if (node->left != NULL) {
		recursive_clear(node->left);
	}
	if (node->right != NULL) {
		recursive_clear(node->right);
	}
	delete node;
	node = NULL;
}
//建树
bool BallTree::buildTree(int numberOfData, int dimension, float** data) {
	this->dimension = dimension;
	indexSlotSize = (sizeof(int) + sizeof(int) + sizeof(float) * dimension + sizeof(float)); // Bytes
	indexSlotNum = PAGE_SIZE / indexSlotSize; // one page 64KB
	dataSlotSize = (sizeof(float) * dimension); // Bytes
	dataSlotNum = PAGE_SIZE / dataSlotSize; // one page 64KB
	makeBallTree(root, numberOfData, data);
	return true;
}

bool BallTree::storeTree(const char* index_path) {
	storeIndexToFile(index_path);
	storeDataToFile(index_path);
	printf("Finish storing tree in %s\n", index_path);
	return true;
}

bool BallTree::storeIndexToFile(const char* index_path) {
	string dir_path(index_path);
	dir_path += "/index/"; // index索引页所在文件夹路径
	ofstream out(dir_path + "0.txt", std::ios::binary | std::ios::out);
	root->storeIndexTraversally(out, dir_path.c_str(), indexSlotNum);
	string index_path_string(index_path);
	printf("Finish storing index in %s\n", (index_path_string + "/index").c_str());
	return true;
}

bool BallTree::storeDataToFile(const char* index_path) {
	int fileIndex = 0;
	string filePath = "";
	filePath = filePath + index_path + "/data/" + to_string(fileIndex) + ".txt";
	int curSlot = 0;
	list<ballTreeNode*> leave;
	getLeaveNodePreOrder(leave, root);
	ofstream fout(filePath.c_str(), std::ios::binary);

	for (ballTreeNode* curNode : leave) {
		if (curSlot == dataSlotNum)
			pharseDataFilePath(curSlot, fileIndex, fout, filePath, index_path);
		int size = curNode->tableSize;
		fout.write((char*)&(size), dataSlotSize);
		curSlot++;

		for (int i = 0; i < curNode->tableSize; i++) {
			if (curSlot == dataSlotNum)
				pharseDataFilePath(curSlot, fileIndex, fout, filePath, index_path);
			fout.write((char*)curNode->data[i], dataSlotSize);
			curSlot++;
		}
	}
	fout.close();
	leave.clear();
	cout << "Finish storing data to file." << endl;
	return true;
}

void BallTree::getLeaveNodePreOrder(list<ballTreeNode*> &leave, ballTreeNode* curNode) {
	if (curNode->indexData != -1) {
		leave.push_back(curNode);
		return;
	}
	if (curNode->left != NULL)
		getLeaveNodePreOrder(leave, curNode->left);
	if (curNode->right != NULL)
		getLeaveNodePreOrder(leave, curNode->right);
	return;
}

void BallTree::pharseDataFilePath(
	int &curSlot, int &fileIndex, ofstream &fout, string filePath, const char* index_path) {
	curSlot = 0;
	fileIndex++;
	fout.close();
	filePath.clear();
	filePath = filePath + index_path + "/data/" + to_string(fileIndex) + ".txt";
	fout.open(filePath.c_str(), std::ios::binary);
}


bool BallTree::restoreTree(const char* index_path) {
	PATH = index_path;
	readOneIndexPage(0);
	return true;
}

void BallTree::readOneIndexPage(int pageId) {
	if (curIndexPageId != pageId) {
		string path = "";
		indexPage.close();
		path = path + PATH + "/index/" + to_string(pageId) + ".txt";

		indexPage.open(path, ios::binary | ios::in);
		if (!indexPage) {
			cout << "open index falied" << endl;
			return;
		}
		curIndexPageId = pageId;
		return;
	}
	return;
}

void BallTree::readOneDataPage(int pageId) {
	if (curDataPageId != pageId) {
		dataPage.close();
		string path = "";
		path = path + PATH + "/data/" + to_string(pageId) + ".txt";

		dataPage.open(path, ios::binary | ios::in);
		if (!indexPage) {
			cout << "open data falied" << endl;
			return;
		}
		curDataPageId = pageId;
		return;
	}
	return;
}

void BallTree::getData(int indexData, int &TableSize, float** &data) {
	int pageId = indexData / dataSlotNum;
	readOneDataPage(pageId);
	dataPage.seekg((indexData % dataSlotNum) * dataSlotSize, ios::beg);
	int slotId = (indexData % dataSlotNum);
	dataPage.seekg(slotId * dataSlotSize, ios::beg);
	int size = 0;
	dataPage.read((char*)&size, sizeof(int));
	TableSize = size;
	dataPage.seekg((dataSlotSize - sizeof(int)), ios::cur);
	data = new float*[size];
	for (int i = 0; i < size; i++) {
		data[i] = new float[dimension];
	}
	//解决跨页读取问题
	for (int i = 0; i < size; i++) {
		if (i != size && (slotId + i + 1 == dataSlotNum)) {
			pageId = pageId + 1;
			readOneDataPage(pageId);
			i--;
			slotId = 0 - i - 1;
		}
		else {
			dataPage.read((char*)data[i], dataSlotSize);
		}
	}
	//return data;
}

void BallTree::getNode(int index, ballTreeNode*& node) {
	int pageid = index / indexSlotNum;
	int slotid = index % indexSlotNum;
	int indexRight, indexData;
	float radius;
	float *center = new float[dimension];
	readOneIndexPage(pageid);
	indexPage.seekg(slotid * indexSlotSize, ios::beg);
	indexPage.read((char*)&indexRight, sizeof(indexRight));
	indexPage.read((char*)&indexData, sizeof(indexData));
	//dataSlotSize在此处相当于一个向量的大小
	indexPage.read((char*)center, dataSlotSize);
	indexPage.read((char*)&radius, sizeof(radius));
	node = new ballTreeNode(radius, center, dimension);
	node->indexRight = indexRight;
	node->indexData = indexData;
	node->index = index;
	delete[] center;
}

float BallTree::length_of(float* query) {
	float* zero = new float[dimension];
	for (int i = 1; i < dimension; i++) {
		zero[i] = 0;
	}
	float ans = getDistance(query, zero);
	delete[] zero;
	return ans;
}

float BallTree::innerProduct(float* vector1, float* vector2) {
	float IP = 0;
	for (int i = 1; i < dimension; i++) {
		IP += vector1[i] * vector2[i];
	}
	return IP;
}


float BallTree::MIP(float* query, ballTreeNode*& T) {
	return innerProduct(query, T->mean) + length_of(query)*T->radius;
}

void BallTree::linearSearch(float* query, ballTreeNode*& T, float &highestInnerProduct, float* &mipQuery) {
	float** data;
	getData(T->indexData, T->tableSize, data);
	for (int i = 0; i < T->tableSize; i++) {
		float* p = data[i];
		float IP = innerProduct(query, p);
		if (IP > highestInnerProduct) {
			for (int i = 0; i < dimension; i++) {
				mipQuery[i] = p[i];
			}
			highestInnerProduct = IP;
		}
	}
	for (int i = 0; i < T->tableSize; i++) {
		delete[] data[i];
	}
	delete[] data;
}

void BallTree::treeSearch(float* query, ballTreeNode* T, float &highestInnerProduct, float* &mipQuery) {
	if (highestInnerProduct < MIP(query, T)) {
		if (T->indexData != -1) {			// if T is a leaf do linearSearch
			linearSearch(query, T, highestInnerProduct, mipQuery);
		}
		else {
			ballTreeNode* leftChild;
			getNode(T->index + 1, leftChild);
			ballTreeNode* rightChild;
			getNode(T->indexRight, rightChild);
			float iL = MIP(query, leftChild);
			float iR = MIP(query, rightChild);
			if (iL <= iR) {
				treeSearch(query, leftChild, highestInnerProduct, mipQuery);
				delete leftChild;
				treeSearch(query, rightChild, highestInnerProduct, mipQuery);
				delete rightChild;
			}
			else {
				treeSearch(query, rightChild, highestInnerProduct, mipQuery);
				delete rightChild;
				treeSearch(query, leftChild, highestInnerProduct, mipQuery);
				delete leftChild;
			}
		}
	}
}

int BallTree::mipSearch(int d, float* query) {
	this->dimension = d;
	indexSlotSize = (sizeof(int) + sizeof(int) + sizeof(float) * dimension + sizeof(float)); // Bytes
	indexSlotNum = PAGE_SIZE / indexSlotSize; // one page 64KB
	dataSlotSize = (sizeof(float) * dimension); // Bytes
	dataSlotNum = PAGE_SIZE / dataSlotSize; // one page 64KB
	float* mipQuery = new float[d];
	float highestInnerProduct = -3.40E+38;
	getNode(0, root);
	treeSearch(query, root, highestInnerProduct, mipQuery);
	delete root;
	root = NULL;
	int a = (int)mipQuery[0];
	delete[] mipQuery;
	return a;
}