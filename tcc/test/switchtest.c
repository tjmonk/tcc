int main()
{
    int x = 3;
    string y;

    switch(x)
    {
        case 0:
            y = "zero\n";
            break;

        case 1:
            y = "one\n";
            break;

        case 3:
            y = "three\n";
            break;

        default:
            y = "unknown\n";
            break;

        case 2:
            y = "two\n";
            break;

    };

    write("the number is ", y);
}
