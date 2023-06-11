int main()
{
    float x = 1.234;
    float y;
    int z = 4;
    int a;
    float p;
    int test=0;

    x = 2.0;
    a = (int)2.0;
    y = x;

    write("y=",y,'\n');

    z = (int)y;

    x = y;

    p = (float)3;

    // floating point addition
    p = 2.0 + 3.0;

    // floating point subtraction
    p = 2.5 - 3.5;

    // floating point multiplication
    p *= y;

    // floating point division
    p /= y;

    // integer times_equals
    z *= 2;

    // floating point LT
    if( x < 2.0 )
    {
        test = 1;
    };

    // floating point GT
    if( x > 2.0 )
    {
        test = 2;
    };

    if( z != 4 )
    {
        test = 10;
    };

    // floating point GTE
    if( x < 2.0 )
    {
        test = 3;
    };

    // floating point LTE
    if( x <= 2.0 )
    {
        test = 4;
    };

    // floating point equals
    if( x == 2.0 )
    {
        test = 5;
    };

    // floating point not equals
    if( x != 2.0 )
    {
        test = 6;
    };

    write("x=",x,'\n');
    write("y=",y,'\n');
    write("z=",z,'\n');
    write("a=", a,'\n');
    write("p=",p,'\n');
    write("test=",test,'\n');
}
