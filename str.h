#ifndef STR_H
#define STR_H
#include <string>
#include <iostream>
using std::string;
using std::cout;
using std::endl;
class str : public string{
    typedef string::size_type size;
    const size npos = string::npos;
    public:
    class parameter{
        public:
        const string& target = "";
        const string& replace = "";
        size begin = 0;
    };
    public:
        str(){};
        str(const char * s){
            assign(s);
        }
        str(int n, char c):string(n,c){}
        str(const str& s){
            assign(s);
        }
        str& operator=(const str& a){
            assign(a);
            return *this;
        }
        str& operator=(str&& a){
            assign(std::move(a));
            return *this;
        }
        size replace(const parameter& pa){
            return replace(pa.target,pa.replace,pa.begin);
        }
        size replace(const string& s, const string& r,
                size begin = 0)
        {
            size found = find(s,begin);
            if (found != npos){
                cout << "found:" << found << endl;
                string::replace(found,s.size(),r);
                cout << *this << endl;
                return found + r.size();
            }
            return npos;
        }
        void replaceAll(const string& s, const string& r){
            size begin = 0;
            //cout << string(10,'*') << endl;
            while((begin = replace(s,r,begin)) != npos){
                ;
            }
        }
        operator const char* (){
            return this->c_str();
        }
};
#endif

