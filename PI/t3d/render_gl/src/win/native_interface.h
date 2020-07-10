#ifndef INCLUDE_NATIVE_INTERFACE_H
#define INCLUDE_NATIVE_INTERFACE_H

#include <pi_lib.h>
#include <gl_interface.h>

PI_BEGIN_DECLS

PiBool PI_API native_Self_IsLostContext(void);

void PI_API native_Self_Init(GLFunc *gl_context, GLCap *cap);

PI_END_DECLS

#endif /* INCLUDE_NATIVE_INTERFACE_H */