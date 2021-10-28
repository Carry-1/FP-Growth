#include <iostream>
#include <fstream>
#include <vector>
#include <algorithm>
#include <cmath>
#include <map>
#include <unordered_set>
#include <set>
#include <unordered_map>
#include <string>
using namespace std;

struct FpNode {
    int value = -1;
    int count;
    FpNode* parent;
    FpNode* next;
    unordered_map<int, FpNode*> children;
};

const double minsupport = 0.2;
const string inputFileName = "data.txt";
const string outputFileName = "out.txt";
int minCount;
vector<vector<int>> results;
FpNode* contructTree(vector<vector<int>>&, vector<int>, vector<int>);



//read input file and converse its content to 2D array 
vector<vector<int>> dataInit() {
    ifstream infile;
    infile.open(inputFileName, ios::in);
    if (!infile.is_open())
    {
        cout << "读取文件失败" << endl;
        return {};
    }
    string buf;
    vector<vector<int>> data;
    int index = 0;
    while (getline(infile, buf))
    {
        int begin = 0, end = -1;
        data.push_back(vector<int>());
        while ((end = buf.find("  ", begin)) != buf.npos) {
            data[index].push_back(atoi(buf.substr(begin, end - begin).c_str()));
            begin = end + 2;
        }
        sort(data[index].begin(), data[index].end());
        index++;
    }
    infile.close();
    return data;
}
//output results to file
void outputFile(vector<vector<int>> &results){
    cout << results.size();
    ofstream dataFile;
    dataFile.open(outputFileName, std::ios_base::app | std::ios_base::in);
    dataFile << "minsupport:" << minsupport << endl;
    for (auto result : results) {
        dataFile << "{";
        for (auto item : result) {
            dataFile << item << ',';
        }
        dataFile << "},";
    }
    dataFile << endl;
    dataFile.close();
}

vector<pair<int, int>> getOrder(unordered_map<int, int>& count) {
    vector<pair<int, int>> order;
    for (auto& c : count) {
        if (c.second >= minCount) {
            order.push_back({ c.second, c.first });
        }
    }
    sort(order.begin(), order.end());
    return order;
}

vector<int> getPath(vector<int> &items, unordered_map<int, int> &count) {
    vector<int> path;
    for (auto& item : items) {
        if (count[item] >= minCount) {
            path.push_back(item);
        }
    }
    sort(path.begin(), path.end(), [&](int& a, int& b) {return count[a] > count[b]; });
    return path;
}

void mining(FpNode* root, vector<int> alpha, unordered_map<int, FpNode*>& head, vector<pair<int, int>> &order) {
    FpNode* r = root;
    vector<int> temp;
    while (r->children.size() == 1) {
        FpNode* t = r;
        for (auto child : r->children) {
            t = child.second;
        }
        r = t;
        temp.push_back(r->value);
    }
    if (r->children.size() == 0) {
        int n = temp.size();
        for (int i = 1; i < (1<<n); i++) {
            vector<int> ans = alpha;
            for (int j = 0; j < n; j++) {
                if ((i >> j) & 1) ans.push_back(temp[j]);
            }
            results.push_back(ans);
        }
        return;
    }
    else {
        for (auto o : order) {
            int num = o.second;
            vector<int> ans = alpha;
            ans.push_back(num);
            results.push_back(ans);
            FpNode* h = head[num];
            vector<vector<int>> data;
            vector<int> cnt;
            while (h) {
                FpNode* p = h;
                cnt.push_back(p->count);
                vector<int> path;
                while (p->parent->value != -1) {
                    p = p->parent;
                    path.push_back(p->value);
                    
                }
                data.push_back(path);
                h = h->next;
            }
            vector<int> next_alpha = alpha;
            next_alpha.push_back(num);
            contructTree(data, cnt, next_alpha);
        }
    }

}

FpNode* contructTree(vector<vector<int>> &data, vector<int> cnt, vector<int> alpha) {
    FpNode* fp = new FpNode();
    unordered_map<int, FpNode*> head;
    unordered_map<int, int> count;
    for (int i = 0; i < data.size(); i++) {
        auto items = data[i];
        for (auto& item : items) {
            count[item]+=cnt[i];
        }
    }
    vector<pair<int, int>> order = getOrder(count);
    for (int i = 0; i < data.size(); i++) {
        vector<int> items = data[i];
        vector<int> path = getPath(items, count);
        FpNode* p = fp;
        for (auto node : path) {
            if (p->children.find(node) == p->children.end()) {
                p->children[node] = new FpNode();
                if (head.find(node) == head.end()) head[node] = p->children[node];
                else {
                    FpNode* q = head[node];
                    while (q->next) q = q->next;
                    q->next = p->children[node];
                }
                p->children[node]->parent = p;
                p->children[node]->value = node;
            }
            p = p->children[node];
            p->count += cnt[i];
        }
    }
    if (fp->children.size()) {
        mining(fp, alpha, head, order);
    }
    return fp;
}

int main(){
    
    vector<vector<int>> data = dataInit();
    minCount = ceil((minsupport * data.size()));
    contructTree(data, vector<int>(data.size(), 1), {});
    outputFile(results);
    cout << "END" << endl;



    

}
