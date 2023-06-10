int main()
{
    string test;
    int fd;

    test = "This is another test\n";

    fd = file_open( "test.out", 'w' );
    file_write( fd, "This is a test", '\n' );
    file_write( fd, test );
    file_close( fd );
}
