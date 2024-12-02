#include <iostream>
#include <string>
#include <stack>
#include <queue>
#include <vector>
#include <fstream>
#include <bitset>
#include <algorithm>
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

// write huffman table to "huffTable.txt"
void writeHuffmanTable(const char* outputFileHuffTable, const string* huffTable) {
    ofstream huffTableStream;

    // open "huffTable.txt"
    huffTableStream.open(outputFileHuffTable);
    if (!huffTableStream.is_open()) {
        cout << "[ERROR] Can't Open \"" << outputFileHuffTable << "\"" << endl;
        huffTableStream.close();
        return;
    }

    for (int i = 0; i < 256; i++) {
        huffTableStream << huffTable[i] << endl;
    }

    cout << "[SUCCEED] Write to \"" << outputFileHuffTable << "\"" << endl;
    huffTableStream.close();
}

// Push huffman code to ofstream
void pushOutHuffmanCode(ofstream* lennaCompressionStream, string* buffString, string* huffCodeString, bool endOfData, int *countByte) {
    do{
        // Total of buffer can store
        int pushLength = 8 - buffString->length();
        // Total of huffman can push
        int pushLimit = huffCodeString->length();
        // Calculate min of push bit
        int pushBit = min(pushLength, pushLimit);

        // Push huffman code to buffer
        *buffString += huffCodeString->substr(0, pushBit);
        if (buffString->length() == 8) {
            char buffChar = static_cast<char>(bitset<8>(*buffString).to_ullong());
            *lennaCompressionStream << buffChar;
            *countByte += 1;
            buffString->clear();
        }

        // Adjust huffCodeString
        if (huffCodeString->length() - pushBit)
            *huffCodeString = huffCodeString->substr(pushBit, huffCodeString->length() - pushBit);
        else
            *huffCodeString = "";

        // Endofdata && buffer has data && No other data
        if (endOfData && !buffString->empty() && huffCodeString->empty()) {
            int addLimit = 8 - buffString->length();
            for (int i = 0; i < addLimit; i++)
                *buffString += "0";
            char buffChar = static_cast<char>(bitset<8>(*buffString).to_ullong());
            *lennaCompressionStream << buffChar;
            *countByte += 1;
            buffString->clear();
        }
            
    }while (!huffCodeString->empty());
}

// use huffman table encode lenna pixel array
void writeLennaCompression(FILE* sourceData, const char* outputFileLennaCompression, const string* huffTable) {
    ofstream lennaCompressionStream;

    // open "lennaCompression.txt"
    lennaCompressionStream.open(outputFileLennaCompression, ios::out | ios::binary);
    if (!lennaCompressionStream.is_open()) {
        cout << "[ERROR] Can't Open \"" << outputFileLennaCompression << "\"" << endl;
        lennaCompressionStream.close();
        return;
    }

    // From 1078 to "1078 + 512 * 512"
    int iLimit = 1078 + 512 * 512;
    // For output code
    string buffString;
    // End of data
    bool endOfData = 0;

    int countByte = 0;
    for (int i = 1078; i < iLimit; i++) {
        if (i == iLimit - 1)
            endOfData = 1;
        // move to next pixel
        if (fseek(sourceData, i, SEEK_SET) == 0) {
            // read 1 pixel
            unsigned char pixelValue;
            if (fread(&pixelValue, 1, 1, sourceData) == 1) {
                // huffman code
                string huffCodeString = huffTable[(int)pixelValue];

                // Push huffman code to buffString
                pushOutHuffmanCode(&lennaCompressionStream, &buffString, &huffCodeString, endOfData, &countByte);

                lennaCompressionStream;
                /*for (int stringPointer = 0; stringPointer < huffTable[(int)pixelValue].length(); stringPointer++) {
                    if (huffTable[(int)pixelValue][stringPointer] == '1')
                        lennaCompressionStream <<  bitset<1>('1'-'0');
                    else
                        lennaCompressionStream <<  bitset<1>('0'-'0');
                }*/

            }
        }
    }

    cout << "\t[CALCULATE] Bits per pixel(contain padding bits): " << ((double)countByte * 8) / (262144) << endl;
    lennaCompressionStream.close();
    cout << "[SUCCEED] Write to \"" << outputFileLennaCompression << "\"" << endl;
}

// ===============================================================================================================================

// Store Huffman Code for decode
void readHuffmanTable(const char* inputFileHuffmanTable, string* huffTableDecode) {

    // Open "huffTable.txt"
    ifstream huffmanTable(inputFileHuffmanTable);
    if (!huffmanTable.is_open()) {
        cout << "[ERROR] Can't Open \"" << inputFileHuffmanTable << "\"" << endl;
        huffmanTable.close();
        return;
    }

    // Read huffman code to "huffTableDecode[]"
    string tempHuffmanCode;
    int lineNumber = 0;
    while (getline(huffmanTable, tempHuffmanCode)) {
        huffTableDecode[lineNumber] = tempHuffmanCode;
        lineNumber++;
    }

    /*for (int i = 0; i < 256; i++) {
        cout << huffTableDecode[i] << endl;
    }*/

    cout << "[SUCCEED] Store Huffman Table from: \"" << inputFileHuffmanTable << "\"" << endl;
    huffmanTable.close();
}

void decodeHuffmanPushOut(ofstream* image_rBMP, const char readChar, string* bufferString, const string* huffTableDecode, bool endOfData) {
    // Store encode huffman code
    string huffmanString = bitset<8>(static_cast<unsigned char>(readChar)).to_string();

    /*int pointerHuffmanString = 0;*/
    while (!huffmanString.empty()) {
        // add 1 char to bufferString
        *bufferString += huffmanString.substr(0, 1);
        // update huffmanString: << 1 char
        if (huffmanString.length() > 1)
            huffmanString = huffmanString.substr(1, huffmanString.length() - 1);
        else
            huffmanString.clear();

        // check 0 - 255
        for (int checkPointer = 0; checkPointer < 256; checkPointer++) {
            if (huffTableDecode[checkPointer] == *bufferString) {
                // cout << hex << checkPointer << " ";

                *image_rBMP << (char)checkPointer;

                bufferString->clear();
            }
        }

        // point to next string char;
        //pointerHuffmanString++;
    }
}

// Decode
void decodeHuffman(const char* inputFileLennaCompressed, ofstream* image_rBMP, const string* huffTableDecode) {
    ifstream lennaCompressed(inputFileLennaCompressed, ios::in | ios::binary);

    // Open "lennaCompression.txt"
    if (!lennaCompressed.is_open()) {
        cout << "[ERROR] Can't Open \"" << inputFileLennaCompressed << "\"" << endl;
        lennaCompressed.close();
        return;
    }

    // Read encode Huffman code byte by byte
    char readChar;
    string bufferString;
    bool endOfData = 0;
    int countHandleByte = 0;
    while (lennaCompressed.read(&readChar, sizeof(readChar))) {
        /*cout << hex << uppercase << (0xFF & readChar) << " ";*/
        if (countHandleByte == 512 * 512 - 1)
            endOfData = true;

        decodeHuffmanPushOut(image_rBMP, readChar, &bufferString, huffTableDecode, endOfData);

        countHandleByte++;
    }

    lennaCompressed.close();
    cout << "Last buffer: \"" << bufferString << "\"" << endl;
    cout << "[SUCCEED] Decode from: \"" << inputFileLennaCompressed << "\"" << endl;
}

// write to lenna_r.bmp, read "bmpHeader.txt", 
void writeDecodeFile(const char* outputFIleImageR, const char* inputFileHeader,
                        const char* inputFileLennaCompressed, const string* huffTableDecode) {
    // "bmpHeader.txt", "lenna_r.bmp"
    ifstream bmpHeader(inputFileHeader, ios::in | ios::binary);
        if (!bmpHeader.is_open()) {
        cout << "[ERROR] Can't Open \"" << inputFileHeader << "\"" << endl;
        bmpHeader.close();
        return;
    }
    ofstream image_rBMP(outputFIleImageR, ios::out | ios::binary);
        if (!image_rBMP.is_open()) {
            cout << "[ERROR] Can't Open \"" << outputFIleImageR << "\"" << endl;
            image_rBMP.close();
            return;
        }

    // write BMP Header
    char readChar;
    int pointerReadChar = 0;
    while (bmpHeader.read(&readChar, sizeof(readChar))) {
        image_rBMP << readChar;
        //cout << hex << (int)(unsigned char)readChar << " ";
    }

    // write pixel array
    decodeHuffman(inputFileLennaCompressed, &image_rBMP, huffTableDecode);

    // add byte to pixel array, make it 4 bytes
    for (int addByte = 0; addByte < 1078 % 4; addByte++) {
        image_rBMP << char(0);
    }

    bmpHeader.close();
    image_rBMP.close();
    cout << "[SUCCEED] Write decode image: \"" << outputFIleImageR << "\"" << endl;
}

//================================================================================================================================

void calculateMSE(const char* sourceImage, const char* decodeImage) {
    ifstream sourceStream(sourceImage);
    ifstream decodeStream(decodeImage);

    // Open "lenna.bmp", "lenna_r.bmp"
    if (!sourceStream.is_open()) {
        cout << "[ERROR] Can't Open \"" << sourceImage << "\"" << endl;
        sourceStream.close();
        return;
    }
    if (!decodeStream.is_open()) {
        cout << "[ERROR] Can't Open \"" << decodeImage << "\"" << endl;
        decodeStream.close();
        return;
    }

    char sourceChar;
    char decodeChar;
    double MSE = 0;
    // Read 1 byte from both of images
    while (sourceStream.read(&sourceChar, sizeof(sourceChar)) && decodeStream.read(&decodeChar, sizeof(decodeChar))) {
        MSE += pow((sourceChar - decodeChar), 2);
    }

    MSE /= 262144;

    cout << "\t[CALCULATE] MSE (Mean-Square Error): " << MSE << endl;

    sourceStream.close();
    decodeStream.close();
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
    void setParentNode(Node* parentNode_) {
        parentNode = parentNode_;
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
    Node* getLeftChild() {
        return leftChild;
    }
    Node* getRightChild() {
        return rightChild;
    }
private:
    // Base Info
    int   name       = NULL;
    int   freq       = NULL;

    // For linklist
    Node* preNode    = NULL;
    Node* nextNode   = NULL;

    // For huffman tree
    Node* parentNode = NULL;
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
        //cout << "[START] Creating Huffman Tree" << endl;
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
                left->setParentNode(parentNode);
                right->setParentNode(parentNode);

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

    void sethuffValueFun(Node* currentNode, string huffValue, string* huffTable) {
        // go to leftChild
        if (currentNode->getLeftChild() != NULL)
            sethuffValueFun(currentNode->getLeftChild(), huffValue + '1', huffTable);
        // go to rightChild
        if (currentNode->getRightChild() != NULL)
            sethuffValueFun(currentNode->getRightChild(), huffValue + '0', huffTable);
        // handle leaf node
        if (currentNode->getLeftChild() == NULL && currentNode->getRightChild() == NULL) {
            //cout << "[" << currentNode->getName() << "] \t" << huffValue << endl;
            huffTable[currentNode->getName()] = huffValue;
        }
    }

    void setHuffValue(string* huffTable) {
        //unsigned int huffValue = 0;
        string huffValue;
        Node* initNode = root;

        sethuffValueFun(initNode, huffValue, huffTable);
        cout << "[SUCCEED] Set huffman value" << endl;
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
	const char* openFileName        = "lenna.bmp";
    const char* outputFileName      = "bmpHeader.txt";
    const char* outputFileHuffTable = "huffTable.txt";
    const char* outputFileLennaCompression = "lennaCompression.txt";
    const char* outputFIleImageR    = "lenna_r.bmp";

	int pixelCount[256] = { 0 };
    string huffTable[256];
    string huffTableDecode[256];
	FILE* sourceData;
	errno_t err;

	err = fopen_s(&sourceData, openFileName, "rb");
	if (err == 0) {
		cout << "[SUCCEED] Open File: \"" << openFileName << "\"" << endl;

        // Handle BMPHeader
        writeBMPHeader(sourceData, outputFileName);

        // Collect pixel value to pixelCount
        writeBMPPixelArray(sourceData, pixelCount);

        //======= Create Huffman Tree ===================================================
        HuffmanTree huff;

        huff.linkNode(pixelCount);
        huff.buildHuffmanTree();
        huff.setHuffValue(huffTable);
        //======= End of Create Huffman Tree ============================================
        
        // write huffman table to "huffTable.txt"
        writeHuffmanTable(outputFileHuffTable, huffTable);

        // compression lenna.bmp
        writeLennaCompression(sourceData, outputFileLennaCompression, huffTable);

        // Store huffman code table
        readHuffmanTable(outputFileHuffTable, huffTableDecode);
        
        // Write decode image
        writeDecodeFile(outputFIleImageR, outputFileName, outputFileLennaCompression, huffTableDecode);

        // Calculate MSE(Mean-Square Error)
        calculateMSE(openFileName, outputFIleImageR);

        // Close File
		fclose(sourceData); 

		cout << "[INFO] Process completed" << endl;
        system("pause");
	}
	else {
		cout << "[ERROR] Failed to open file: \"" << openFileName << "\"!" << endl;
	}
}

