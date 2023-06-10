int main()
{
    int sig;
    int id;
    int count = 0;
    int done = 0;

    // set up timer 1 to fire every 1.5 seconds
    set_timer( 1, 1500 );

    // set up timer 2 to fire every 2 seconds
    set_timer( 2, 2000 );

    // run until we have received 10 timer 1 signals
    while( !done )
    {
        // wait for a signal from one of the timers
        wait_sig( &sig, &id );
        if( sig == SIG_TIMER )
        {
            // check which timer expired
            switch( id )
            {
                case 1:
                    // Timer 1
                    write("Timer 1 expired\n");
                    count++;
                    if( count >= 4 )
                    {
                        // we have received enough timer pulses
                        // we can quit now
                        done = 1;
                    }
                    break;

                case 2:
                    // Timer 2
                    write("Timer 2 expired\n");
                    break;

                default:
                    break;
            }
        }
    }
}
