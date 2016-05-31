#include <stdint.h>

#define PM_WAKE_16MS  0
#define PM_WAKE_32MS  1
#define PM_WAKE_64MS  2
#define PM_WAKE_128MS 3
#define PM_WAKE_250MS 4
#define PM_WAKE_500MS 5
#define PM_WAKE_1S    6
#define PM_WAKE_2S    7
#define PM_WAKE_4S    8
#define PM_WAKE_8S    9

class PowerManager
{
  public:
    void setup(uint8_t waketime);
    void sleep(uint8_t cycles);
    bool sleep(void);
};

