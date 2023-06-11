int test()
{
    int i;

    for(i=0;i<10;i++)
    {
        write("Hi there\0\");
        writeLn();
    };
}

int main()
{
    test();
}

