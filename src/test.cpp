#include "BallTree.h"
#include "Utility.h"

//#define Netflix
#define MNIST
#define L 256

#ifdef MNIST
char dataset[L] = "Mnist";
int n = 60000, d= 51;
int qn = 1000;
#endif

#ifdef Netflix
char dataset[L] = "Netflix";
int n = 17770, d = 51;
int qn = 1000;
#endif // Netflix

#ifdef YAHOO
char dataset[L] = "Yahoo";
int n = 624, d = 300;
int qn = 1000;
#endif


int main() {
	char data_path[L], query_path[L];
	char index_path[L], output_path[L];
	float** data = nullptr;
	float** query = nullptr;

	sprintf(data_path, "%s/src/dataset.txt", dataset);
	sprintf(query_path, "%s/src/query.txt", dataset);
	sprintf(index_path, "%s/index", dataset);
	sprintf(output_path, "%s/dst/answer.txt", dataset);

	if (!read_data(n, d, data, data_path)) {
		return 1;
	}

	BallTree ball_tree1;
	ball_tree1.buildTree(n, d, data);
	ball_tree1.storeTree(index_path);
	ball_tree1.clear();

	if (!read_data(qn, d, query, query_path)) {
		return 1;
	}
	FILE* fout = fopen(output_path, "w");
	if (!fout) {
		printf("can't open %s!\n", output_path);
		return 1;
	}

	BallTree ball_tree2;
	ball_tree2.restoreTree(index_path);
	for (int i = 0; i < qn; i++) {
		int index = ball_tree2.mipSearch(d, query[i]);
		cout << i << endl;
		fprintf(fout, "%d\n", index);
	}
	system("pause");
	fclose(fout);
	return 0;
}
