// string buffer testing
// here is a comment

//int funcb()
//{
//    string blah;
//
//    blah = "a different blah\n";
//
//    write(blah);

//    return ( 0 );
//}

string func_a(string foo)
{
    string blah;

    write("foo:",foo,'\n');

    blah.append('x','y','z');

    write("blah:",blah,'\n');

    blah.append(foo);
    write("blah:",blah,'\n');
//    write("calling func2():\n");
//    funcb();
//    write("returned from func2()\n");

//    write("blah",blah,'\n');
    return ( blah );
}


int main()
{
    int i;
    char x = 'A';
    int foo = 1234;
    string src;
    string dst;

    src.append(x, " test ", foo, '\n');
    write(src);
    dst.append( "one ", "two ", "three ", src );
    write(dst);

//    for(i=0;i<10;i++)
//    {
//        dst.append( i, ':', src );
//    };

    write("calling func_a():\n");
    dst = func_a(src);
    write("returned from func_a()\n");
    write(dst);
}
