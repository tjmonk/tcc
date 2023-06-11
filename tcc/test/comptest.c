int main()
{
    float x;
    float y;

    x = 2.0;
    y = 4.0;

    if( x < y )
    {
        write(x, " is less than ", y);
        writeLn();
    };

    if( x > y )
    {
        write(x, " is greater than ", y);
        writeLn();
    };

    if( x <= y )
    {
        write(x, " is less than or equal to ", y);
        writeLn();
    };

    if( x >= y )
    {
        write(x, " is greater than or equal to ", y);
        writeLn();
    };

    if( x == y )
    {
        write(x, " is equal to ", y);
        writeLn();
    };

    if( x != y )
    {
        write(x, " is not equal to ", y);
        writeLn();
    };
}