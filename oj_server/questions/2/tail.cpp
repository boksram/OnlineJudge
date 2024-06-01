#include <iostream>
#include <vector>
#include <limits.h>

using namespace  std;

class Solution
{
    public:
        int Max(const vector<int> &v)
        {
            int ret =INT_MIN;
            for(const int &i:v)
            {
                ret=i>ret?i:ret;
            }
            return ret;
        }
};

void Test1()
{
    vector<int> v={1,2,3,4,5,6};
    int max = Solution().Max(v);
    if(max==6)
    {
        cout<<"Pass test1 success"<<endl;
    }
    else
    {
        cout<<"Failed to pass test2"<<endl;
    }
}

void Test2()
{
    vector<int> v={-1,-2,-3,-4,-5,-6};
    int max = Solution().Max(v);
    if(max==-1)
    {
        cout<<"Pass test2 success"<<endl;
    }
    else
    {
        cout<<"Failed to pass test2"<<endl;
    }
}

int main()
{
    Test1();
    Test2();
    return 0;
}
