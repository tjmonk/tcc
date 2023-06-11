int fact(int n)
{
    int a,b,c;

    if (n == 1) return(1);

    a = (n - 1);
    b = fact(a);
    c = n * b;

    return(c);
}

int main()
{
    int i;
    int n;

    write("factorial generation\0\");
    writeLn();
    writeLn();

    for(i=1;i<6;i++)
    {
        n = fact(i);
        write(i," factorial is \0\",n);
        writeLn();
    };
}



