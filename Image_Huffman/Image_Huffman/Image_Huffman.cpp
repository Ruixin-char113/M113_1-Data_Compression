#include <iostream>
#include <string>
#include <queue>
#include <vector>
#include <fstream>
#include <bitset>
#define pixelLengthWidth = 512
using namespace std;

// write BMP Header to file
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

// count and write "BMP pixel array" to pixelCount[]
void writeBMPPixelArray(FILE* sourceData, int* pixelCount) {

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
}


//================================================================================================================================
class Node {
public:
    Node(int name_, int freq_) {
        setName(name_);
        setFreq(freq_);
    }
    // set
    void setName(int name_) {
        name = name_;
    }
    void setFreq(int freq_) {
        freq = freq_;
    }
    void setPreNode(Node* preNode_) {
        preNode = preNode_;
    }
    void setNextNode(Node* nextNode_) {
        nextNode = nextNode_;
    }
    void setLeftChild(Node* leftChild_) {
        leftChild = leftChild_;
    }
    void setRightChild(Node* rightChild_) {
        rightChild = rightChild_;
    }

    // get
    int getName() {
        return name;
    }
    int getFreq() {
        return freq;
    }
    Node* getPreNode() {
        return preNode;
    }
    Node* getNextNode() {
        return nextNode;
    }
private:
    // Base Info
    int   name       = NULL;
    int   freq       = NULL;

    // For linklist
    Node* preNode    = NULL;
    Node* nextNode   = NULL;

    // For huffman tree
    Node* leftChild  = NULL;
    Node* rightChild = NULL;
};

class HuffmanTree {
public:
    void linkNode(const int* pixelCount) {
        for (int i = 0; i <= 255; i++) {
            // nodeName = i; nodeFreq = pixelCount[i];
            Node* node = new Node(i, pixelCount[i]);

            // set preNode, preNode.nextNode;
            node->setPreNode(endLinkNode);
            if(endLinkNode != NULL)
                endLinkNode->setNextNode(node);

            // Update endLinkNode
            endLinkNode = node;

            // init firstLinkNode
            if (firstLinkNode == NULL) {
                firstLinkNode = node;
            }
        }
        cout << "[SUCCEED] Link ALL node" << endl;
    }

    void buildHuffmanTree() {
        cout << "[START] Creating Huffman Tree" << endl;
        // For huffman tree create node;
        Node* left     = NULL;
        Node* right    = NULL;
        Node* nextNode = NULL;

        // create tree, all node of linklist
        while (firstLinkNode != endLinkNode) {
            // left, right = first, second node; nextNode = right->getNextNode();
            left     = firstLinkNode;
            right    = firstLinkNode->getNextNode();
            nextNode = right->getNextNode();

            // find the two small freq node left and right
            while (nextNode != NULL) {
                // Three node's freq
                int nextNodeFreq  = nextNode->getFreq();
                int leftNodeFreq  = left->getFreq();
                int rightNodeFreq = right->getFreq();

                // No chance
                if (nextNodeFreq > leftNodeFreq && nextNodeFreq > rightNodeFreq) {
                    nextNode = nextNode->getNextNode();
                    continue;
                }
                // Kick left
                else if (leftNodeFreq >= rightNodeFreq) {
                    left  = right;
                    right = nextNode;
                }
                // Kick right
                else if (rightNodeFreq > leftNodeFreq) {
                    right = nextNode;
                }

                // pointer to nextNode
                nextNode = nextNode->getNextNode();
            }

            // Create huffman node
            {
                // node name = -1, freq = leftFreq + rightFreq;
                Node* parentNode = new Node(-1, left->getFreq() + right->getFreq());
                parentNode->setLeftChild(left);
                parentNode->setRightChild(right);

                // set root to new node
                root = parentNode;

                // add to end of linklist
                //endLinkNode->getPreNode()->setNextNode(parentNode);
                parentNode->setPreNode(endLinkNode);
                endLinkNode->setNextNode(parentNode);
                endLinkNode = parentNode;
            }

            // relink linklist
            {
                Node* linkPreNode;
                Node* linkNextNode;
                // left
                // link left->prenode to left->nextnode;
                linkPreNode = left->getPreNode();
                linkNextNode = left->getNextNode();
                if (linkPreNode != NULL) {
                    // pre to next
                    linkPreNode->setNextNode(left->getNextNode());
                }
                if (linkNextNode != NULL) {
                    // next to pre
                    linkNextNode->setPreNode(left->getPreNode());
                }
                // right
                // link right->prenode to right->nextnode;
                linkPreNode = right->getPreNode();
                linkNextNode = right->getNextNode();
                if (linkPreNode != NULL) {
                    // pre to next
                    linkPreNode->setNextNode(right->getNextNode());
                }
                if (linkNextNode != NULL) {
                    // next to pre
                    linkNextNode->setPreNode(right->getPreNode());
                }
            }

            // relnik fistLinkNode, endLinkNode
            {
                // relink firstLinkNode
                if (firstLinkNode == left) {
                    firstLinkNode = left->getNextNode();
                    if (firstLinkNode == right) {
                        firstLinkNode = right->getNextNode();
                    }
                }
                // relink endLinkNode
                if (endLinkNode == right) {
                    endLinkNode = right->getPreNode();
                    if (endLinkNode == left)
                        endLinkNode = left->getPreNode();
                }
            }
        }
        cout << "[SUCCEED] Create Huffman Tree" << endl;
    }
private:
    // For linklist
    Node* firstLinkNode = NULL;
    Node* endLinkNode   = NULL;
    // For huffman tree
    Node* root          = NULL;
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
        HuffmanTree huff;

        huff.linkNode(pixelCount);
        huff.buildHuffmanTree();
//================================================================================================================================
        // Close File
		fclose(sourceData); 

		cout << "[INFO] File read completed!" << endl;
	}
	else {
		cout << "[ERROR] Failed to open file: \"" << openFileName << "\"!" << endl;
	}
}

