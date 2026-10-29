#pragma once
#include <string>
#include <cstdint>
constexpr int HEX=16;
class String:public std::string {public:using std::string::string;String(const std::string&s):std::string(s){}String(size_t n):std::string(std::to_string(n)){}String(int n):std::string(std::to_string(n)){}String(unsigned char n,int base):std::string(base==16?hex(n):std::to_string(n)){}static std::string hex(unsigned char n){const char*d="0123456789abcdef";return n<16?std::string(1,d[n]):std::string(1,d[n>>4])+d[n&15];}String(size_t n,char c):std::string(n,c){}String substring(size_t a,size_t b)const{return substr(a,b-a);}void remove(size_t p){erase(p);} };
