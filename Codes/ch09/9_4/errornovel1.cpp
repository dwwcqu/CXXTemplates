#include <string>
#include <map>
#include <algorithm>

int main()
{
    std::map<std::string, double> coll;

    // 在 coll 中寻找第一个非空的字符串
    auto pos = std::find_if(coll.begin(), coll.end(),
     [](std::string const& s){
        return s != "";
     });
}