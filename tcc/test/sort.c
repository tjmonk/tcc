int main()
{
    int list[100];
    int numbers;
    int num;
    bool sorted;
    int i;
	int j;

	write("How many numbers do u wish to sort? ");
	read(numbers);

	if (numbers < 0)
	{
    	write("I cant sort a negative number of values!\n");
    	return(0);
	}
	else if (numbers > 100)
	{
    	write("too large   max 100!\n");
    	return(0);
	}
	else if (numbers == 0)
	{
		write("what is the point of sorting nothing at all?\n");
		return(0);
	};

    write("Enter ", numbers, " values to be sorted:\n");

    for(i=0;i<numbers;i++)
    {
        read(num);
        list[i] = num;
    };

	write("Sorting...\n");

    for(;sorted == 0;)
    {
        sorted = 1;

        for(i=0 ; i < (numbers - 1) ; i++)
    	{
			j = i+1;

        	if ( list[i] > list[j] )
    	    {
        	    sorted = 0;
				num = list[i];
        	    list[i] = list[j];
        	    list[j] = num;
    	    };
    	};
    };

	write("sorted values:\n");

	for(i=0;i<numbers;i++)
    {
	    num = list[i];
	    write(num, '\n');
    };
}








