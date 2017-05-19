#ifndef _DOUBLEFETCH_H_
#define _DOUBLEFETCH_H_


#define _XBEGIN_STARTED		(~0u)
#define _XABORT_EXPLICIT	(1 << 0)
#define _XABORT_RETRY		(1 << 1)
#define _XABORT_CONFLICT	(1 << 2)
#define _XABORT_CAPACITY	(1 << 3)
#define _XABORT_DEBUG		(1 << 4)
#define _XABORT_NESTED		(1 << 5)
#define _XABORT_CODE(x)		(((x) >> 24) & 0xFF)


__attribute__((always_inline)) inline unsigned int xbegin(void)
{
  unsigned status;
  asm volatile ("xbegin 1f \n 1:" : "=a"(status) : "a"(-1UL) );
  return status;
}
__attribute__((always_inline)) inline void xend(void)
{
  asm volatile ("xend");
}



typedef struct {
    int retries;
    int retry;
} doublefetch_t;

doublefetch_t doublefetch_init(int retries) {
    doublefetch_t config = { .retries = retries, .retry = 0 };
    return config;
}

#define doublefetch_start(conf) do {\
    conf.retry = 0; \
    doublefetch_retry:\
    if (xbegin() == _XBEGIN_STARTED) {

#define doublefetch_end(conf, fail) \
         xend ();\
        } else { \
            conf.retry++;\
            if(conf.retry < conf.retries) {\
                goto doublefetch_retry;\
            } else {\
                fail\
            }\
        }\
    } while(0)
 
#endif
