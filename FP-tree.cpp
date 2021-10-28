# include<iostream>
# include<fstream>
# include<string>
# include<vector>
# include<cstring>
# include<unordered_set>
# include<set>
# include<map>
# include<unordered_map>
# include<functional>
#include <algorithm>

using namespace std;

struct FPTree{
    int val;
    int support_count; //支持度计数
    FPTree* parent;
    vector<FPTree*> children; //
    FPTree* next;   //指向FP树中与自己val相同的树结点
};

typedef FPTree* FPTreePointer;
typedef map<int, pair<int, FPTreePointer> > item_head_table;  //项头表,在FP-Growth过程中按项头表从下往上的顺序寻找各个项的条件模式基
typedef pair<FPTreePointer, item_head_table> tree_and_table;


vector<string> split(const string& str, const string& delim) //将输入字符串str按delim标志进行分割
{
    vector<string> vec_result;
	if("" == str) return vec_result; //如果要分割的串为空串，那么不进行分割

	//先将要切割的字符串从string类型转换为char*类型，之所以要这样做，是因为下面要调用的C库函数strtok的接口是这样要求的
	char * strs = new char[str.length() + 1] ; 
	strcpy(strs, str.c_str()); 
    
    //同上
	char * d = new char[delim.length() + 1];
	strcpy(d, delim.c_str());
 
	char *p = strtok(strs, d); //调用C库函数strok()进行字符串分割
	while(p) { 
		string s = p; //分割得到的字符串转换为string类型
		vec_result.push_back(s); //存入结果数组
		p = strtok(NULL, d); //这里之所以要这样写是因为strtok函数是多次调用的，详情见CSDN
	}
 
	return vec_result;
}

vector<vector<int> > Read(){ //读取txt文件，处理成一个vector<vector<int>>--二维整型数组，用于Apriori(...)函数的输入
    ifstream Infile("E:/Personal Blog/data-mining-labs/Lab-2nd/test.txt");
    vector<string> vec_str; //vec_str数组用于存储从data.txt文件中读取的每一行的内容--每一行的内容都是字符串
    vector<vector<string> > vec_substr; //vec_substr数组用于存储对vec数组中每一行分割过后的内容
    vector<vector<int> > vec_result;  //vec2用于存储将vec_substr中每个字符串转换为对应整数的形式
    string s; //字符串s用于存储从.txt文件中读取的每行内容
    while(getline(Infile,s)) //按行读取txt文件的内容--读取后的每行内容是字符串，存于vec_str中，
    {
        vec_str.push_back(s);
    }
    
    for(int i=0; i<vec_str.size(); i++) //对读取进来的每行原始字符串进行分割
    {
        string s;
        vec_substr.push_back(vector<string>());
        vec_substr[vec_substr.size()-1] = split(vec_str[i], "  "); //对vec数组的每一行按空格进行分割,原vec中每行字符串会被分割为多个字符串

    }

    for(int i=0; i<vec_substr.size(); i++) //将vec_substr中每个字符串转换为对应的整数
    {
        vec_result.push_back(vector<int>());
        for(int j=0; j<vec_substr[i].size(); j++)
        {
            char * strs = new char[vec_substr[i][j].length() + 1] ;//vec1中每个元素都是string类型的，下面要调用的库函数atoi的接口要求其传入参数是C类型的字符串  
            strcpy(strs, vec_substr[i][j].c_str());
            vec_result[vec_result.size()-1].push_back(atoi(strs)); //调用库函数atoi
        }
    }
    return vec_result;
}

void Insert_Tree(const vector<int>& transaction,  item_head_table& table, FPTreePointer T )
{
    if(transaction.size())
    {
        FPTreePointer child;
        int flag = 0;
        int p = *transaction.begin();
        for(int i=0; i<T->children.size(); i++)
        {
            if(T->children[i]->val==p)
            {
                flag = 1;
                T->children[i]->support_count++;
                child = T->children[i];
            }
        }

        if(flag==0) //新增节点
        {
            child = new FPTree;
            child->val = p;
            child->next = nullptr;
            child->parent = T;
            child->support_count = 1;
            child->children = {};
            T->children.push_back(child);  //将这个孩子指针链接到父节点上面去
            auto ite = table.find(p);
            /*
            */
            FPTreePointer pointer = ite->second.second;

            if(pointer==nullptr)  //如果项头表中这个item对应的指针为空，即这个项在树中是第一次出现
            {
                ite->second.second = child; //让项头表中这个item的指针指向这个item
            }
            else //若这个item在树中不是第一次出现，从项头表开始顺着链一直找，直到某个val相同的item的next为空
            {
                while(pointer->next)
                {
                    pointer = pointer->next;
                }
                pointer->next = child;
            }
        }
        auto ite = transaction.begin();
        vector<int> subtransaction(++ite, transaction.end());
        Insert_Tree(subtransaction, table, child);
    }
}
vector <pair<int, int> > PairSorting (vector< pair<int, int> >& vec ) // 将输入的pair按second降序排列
{
    for(int i=1; i<vec.size(); i++)
    {       
        if(vec[i].second>vec[i-1].second) //降序排列,所以是大于号
        {
            
            int temp1 = vec[i].first;
            int temp2 = vec[i].second;
            int j = 0;
            for( j=i-1; j>=0&&temp2 > vec[j].second; j--)
            {
                vec[j+1].first = vec[j].first;
                vec[j+1].second = vec[j].second;
            }
            vec[j+1].first = temp1;
            vec[j+1].second = temp2;
        }  
    }
    return vec;
}
 tree_and_table Build_FP_Tree(const vector<vector<int> >& Transaction_DataBase, double minsupport){ //生成FP-Tree以及对应的项头表
    //第一步：生成Transaction_DataBase的所有频繁1项集的集合，得到它们的支持度计数，并按降序排序
    unordered_set<int> s;
    unordered_multiset<int> s1;
    vector< pair<int,int> > s2;   //按降序存储所有的频繁1项集, 每个pair都是item和对应的支持度
    item_head_table table; //项头表，输入Transaction_DataBase的所有频繁1项集
    FPTreePointer NULL_Node = new FPTree;  //null 节点
    NULL_Node->val = 0; //初始化
    NULL_Node->next = nullptr;
    NULL_Node->children = {};
    NULL_Node->parent = nullptr;
    NULL_Node->support_count = 0;

    vector< unordered_set<int> > VecInput; //将输入事务数据库转换为哈希表数组
    vector< vector<int> > cast_database;

    for(int i=0; i<Transaction_DataBase.size(); i++) //将输入的VecInput的每一行转换为哈希set，便于查找
    {
        VecInput.push_back(unordered_set<int>()); //添加一个新行
        for(int j=0; j<Transaction_DataBase[i].size(); j++)
            VecInput[VecInput.size()-1].insert(Transaction_DataBase[i][j]);
    }

     for(int i=0; i<Transaction_DataBase.size(); i++)
    { //生成哈希set、哈希multiset
        for(int j=0; j<Transaction_DataBase[i].size(); j++)
        {
            s.insert(Transaction_DataBase[i][j]);
            s1.insert(Transaction_DataBase[i][j]);
        }

    }
    
    for(auto ite = s.begin(); ite!=s.end(); ite++) //生成项头表，但此时项头表中每一项的链接指针仍为空
    {
        if ((double)s1.count(*ite) >= minsupport )
        {

            s2.push_back(make_pair(*ite, s1.count(*ite)));
            table.insert(make_pair(*ite, make_pair(s1.count(*ite), nullptr))); //在项头表中插入新的一项
           
        }         
    }

/*
对s2排序，生成降序的频繁1项集
*/
    s2 = PairSorting(s2);   //此时的s2已经是降序的频繁1项集

    //生成投影事务数据库
    for(int i = 0; i<VecInput.size(); i++)
    {
        cast_database.push_back({}); //对投影数据库新增一行
        for(int j=0; j<s2.size(); j++) //对输入事务数据库的每一条事务分别处理
        {
            if(VecInput[i].count(s2[j].first))  //若在事务VecInput[i]中有频繁项s2[j]。first
            {
                cast_database[cast_database.size()-1].push_back(s2[j].first); //在投影事务数据库的最后一条事务中新增一项
            }
        }
    }

    for(int i=0; i<cast_database.size(); i++)
    {
        Insert_Tree(cast_database[i], table, NULL_Node);
    }


    tree_and_table conditional_FP_Tree = make_pair(NULL_Node,table); 
    return  conditional_FP_Tree;   //返回

}

vector< vector<int> > vec_sort(vector<vector<int>>& vec)
{
    for(int i=1; i<vec.size(); i++)
    {       
        if(vec[i][1]>vec[i-1][1])
        {
            
            int temp1 = vec[i][0];
            int temp2 = vec[i][1];
            int j = 0;
            for( j=i-1; j>=0&&temp2 > vec[j][1]; j--)
            {
                vec[j+1][0] = vec[j][0];
                vec[j+1][1] = vec[j][1];
            }
            vec[j+1][0] = temp1;
            vec[j+1][1] = temp2;
        }  
    }
    return vec;
}

/*
全排列
*/
vector< vector<int> >  Full_Permutation(const vector<int>& prefix_pattern, vector<int>& post_pattern , vector<vector<int>>& L)
{
    int n = prefix_pattern.size();
    int quotient = 2^n-1;

    for(int i=0; i<2^n; i++)
    {
        L.push_back({});
        for(int j=0; j<n; j++)
        {
            if(quotient%2==1)
            {
                L[L.size()-1].push_back(prefix_pattern[n-1-j]);
            }
            quotient/=2;
        }
        reverse(L[L.size()-1].begin(), L[L.size()-1].end());   //逆序一下
        for(int k=0; k<post_pattern.size(); k++)
        {
            L[L.size()-1].push_back(post_pattern[k]);
        }
    }
    return L;
}

vector<vector<int>> FP_Growth(vector<vector<int>> & L, const tree_and_table& tr_and_ta, vector<int>& postfix_pattern, double minsupport)
{
    FPTreePointer tree = tr_and_ta.first;
    if(tree->children.size()==1)  //如果这棵FP树只有一条路径
    {
        vector<int> vec;
        FPTreePointer child = tree->children[0];
        while(true) //如果当前结点
        {
            vec.push_back(child->val); //沿着路径从上往下存储
            if(!child->children.size()) //如果这是这条路径上最后一个节点，跳出循环
                break;
            child = child->children[0];
            
        }

        L = Full_Permutation(vec, postfix_pattern, L);
        
    }
    else //有多条路径
    {

            //先对项头表中各项的支持度按递减排序，传入的项头表并未按支持度排序
        item_head_table table = tr_and_ta.second;
        vector<vector<int>> tbl;  //它是一个二维数组，第一列保存item，第二列保存支持度
        for(auto ite = table.begin(); ite!=table.end(); ite++)
        { 
            tbl.push_back({});
            tbl[tbl.size()-1].push_back(ite->first);
            tbl[tbl.size()-1].push_back(ite->second.first); 
        }
        tbl = vec_sort(tbl); //对项头表中的item按支持度递减排序
        
        for(int i=tbl.size()-1; i>=0; i--) //对项头表中的每一个后缀模式进行模式挖掘
        {   L.push_back({});
            L[L.size()-1].push_back(tbl[i][0]);
            for(int j=0; j<postfix_pattern.size(); j++)
            {
                L[L.size()-1].push_back(postfix_pattern[j]);
            }
            //先寻找条件模式基，利用项头表中存的链寻找每个后缀模式对应的条件模式基
            int key = tbl[i][0];
            auto ite = table.find(key);
            vector<FPTreePointer> condition_pattern_base;
            FPTreePointer fp = ite->second.second;
            while(fp)
            {
                condition_pattern_base.push_back(fp);
                fp = fp->next;
            }
            //挖掘项头表中该项对应的所有条件模式基
            vector<vector<int>> sub_transaction_data_base;
            for(int i=0; i<condition_pattern_base.size(); i++) //遍历所有的条件模式基，注意：一个条件模式基可能生成多条事务
            {
                FPTreePointer p = condition_pattern_base[i]->parent;
                sub_transaction_data_base.push_back({});
                while(p->val!=0) //Null结点的val为0
                {
                    sub_transaction_data_base[sub_transaction_data_base.size()-1].push_back(p->val);
                    p = p->parent;
                }
                reverse(sub_transaction_data_base[(sub_transaction_data_base.size()-1)].begin(),sub_transaction_data_base[(sub_transaction_data_base.size()-1)].end());
                for(int j=0; j<condition_pattern_base[i]->support_count-1; j++)
                {
                    sub_transaction_data_base.push_back( {} );
                    sub_transaction_data_base[sub_transaction_data_base.size()-1] = sub_transaction_data_base[sub_transaction_data_base.size()-2];
                }
            }

            tree_and_table tr_and_ta_1 = Build_FP_Tree(sub_transaction_data_base, minsupport);

            if(tr_and_ta_1.first->children.size()) //如果生成的FP树不为空
            {
                FP_Growth(L, tr_and_ta_1, L[L.size()-1], minsupport); //递归调用FP_Growth函数
            }
            
        }

    }
    
    return L;
}

int main()
{
    vector<vector<int> > Transaction_DataBase; 
    double minsupport = 2;
    Transaction_DataBase = Read(); //生成初始事务数据库

    tree_and_table TreeAndTable; 
    TreeAndTable = Build_FP_Tree(Transaction_DataBase, minsupport); //建立初始FP_Tree
    vector<vector<int>> L; //用于存储所有的频繁模式
    vector<int> postfix_pattern({});  //初始频繁模式为空
    L = FP_Growth(L, TreeAndTable,  postfix_pattern, minsupport); //调用FP_Growth算法
    
    return 1;
}


 