int main()
{
	extern string __sys__test__c;
    extern int __sys__test__b;
    extern float __sys__test__f;
    float pi = 3.1415926535;
    float x;

    x = __sys__test__f;
    write("x=",x,'\n');
    __sys__test__f *= pi;

    write( "/sys/test/f=", __sys__test__f, '\n' );

    write("/sys/test/b=",__sys__test__b,'\n');

    __sys__test__c.append('.');

    write( __sys__test__c, '\n');
}
