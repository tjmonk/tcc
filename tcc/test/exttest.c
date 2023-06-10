int main()
{
	extern int __sys__test__a;
    extern int __sys__test__b;
    extern float __sys__test__f;
    extern int __sys__test__delay;
    int dly = 0;
    int count = 0;
    int x;
    int i;

    while( ++count <= 5 )
    {
        write("count=",count,'\n');
        write("This is a test\n");
        if( dly != __sys__test__delay )
        {
            dly = __sys__test__delay;
            write("delay=",dly,'\n');
        };

        write("This is another test\n");
        write("delay=",dly,'\n');
        write("/sys/test/delay=",__sys__test__delay,'\n');

        x = __sys__test__a;
        write("x = ",x,'\n');
        write("/SYS/TEST/A=",__sys__test__a,'\n');

        __sys__test__a++;
        write("/SYS/TEST/A=",__sys__test__a,'\n');

        write("/SYS/TEST/B=",__sys__test__b,'\n');

        __sys__test__b++;
        write("/SYS/TEST/B=",__sys__test__b,'\n');

        write("/SYS/TEST/F=",__sys__test__f,'\n');

        __sys__test__f = __sys__test__f + 1.0;
        write("/SYS/TEST/F=",__sys__test__f,'\n');

        delay( dly );
    };
}