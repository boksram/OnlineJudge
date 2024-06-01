void Test1()
{
    bool ret = Solution().isPalindrome(121);
    if(ret)
    {
        std::cout<<"Pass test1 success"<<std::endl;
    }
    else
    {
        std::cout<<"Failed to pass test1"<<std::endl;
    }
}

void Test2()
{
    bool ret = Solution().isPalindrome(-10);
    if(!ret)
    {
        std::cout<<"Pass test2 success"<<std::endl;
    }
    else
    {
        std::cout<<"Failed to pass test2"<<std::endl;
    }
}

int main()
{
    Test1();
    Test2();
}
