/*
 * $Id: H:/drh/idioms/book/RCS/except.doc,v 1.10 1997/02/21 19:43:55 drh Exp $
 */
#ifndef EXCEPT_INCLUDED
#define EXCEPT_INCLUDED

#include <setjmp.h>

typedef struct except_t {
	char *reason;
} except_t;

typedef struct except_frame except_frame;

struct except_frame {
	except_frame *prev;
	jmp_buf env;
	const char *file;
	int line;
	const except_t *exception;
};

enum {
	except_entered = 0,
	except_raised,
	except_handled,
	except_finalized
};

extern except_frame *except_stack;
extern const         except_t Assert_Failed;
void                 except_raise(const except_t *e, const char *file, int line);

#define RAISE(e) except_raise(&(e), __FILE__, __LINE__)
#define RERAISE  except_raise(except_frame.exception, except_frame.file, except_frame.line)
#define RETURN   switch (except_stack = except_stack->prev,0) default: return
#define TRY do { \
	volatile int except_flag; \
	except_frame except_frame; \
	except_frame.prev = except_stack; \
	except_stack = &except_frame;  \
	except_flag = setjmp(except_frame.env); \
	if (except_flag == except_entered) {
#define EXCEPT(e) \
		if (except_flag == except_entered) except_stack = except_stack->prev; \
	} else if (except_frame.exception == &(e)) { \
		except_flag = except_handled;
#define ELSE \
		if (except_flag == except_entered) except_stack = except_stack->prev; \
	} else { \
		except_flag = except_handled;
#define FINALLY \
		if (except_flag == except_entered) except_stack = except_stack->prev; \
	} { \
		if (except_flag == except_entered) \
			except_flag = except_finalized;
#define END_TRY \
		if (except_flag == except_entered) except_stack = except_stack->prev; \
		} if (except_flag == except_raised) RERAISE; \
} while (0)

#endif
