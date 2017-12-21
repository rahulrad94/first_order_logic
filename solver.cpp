#include<bits/stdc++.h>
using namespace std;

struct Predicate{
    string name;
    vector<string> arguements;
    vector<char> arguementType;
    int numberOfArguements;
    bool isNegated;
};

struct Sentence{
    string name;
    vector<Predicate> predicate;
    int numberOfPredicates;
};

//global variables
int numberOfQueries, numberOfSentences;
vector<Sentence> KB;
vector<string> queries;
int standardizeCount = 1;

// delimeter of size 1 only
vector<string> splitter(string s, char delimeter){
    size_t pos = 0;
    string token;
    vector<string> v;

    while ((pos = s.find(delimeter)) != string::npos) {
        token = s.substr(0, pos);
        v.push_back(token);
        s.erase(0, pos + 1);
    }

    //get the last predicate
    token = s.substr(0, s.length());
    v.push_back(token);
    return v;

}

Predicate predicateParser(string str){
    Predicate res;
    size_t pos=0;

    if(str[0]=='~'){
        res.isNegated=1;
        str.erase(str.begin());
    }
    else{
        res.isNegated=0;
    }

    //remove ')'
    str.erase(str.end()-1);

    pos = str.find('(');
    string name = str.substr(0,pos);
    res.name = name;

    str.erase(0, pos+1);

    vector<string> v = splitter(str, ',');
    res.numberOfArguements = v.size();
    res.arguements = v;

    vector<char> argType(v.size());

    for(int i=0;i<v.size();i++)
        if('A'<=v[i][0] && 'Z'>=v[i][0])
            argType[i] = 'C';
        else
            argType[i] = 'V';

    res.arguementType = argType;
    return res;
}

Sentence standardize(Sentence sentence){
    int flag=0;

    for(int i=0;i<sentence.numberOfPredicates;i++)
        for(int j=0;j<sentence.predicate[i].numberOfArguements;j++)
            if(sentence.predicate[i].arguementType[j] == 'V'){
                flag=1;
                sentence.predicate[i].arguements[j].append(to_string(standardizeCount));
            }
    if(flag)
        standardizeCount++;
    return sentence;
}

Sentence sentenceParser(string str){
    str.erase(remove(str.begin(), str.end(), ' '), str.end());
    vector<string> v;

    v = splitter(str, '|');
    Sentence sentence;
    sentence.name = str;

    for(int i=0;i<v.size();i++){
        Predicate p = predicateParser(v[i]);
        sentence.predicate.push_back(p);
    }
    sentence.numberOfPredicates = v.size();
    return sentence;
}

void parseAndStandardize(string str){
    Sentence sentence = sentenceParser(str);
    sentence = standardize(sentence);
    KB.push_back(sentence);
}

string constructPredicate(Predicate predicate){
    string res1;
    if(predicate.isNegated)
        res1.append(1,'~');
    res1.append(predicate.name);
    res1.append(1,'(');
    for(int j=0;j<predicate.numberOfArguements;j++){
        res1.append(predicate.arguements[j]);
        if(j!=predicate.numberOfArguements-1)
            res1.append(1,',');
    }
    res1.append(1,')');
    return res1;
}

string constructQuery(Sentence query){

    string res;
    for(int i=0;i<query.numberOfPredicates;i++){
        string res1;
        res1 = constructPredicate(query.predicate[i]);
        if(i!=query.numberOfPredicates-1)
            res1.append(1,'|');
        res.append(res1);
    }
    return res;
}

Sentence handleDuplicates(Sentence sentence){
    unordered_map<size_t, int> duplicatesMap;
    hash<string> str_h;
    vector<Predicate> res2;
    for(int i=0;i<sentence.numberOfPredicates;i++){
        string ss = constructPredicate(sentence.predicate[i]);
        size_t hVal = str_h(ss);
        if(duplicatesMap.find(hVal) == duplicatesMap.end()){
            res2.push_back(sentence.predicate[i]);
            duplicatesMap[hVal] = 1;
        }
    }
    sentence.predicate = res2;
    sentence.numberOfPredicates = sentence.predicate.size();
    return sentence;
}

bool sortHelper(Predicate a, Predicate b){
    return a.name.compare(b.name)<0;
}

bool unification(Sentence kbSentence, Sentence querySentence, Predicate kb, int kbInd, Predicate query, int queryInd, Sentence *sentence, int *done){

    map<string, string> kbArgMap;
    map<string, string> queryArgMap;
    Sentence res;

    if(kb.numberOfArguements != query.numberOfArguements)
        return false;

    for(int i=0;i<kb.numberOfArguements;i++){
        if( (kb.arguementType[i] == 'C') &&
                (query.arguementType[i] == 'C') ){
            if(kb.arguements[i].compare(query.arguements[i]) != 0)
                return false;
        } 
        else if(kb.arguementType[i]=='C' && query.arguementType[i]=='V'){
            if( (queryArgMap.find(query.arguements[i]) != queryArgMap.end())  && 
                    (queryArgMap[query.arguements[i]].compare(kb.arguements[i]) != 0) )
                return false;
            queryArgMap[query.arguements[i]] = kb.arguements[i];
        }
        else if(kb.arguementType[i]=='V' && query.arguementType[i]=='C'){
            if( (kbArgMap.find(kb.arguements[i]) != kbArgMap.end())  && 
                    (kbArgMap[kb.arguements[i]].compare(query.arguements[i]) != 0) )
                return false;
            kbArgMap[kb.arguements[i]]=query.arguements[i];
        } else if(kb.arguementType[i]=='V' && query.arguementType[i]=='V'){
            if( (kbArgMap.find(kb.arguements[i]) != kbArgMap.end())  && 
                    (kbArgMap[kb.arguements[i]].compare(query.arguements[i]) != 0) )
                return false;
            kbArgMap[kb.arguements[i]] = query.arguements[i];
        }
    }

    for(int i=0;i<kbSentence.numberOfPredicates;i++){
        if(i==kbInd)
            continue;
        for(int j=0;j<kbSentence.predicate[i].numberOfArguements;j++){
            if(kbArgMap.find(kbSentence.predicate[i].arguements[j]) != kbArgMap.end()){
                kbSentence.predicate[i].arguements[j] = kbArgMap[kbSentence.predicate[i].arguements[j]];
                if(kbSentence.predicate[i].arguements[j][0]>='A' && kbSentence.predicate[i].arguements[j][0]<='Z')
                    kbSentence.predicate[i].arguementType[j] = 'C';
                else
                    kbSentence.predicate[i].arguementType[j] = 'V';

            }
        }
        res.predicate.push_back(kbSentence.predicate[i]);
    }

    for(int i=0;i<querySentence.numberOfPredicates;i++){
        if(i==queryInd)
            continue;
        for(int j=0;j<querySentence.predicate[i].numberOfArguements;j++){
            if(queryArgMap.find(querySentence.predicate[i].arguements[j]) != queryArgMap.end()){
                querySentence.predicate[i].arguements[j] = queryArgMap[querySentence.predicate[i].arguements[j]];
                if(querySentence.predicate[i].arguements[j][0]>='A' && querySentence.predicate[i].arguements[j][0]<='Z')
                    querySentence.predicate[i].arguementType[j] = 'C';
                else    
                    querySentence.predicate[i].arguementType[j] = 'V';
            }
        }
        res.predicate.push_back(querySentence.predicate[i]);
    }
    res.numberOfPredicates = res.predicate.size();
    if(res.numberOfPredicates==0){
        *done=1;
    }
    res = handleDuplicates(res);
    sort(res.predicate.begin(), res.predicate.end(), sortHelper);
    *sentence = res;
    return true;
}

vector<Sentence> unificationAndResolution(Sentence kbSentence, Sentence query, int *done){

    vector<Sentence> res;

    for(int i=0;i<query.numberOfPredicates;i++){
        for(int j=0;j<kbSentence.numberOfPredicates;j++){
            if( (query.predicate[i].name.compare(kbSentence.predicate[j].name) == 0) && (
                        (query.predicate[i].isNegated && !kbSentence.predicate[j].isNegated) || (
                            !query.predicate[i].isNegated && kbSentence.predicate[j].isNegated) ) ){

                Sentence sentence;
                if(unification(kbSentence, query, kbSentence.predicate[j], j,query.predicate[i],i, &sentence, done)){
                    if(*done)
                        return res;
                    res.push_back(sentence);
                }
            }
        }
    }
    return res;
}


bool trueOrFalse(string str, int beginTime){
    queue<Sentence> qu;
    int done=0;
    unordered_map<size_t, int> hMap;
    hash<string> str_hash;

    Sentence query = sentenceParser(str);

    // negate the initial query
    if(query.predicate[0].isNegated)
        query.predicate[0].isNegated=0;
    else
        query.predicate[0].isNegated=1;

    qu.push(query);
    KB.push_back(query);

    while(!qu.empty()){
        if((time(NULL)-beginTime)>60){
            cout<<"Time over"<<endl;
            KB.pop_back();
            return false;
        }
        query = qu.front();
        qu.pop();
        string constructedQuery = constructQuery(query);
        query.name = constructedQuery;

        size_t hVal = str_hash(query.name);

        if(hMap.find(hVal)==hMap.end()){
            hMap[hVal] = 1;

            for(int i=0;i<KB.size();i++){
                vector<Sentence> matches = unificationAndResolution(KB[i],query,&done);

                if(done){
                    KB.pop_back();
                    return true;
                }
                for(int i=0;i<matches.size();i++)
                    qu.push(matches[i]);
            }
        }
    }
    cout<<"I am exhausted"<<endl;
    KB.pop_back();
    return false;
}

int main(){
    ifstream fin("input.txt");

    string str;
    getline(fin, str);
    stringstream str1(str);
    str1 >> numberOfQueries;
    for(int i=0;i<numberOfQueries;i++){
        str.clear();
        getline(fin, str);
        queries.push_back(str);
    }

    str.clear();
    getline (fin,str);
    stringstream str2(str);
    str2 >> numberOfSentences;

    for(int i=0;i<numberOfSentences;i++){
        str.clear();
        getline(fin, str);
        parseAndStandardize(str);
    }

    ofstream fout("output.txt");
    for(int i=0;i<numberOfQueries;i++){
        int beginTime = time(NULL);
        if(trueOrFalse(queries[i], beginTime))
            fout<<"TRUE"<<endl;
        else
            fout<<"FALSE"<<endl;
        cout<<time(NULL)-beginTime<<endl;
    }
    return 0;
}
