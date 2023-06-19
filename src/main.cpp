#include "main.h"

using namespace std;

int64_t micros()
{
    FILETIME ft_now;
    GetSystemTimeAsFileTime(&ft_now);
    return ((LONGLONG)ft_now.dwLowDateTime + ((LONGLONG)(ft_now.dwHighDateTime) << 32LL)) / 10;
}

int64_t t_start = 0;

void run(uint16_t breakpoint = 0)
{
    while (pc != breakpoint)
    {
        exec_ins();
        if (micros() - t_start > 16666)
        {
            irq();
            t_start = micros();
        }
    }
}

void run_debug()
{
    while (1)
    {
        uint16_t rounded = pc - pc % 16;
        cout << setfill('0') << setw(4) << rounded << "\t"
             << "a=" << setfill('0') << setw(2) << (int)(uint8_t)a
             << " x=" << setfill('0') << setw(2) << (int)(uint8_t)x
             << " y=" << setfill('0') << setw(2) << (int)(uint8_t)y
             << " fZ=" << (int)fZ
             << " fN=" << (int)fN
             << " fC=" << (int)fC
             << " fV=" << (int)fV;

        cout << " stack(" << setfill('0') << setw(2) << (int)(uint8_t)stack_pointer << "): ";
        for (int i = stack_pointer + 1; i <= 0xff && i <= stack_pointer + 10; i++)
        {
            cout << setfill('0') << setw(2) << (int)(uint8_t)mem_read(0x100 + i) << " ";
        }
        cout << endl;

        for (int i = 0; i < 20; i++)
        {
            if (rounded + i > 0xffff)
                cout << "--";
            else
                cout << setfill('0') << setw(2) << (int)(uint8_t)mem_read(rounded + i);
            cout << " ";
        }
        cout << endl;
        for (int i = 0; i < pc % 16; i++)
            cout << "   ";
        cout << "^"
             << " (" << setfill('0') << setw(4) << pc << ")" << endl;
        getchar();
        exec_ins();
    }
}

int main(int argc, char *argv[])
{
    cout << hex;

    t_start = micros();

    while (1)
    {
        reset();
        pc = 0xc000;
        int64_t start = micros();
        run(0xfce2);
        int32_t duration = micros() - start;
        printf("%f\n", duration / 1000000.0);
    }
    run_debug();

    return 0;
}
