int main()
{
	char c = '5';
	int foo = 1234;
	int i;
	int len;
	string str;
	string baa;
	string result;

	baa = "baa\n";
	write(baa);
	str.append(c, " test ", foo, '\n');
	write(str);
	write(str);
	result = baa;
	write(result);
	write("result1: ");
	result = str;
	write(result);
	write("result2: ");
	write(str);
	write("result3: ");
	write(baa);

	str = "Hello World!";
	write("before: string = '", str, "'\n" );
	len = str.length();

	// replace upper case characters in the string with lower case characters
	for(i=0;i<len;i++)
	{
		c = str.charAt(i);
		if( ( c >= 65 ) && ( c <= 90 ) )
		{
			c += 32;
			str.setAt(i,c);
		}
	}

	write("after: string = '", str, "'\n" );

}