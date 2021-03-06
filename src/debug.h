

#ifndef ML_DEBUG_H
#define ML_DEBUG_H

#include <stdint.h>
#include <stdio.h>
#include <stdarg.h>

#include "config.h"


#if DEBUG_ENABLE

#define debug printf
void debug_err(const char *fmt, ...);
#define debug_info printf
  
#define func_s() debug("\n%s \n", __func__)
#define func_e() debug("%s end \n\n", __func__)
#define func_ok() debug("%s ok \n\n", __func__)
#define func_fail() debug("%s failed \n\n", __func__)

#define show_func_line() debug("%s: %dL \n", __func__, __LINE__)

#else

#define debug(...) ;
#define debug_err(...) ;
#define debug_info ;

#define func_s() ;
#define func_e() ;
#define func_ok() ;
#define func_fail() ;
#define show_func_line() ;

#endif /* DEBUG_ENABLE */

#define fs() func_s()
#define fe() func_e()


#define show printf


#define out(type, x)				\
    {						\
	func_##type();				\
	return x;				\
    }

#define debug_suspend()				\
    {						\
	debug("suspending \n");			\
	while(1);				\
    }

  
#endif /* ML_DEBUG_H */


