int inc10( int val )
{
	return ( val + 10 );
}

int main()
{
	extern string __sys__test__c;
	extern int __sys__test__a;
	extern int __sys__test__b;
    extern float __sys__test__f;
	float result;
	float scale = 2.5;
	float offset = 1.3;
	int x;
	int y;

	__sys__test__a = 10;

	__sys__test__b = 200;

	__sys__test__f = 5.4;


	x = __sys__test__b++;
	write("TEST1:/sys/test/b = ", __sys__test__b,'\n');
	write("TEST2:x=",x,'\n');

	y = ++__sys__test__b;
	write("TEST3:y=",y,'\n');

	__sys__test__c = "testing one two three...\n";

	write("TEST4:/sys/test/c = ", __sys__test__c );

	write("TEST5:/sys/test/b = ", __sys__test__b ,'\n');
	write("TEST6:/SYS/TEST/A = ", __sys__test__a ,'\n');
	write("TEST7:/SYS/TEST/F = ",__sys__test__f,'\n');
	__sys__test__b = 3;
	write("TEST8:/SYS/TEST/B = ",__sys__test__b,'\n');
	x = __sys__test__b;
    write("TEST9:x=",x,'\n');

	x = __sys__test__b + __sys__test__a;
	write("TEST10:x=",x,'\n');

	__sys__test__a = x;
	write("TEST11:/sys/test/a =", __sys__test__a,'\n');
   	__sys__test__a += 4;
	write("TEST12:/sys/test/a =", __sys__test__a,'\n');


	__sys__test__a = __sys__test__b;
	write("TEST13:/sys/test/a = ", __sys__test__a,'\n');

	write("decrementing...\n");
	write("TEST14:/sys/test/a = ",__sys__test__a,'\n');
	x = __sys__test__a--;
	write("TEST15:/sys/test/a = ",__sys__test__a,'\n');
	write("TEST16:x=",x,'\n');
	y = --__sys__test__a;
	write("TEST17:/SYS/TEST/A = ", __sys__test__a,'\n');
	write("TEST18:y=",y,'\n');

	for( __sys__test__a = 10; __sys__test__a > 0; __sys__test__a = __sys__test__a - 1 )
	{
		write("TEST19:/SYS/TEST/A = ", __sys__test__a, '\n');
		result = (scale * (float)__sys__test__a) + offset;
		write("TEST20:result=",result,'\n');
	};

	__sys__test__a = 10;
	__sys__test__a = inc10( __sys__test__a );
	write("TEST21:/SYS/TEST/A = ",__sys__test__a,'\n');
}
