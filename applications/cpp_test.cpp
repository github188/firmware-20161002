#include <rtthread.h>

class A
{
public:
    A(int a) : value(a) {rt_kprintf("A::a=%d\n", a);}

    int getValue(void) {return value;}

private:
    int value;
};
A abcd(100);

class B
{
public:
    B(const char* str){rt_kprintf("B::str %s\n", str); value = 1000; }
    void ping(void) {rt_kprintf("B::ping ... %d\n", value);}

private:
    int value;
};
B bdef("test");

extern "C" {
int c_function(void)
{
    static A aa(10);
    B bb("B object");

    rt_kprintf("abcd::valule=%d\n", abcd.getValue());
    bdef.ping();

    return aa.getValue();
}

#include <finsh.h>
MSH_CMD_EXPORT(c_function, c++ function test);
}
