#ifndef OPT_H
#define OPT_H
#include <memory>
#include <map>
#include <vector>
#include <unistd.h>
#include "str.h"
using std::map;
using std::shared_ptr;
using std::make_pair;
using std::vector;
class Opt{
    int argc;
    char ** const argv;
    bool invalid=false;
    shared_ptr<map<char,str>> m = shared_ptr<map<char,str>>( new map<char,str>());
    vector<str> a;
    public:
    Opt(int c, char ** const v): argc(c),argv(v){};
    Opt(int c, char ** const v, const str& optstr) : argc(c), argv(v)
    {
        parse(optstr);
    };
    void parse(const str& optstr)
    {
        int opt;
        int len = optstr.length();
        while((opt = getopt(argc,argv,optstr.c_str())) != -1){
            int i;
            for (i = 0; i < len ;i++){
                if (opt == optstr[i]){
                    if (optstr[i+1] == ':'){
                        (*m)[opt] = optarg;
                       // m->insert(make_pair(static_cast<char>(opt),optarg));
                    }else{
                        (*m)[opt] = "";
                        //m->insert(make_pair(static_cast<char>(opt),""));
                    }
                    break;
                }
                if (opt == '?'){
                    invalid=true;
                }
            }
        }
        for (int i = optind; i < argc ;i++){
            a.push_back(argv[i]);
        }
    }
    vector<str>::size_type argC(){
        return a.size();
    }
    str argV(unsigned int i){
        if (i < a.size())
            return a[i];
        else
            return "";
    }
    bool operator[] (char c){
        return has(c);
    }
    str operator - (char c){
        return value(c);
    }
    bool has(char c){
        return m->find(c) != m->end();
    }
    bool valid(){
        return !invalid;
    }
    str value(char c){
        if (has(c))
            return m->find(c)->second;
        return "";
    }
    void setOpt(char c , const str& s){
        m->insert(make_pair(c,s));
    }
    str getOpt(char c){
        return value(c);
    }
};
#endif

