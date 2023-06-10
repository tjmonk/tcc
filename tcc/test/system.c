int main()
{
    string exec;
    int rc;

    write("Running SYSTEM test\n");

    rc = system( "/bin/date" );
    write("rc= ", rc, '\n' );
    exec = "/usr/bin/uptime";
    rc = system( exec );
    write("rc= ", rc, '\n' );
    rc = system( "no command" );
    write("rc= ", rc, '\n' );
}
