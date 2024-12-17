#include <iostream>
#include <iomanip>
#include <string>
#include <map>
#include <fstream>
#include <vector>
#include <queue>

using namespace std;

struct CountNode{
    int count;
    CountNode* left;
    CountNode* right;

    CountNode(int c = 0, CountNode* l = nullptr, CountNode* r = nullptr){
        count = c;
        left = l;
        right = r;
    }
    ~CountNode(){ // will recursively delete pointers to remaining nodes
        delete left;
        delete right;
    }

    /*
    * The (pre-order) traversal to encode the tree. Only leaves (HuffmanTreeNodes) are encoded.
    * @param codeMap The map between a character and its binary (as a string).
    * @param prefixBits The prefix of bits representing the encoding to this node.
    */
    virtual void traverseForCodeMap(unordered_map<char, string>& codeMap, string prefixBits = ""){
        if (this->left)
            left->traverseForCodeMap(codeMap, prefixBits+"0");
        if (this->right)
            right->traverseForCodeMap(codeMap, prefixBits+"1");
    }
    
    /*
    * The struct containing a special function to for initializing priority queues.
    */
    struct NodeComparison {
        // The special comparison function for priority_queue
        bool operator()(CountNode* const& node1, CountNode* const& node2){ 
            return node1->count > node2->count; // min heap
        }
    };
};
struct HuffmanTreeNode: CountNode{
    char letter;

    HuffmanTreeNode(char ch, int c = 0, HuffmanTreeNode* l = nullptr, HuffmanTreeNode* r = nullptr): CountNode(c, l, r){
        letter = ch;
    }

    /*
    * Saves the encoding for HuffmanTreeNodes when visited by a CountNode's invocation of traverseForCodeMap.
    * @param codeMap The map between a character and its binary (as a string).
    * @param bits The prefix of bits representing the encoding to this node.
    */
    void traverseForCodeMap(unordered_map<char, string>& codeMap, string bits = "") override {
        codeMap[letter] = bits;
    }
};
struct HuffmanTree{
    CountNode* root;
    priority_queue<CountNode*, vector<CountNode*>, CountNode::NodeComparison> nodeHeap;
    unordered_map<char, string> codeMap; // char to code
    unordered_map<string, char> reverseCodeMap; // code to char

    ~HuffmanTree(){
        delete root;
    }

    /*
    * Counts the frequencies of each character in a file and creates HuffmanTreeNodes for each one. The nodes are stored in nodeHeap.
    * @param fileName The name of the file from which text is read.
    */ 
    void countCharFrequencies(string fileName){
        ifstream input(fileName);
        unordered_map<char, int> charCounts;
        char ch;

        while (input.get(ch)){
            if (charCounts.find(ch) != charCounts.end()){
                charCounts[ch] += 1;
            } else {
                charCounts[ch] = 1;
            }
        }
        input.close();
        if (charCounts.empty()) return; // no text entered
        // cout << "total entries: " << charCounts.size() << endl;

        // set up min-heap to build tree leafs-up
        for (const pair<char, int> p: charCounts){
            // cout << p.first << ": " << p.second << endl;
            HuffmanTreeNode* node = new HuffmanTreeNode(p.first, p.second);
            nodeHeap.push(node);
        }
    }

    /*
    * Builds a huffman coding tree by depleting nodeHeap and updates root.
    * Precondition: countCharFrequencies() has been called, and the ifstream has at least one character.
    */
    void buildTree(){
        if (nodeHeap.empty()) return;
 
        // first letter node (child on right)
        if (root) delete root;
        codeMap.clear();
        root = new CountNode();
        root->right = nodeHeap.top();
        nodeHeap.pop();
        root->count = root->right->count;
        
        // second letter node (next greatest; child on left)
        if (!nodeHeap.empty()){
            root->left = nodeHeap.top();
            nodeHeap.pop();
            root->count += root->left->count;
        }
        nodeHeap.push(root);

        // add remaining nodes
        while (nodeHeap.size() > 1){
            CountNode* next = new CountNode();
            next->right = nodeHeap.top();
            nodeHeap.pop();
            next->left = nodeHeap.top();
            nodeHeap.pop();
            next->count = next->right->count + next->left->count;

            nodeHeap.push(next);
            // cout << next->letter << endl;
        }

        // update root node
        root = nodeHeap.top();
        nodeHeap.pop();
        
        // store the binary of each character in the tree
        // codeMap is edited in-place
        root->traverseForCodeMap(codeMap);
    }

    /*
    * Export the Huffman coding tree and encoded text to a file.
    * Exports in the format "[char][code]\n"..."[char][code]\n\n[encodedText]"
    * @param fileName The name of the file to be encoded.
    */
    virtual void encode(string fileName){
        ifstream input(fileName);
        int index = 0;
        while (fileName.find('.', index+1) != -1){
            index = fileName.find('.', index+1);
        }
        ofstream output(fileName.substr(0, index) + "_encoded.txt");

        countCharFrequencies(fileName);
        buildTree(); 

        // export character encodings (codeMap)
        for (pair<char, string> p: codeMap){
            output << p.first << p.second << endl;
        }
        output << endl;

        // export encoded file
        char ch;
        while (input.get(ch)){
            output << codeMap[ch];
        }

        input.close();
        output.close();
    }

    /*
    * Import a Huffman coding tree from a file.
    * The file should be in the format of "[char][code]\n"..."[char][code]\n\n".
    * @param input The file stream containing tree mappings followed by an empty line.
    */
    void reconstructTree(istream& input){
        if (root) delete root;
        codeMap.clear();
        reverseCodeMap.clear();
        CountNode* root = new CountNode();

        // rebuild the tree
        string line;
        char ch;
        string s_binary;
        int binary;
        while (getline(input, line)){
            if (line.empty()) break;

            ch = line[0];
            s_binary = line.substr(1);
            codeMap[ch] = s_binary;
            reverseCodeMap[s_binary] = ch;
            binary = stoi(s_binary);

            CountNode* current = root;
            int length = s_binary.length();
            while (length-- > 1){
                if ((binary & 1) == 1){
                    if (!current->right) 
                        current->right = new CountNode();
                    current = current->right;
                } else {
                    if (!current->left) 
                        current->left = new CountNode();
                    current = current->left;
                }
                binary >>= 1;
            }
            if ((binary & 1) == 1){
                current->right = new HuffmanTreeNode(ch);
            } else {
                current->left = new HuffmanTreeNode(ch);
            }
        }
    }
    /*
    * Import a Huffman coding tree given tree mappings and encoded text. Then, decode and export the text.
    * @param fileName The name of the file stream containing tree mappings ("[char][code]\n"..."[char][code]\n\n") and the binary encoding to be decoded.
    */
    void decode(string fileName){
        ifstream input(fileName); 
        int index = 0;
        while (fileName.find('.', index+1) != -1){
            index = fileName.find('.', index+1);
        }
        ofstream output(fileName.substr(0, index) + "_decoded.txt");

        // reconstruct tree and codeMap
        reconstructTree(input);

        // decode binary
        string binary = "";
        char ch;
        while (input >> ch){
            binary += ch;
            if (reverseCodeMap.find(binary) != reverseCodeMap.end()){
                output << reverseCodeMap[binary];
                binary = "";
            }
        }

        input.close();
        output.close();
    }
};

int main() { 

    string fileName;
    ifstream input;
    unsigned int option;
    HuffmanTree tree;
    
    // get the file name
    do {
        cout << "Enter a file name: ";
        getline(cin, fileName);
        input.open(fileName);

        if (!input.is_open()){
            cout << endl << "Error opening file. Please try again." << endl;
            input.clear();
        }
    } while (!input.is_open());
    input.close();

    // get the encode/decode option
    do {
        cout << setfill('#') << setw(10) << ' ' << "Menu: " << setfill('#') << setw(10) << ' ' << endl
            << "1: Encode" << endl
            << "2: Decode" << endl
            << "Would you like to encode or decode? Option: ";
        cin >> option;

        if (cin.fail() || option != 1 || option != 2){
            cin.clear(); 
            cin.ignore(numeric_limits<streamsize>::max(), '\n'); 
            cout << "Invalid input." << endl << endl;
        }
    } while (!option || option > 2);

    // perform the operation
    switch(option){
        case 1: // encode
            tree.encode(fileName);
            cout << "Encoding complete." << endl;
            break;
        case 2: // decode
            tree.decode(fileName);
            cout << "Decoding complete." << endl;
            break;
    }

    return 0;

    // rm -f lorem_encoded.txt & rm -f lorem_encoded_decoded.txt & rm -f huffmantree & g++ ./huffmantree.cpp -o ./huffmantree
    // ./huffmantree
}
