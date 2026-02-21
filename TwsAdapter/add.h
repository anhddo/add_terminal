#ifndef MYAPI
#define MYAPI __declspec(dllexport)
#endif;
extern "C" MYAPI int Add(int a, int b);