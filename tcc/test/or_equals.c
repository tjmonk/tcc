int main()
{
    int x;
    int y = 0x01;

    x = 0x20;

    x |= 0x06;

    write( "x=",x,'\n');

    x |= y;

    write( "x=",x,'\n');
    write( "y=",y,'\n');
}