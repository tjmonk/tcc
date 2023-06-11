int FindPrimes()
{
    int i,j;
    int x;
    int count;
    int p[99];

    write("Initialising Prime Array\n");

    for(i=0;i<=99;i++) p[i] = 1;

    write("Calculating primes\n");

    for(i=2;i<=99;i++)
    {
        for(j=(i*2);j<=99;j+=i)
        {
            p[j] = 0;
        };
    };

    write("Prime Numbers\n");

    for(i=2;i<=99;i++)
    {
        if (p[i] == 1)
        {
            count++;
            write(i);
            writeLn();
        };
    };

    return(count);
}

int main()
{
    int num;

    write("Prime Number generator\n");

    num = FindPrimes();

    write("There are ",num," primes less than 100\n");
}








