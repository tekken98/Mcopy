#ifndef OPT_H
#define OPT_H
#include <memory>
#include <map>
#include <vector>
#include <string>
#include <unistd.h>
using std::string;
using std::map;
using std::shared_ptr;
using std::make_pair;
using std::vector;
class Opt{
    int argc;
    char ** const argv;
    shared_ptr<map<char,string>> m;
    vector<string> a;
    public:
    Opt(int c, char ** const v, const string& optstring) : argc(c), argv(v)
    {
        getOpt(optstring);
    };
    void getOpt(const string& optstring)
    {
        map<char,string>* ret = new map<char,string>();
        int opt;
        int len = optstring.length();
        while((opt = getopt(argc,argv,optstring.c_str())) != -1){
            for (int i = 0; i < len ;i++){
                if (opt == optstring[i]){
                    if (optstring[i+1] == ':'){
                        ret->insert(make_pair(static_cast<char>(opt),optarg));
                    }else{
                        ret->insert(make_pair(static_cast<char>(opt),""));
                    }
                    break;
                }
            }
        }
        m = shared_ptr<map<char,string>>(ret);
        for (int i = optind; i < argc ;i++){
            a.push_back(argv[i]);
        }
    }
    vector<string>::size_type argC(){
        return a.size();
    }
    string argV(unsigned int i){
        if (i < a.size())
            return a[i];
        else
            return "";
    }
    bool has(char c){
        return m->find(c) != m->end();
    }
    string value(char c){
        if (has(c))
            return m->find(c)->second;
        return "";
    }
};
#endif

