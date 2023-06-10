int main()
{
    int sig;
    int id;
    int count = 0;
    int renderCount = 0;
    int hdl;
    int ok;
    extern int __sys__test__b;
    extern int __sys__test__a;
    extern string __sys__test__c;
    int hSysTestB;
    int hSysTestC;
    int ERANGE = 34;
    char c;
    int x;
    int i;
    int fd;

    write("Running MODIFIED Notification test\n");

    hSysTestB = handle( __sys__test__b );
    hSysTestC = handle( __sys__test__c );

    // set up MODIFIED notification
    notify( __sys__test__b, NOTIFY_MODIFIED );
    notify( __sys__test__a, NOTIFY_CALC );

    // set up VALIDATE notification
    notify( __sys__test__b, NOTIFY_VALIDATE );
    notify( __sys__test__c, NOTIFY_VALIDATE );
    notify( __sys__test__c, NOTIFY_PRINT );

    // check that no-self-notification works for CALC
    write( "/SYS/TEST/A = ", __sys__test__a, '\n' );
    write( "/SYS/TEST/C = ", __sys__test__c, '\n' );

    write("waiting for signals\n");

    // run until we have received 10 timer 1 signals
    while( 1 )
    {
        // wait for a signal from one of the timers
        wait_sig( &sig, &id );

        switch( sig )
        {
            case SIG_VAR_MODIFIED:
                if( id == handle( __sys__test__b ) )
                {
                    write( "/SYS/TEST/B changed to ", __sys__test__b, '\n' );
                }
                else
                {
                    write("Var Modified: handle = ", id, '\n' );
                }
                break;

            case SIG_VAR_CALC:
                write("Received a calc notification\n");
                if( id == handle( __sys__test__a ) )
                {
                    write("Updating /sys/test/a\n");
                    __sys__test__a = count;
                }
                break;

            case SIG_VAR_PRINT:
                write("Printing /SYS/TEST/C\n");
                fd = open_print_session( id, &hdl );
                if( hdl == hSysTestC )
                {
                    file_write( fd, "Hello from the renderer: ", renderCount, '\n' );
                    renderCount++;
                }
                close_print_session( id, fd );
                break;

            case SIG_VAR_VALIDATE:
                write("Recevied a validate notification\n");
                hdl = validate_start( id );
                ok = ERANGE;
                switch( hdl )
                {
                    case hSysTestB:
                        if( __sys__test__b < 100 )
                        {
                            ok = 0;
                        }
                        break;

                    case hSysTestC:
                        ok = 0;
                        for(i=0;i<__sys__test__c.length();i++)
                        {
                            c = __sys__test__c.charAt(i);
                            if( c != 32 && ( c < 97 || c > 122 ) )
                            {
                                write("E: invalid character '", c, "' at index ", i, '\n' );
                                ok = ERANGE;
                            }
                        }
                        break;

                    default:
                        break;
                }
                validate_end( id, ok );
                break;


            default:
                write("received sig: ", sig, " id: ", id, '\n' );
                break;
        }

        count++;
        if( count >= 10 )
        {
            // we have received enough timer pulses
            // we can quit now
            break;
        }
    }
}
