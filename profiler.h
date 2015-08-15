
#include <ctime>

class TimeyWimey
{
public:
    TimeyWimey();
    TimeyWimey( const char *label );

	~TimeyWimey();

    double elapsed();
    void reset();

private:
    timespec beg_, end_;
	char *strLabel;
};
