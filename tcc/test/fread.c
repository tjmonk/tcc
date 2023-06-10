int main()
{
    string inputFile;
    int i;
    int fd;
    int n;

    inputFile = "/dev/urandom";

    // open file for binary reading
    fd = file_open( inputFile, 'R' );
    for(i=0;i<10;i++)
    {
        file_read( fd, n );
        write( n, '\n' );
    }

    file_close( fd );
}
