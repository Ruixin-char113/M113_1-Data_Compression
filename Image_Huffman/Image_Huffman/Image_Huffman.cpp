#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <fstream>
#include <bitset>
#define pixelLengthWidth = 512
using namespace std;

void writeBMPHeader(FILE* sourceData, const char* outputFileName) {
    FILE* outputData;
    errno_t err;
    // 讀取 BMP 檔頭
    unsigned char header[1078];

    if (fread(header, 1, 1078, sourceData) != 1078) {
        cout << "[ERROR] Failed to read BMP header!" << endl;
        fclose(sourceData);
    }

    err = fopen_s(&outputData, outputFileName, "wb");
    if (err != 0) {
        cout << "[ERROR] Failed to open output file: \"" << outputFileName << "\"!" << endl;
        //delete[] imageData;
        return;
    }

    cout << "[SUCCEED] Write to \"" << outputFileName << "\"" << endl;
    fwrite(header, 1, 1078, outputData);
    fclose(outputData);
}

void writeBMPPixelArray(FILE* sourceData, int* pixelCount) {
    // FILE* outputData;
    errno_t err;

 /*   if (fread(header, 1, 1078, sourceData) != 1078) {
        cout << "[ERROR] Failed to read BMP header!" << endl;
        fclose(sourceData);
    }*/

    unsigned char buffer[1]; // 暫存緩衝區
    size_t bytesRead;

    int count = 0;
    while ((bytesRead = fread(buffer, 1, sizeof(buffer), sourceData)) > 0) {
        count++;
        if (count > 512 * 512)
            break;
        if (0 <= buffer[0] && buffer[0] <= 255) {
            pixelCount[buffer[0]] += 1;
        }
    }

    /*for (int i = 0; i < 255; i++) {
        cout << pixelCount[i] << ' ';
        if (i % 10 == 0)
            cout << endl;
    }*/
}

void linkNode(const int* pixelCount){
    
}

//================================================================================================================================
class node {
public:
    void setPreNode(node* preNode_) {
        preNode = preNode_;
    }
    void setNextNode(node* nextNode_) {
        nextNode = nextNode_;
    }
    void setLeftChild(node* leftChild_) {
        leftChild = leftChild_;
    }
    void setRightChild(node* rightChild_) {
        rightChild = rightChild_;
    }
private:
    // For linklist
    node* preNode    = NULL;
    node* nextNode   = NULL;
    // For huffman tree
    node* leftChild  = NULL;
    node* rightChild = NULL;
};

class huffmanTree {
public:

private:
};
//================================================================================================================================
int main() {
    // Input or Output file name
	const char* openFileName   = "lena_gray.bmp";
    const char* outputFileName = "bmpHeader.txt";

	int pixelCount[256] = { 0 };
	FILE* sourceData;
	errno_t err;

	err = fopen_s(&sourceData, openFileName, "rb");
	if (err == 0) {
		cout << "[SUCCEED] Open File: \"" << openFileName << "\"!" << endl;

        // Handle BMPHeader
        writeBMPHeader(sourceData, outputFileName);

        // Collect pixel value to pixelCount
        writeBMPPixelArray(sourceData, pixelCount);

//================================================================================================================================
// 
//================================================================================================================================
        // Close File
		fclose(sourceData); 

		cout << "[INFO] File read completed!" << endl;
	}
	else {
		cout << "[ERROR] Failed to open file: \"" << openFileName << "\"!" << endl;
	}
}

