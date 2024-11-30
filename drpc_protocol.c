#include "drpc_protocol.h"
#include "drpc_struct.h"
#include "drpc_types.h"

#include <assert.h>
#include <stdlib.h>

struct d_struct* drpc_call_to_massage(struct drpc_call* call){
    struct d_struct* massage = new_d_struct();

    size_t arguments_buflen = drpc_types_buflen(call->arguments,call->arguments_len);
    char* arguments_buf = malloc(arguments_buflen); assert(arguments_buf);
    drpc_types_buf(call->arguments,call->arguments_len,arguments_buf);
    drpc_types_free(call->arguments,call->arguments_len);

    d_struct_set(massage,"packed_arguments",arguments_buf,d_sizedbuf,arguments_buflen);
    free(arguments_buf);

    d_struct_set(massage,"fn_name", call->fn_name, d_str);


    free(call->fn_name);
    free(call);

    return massage;
}

struct drpc_call* massage_to_drpc_call(struct d_struct* massage){
    struct drpc_call* call = calloc(1,sizeof(*call));

    size_t unused = 0;
    char* packed_arguments = NULL;
    d_struct_get(massage,"packed_arguments",&packed_arguments,d_sizedbuf,&unused);

    size_t unpacked_len = 0;
    call->arguments = buf_drpc_types(packed_arguments,&unpacked_len);
    call->arguments_len = (uint8_t)unpacked_len;

    d_struct_get(massage,"fn_name",&call->fn_name,d_str);

    d_struct_free(massage);

    return call;
}

struct d_struct* drpc_return_to_massage(struct drpc_return* drpc_return){
    struct d_struct* massage = new_d_struct();

    size_t arguments_buflen = drpc_types_buflen(drpc_return->updated_arguments,drpc_return->updated_arguments_len);
    char* arguments_buf = malloc(arguments_buflen); assert(arguments_buf);
    drpc_types_buf(drpc_return->updated_arguments,drpc_return->updated_arguments_len,arguments_buf);

    d_struct_set(massage,"updated_arguments",arguments_buf,d_sizedbuf,arguments_buflen);
    drpc_types_free(drpc_return->updated_arguments,drpc_return->updated_arguments_len);
    free(arguments_buf);

    size_t returned_buflen = drpc_buflen(&drpc_return->returned);
    char* returned_buf = malloc(returned_buflen);
    drpc_buf(&drpc_return->returned,returned_buf);

    d_struct_set(massage,"return",returned_buf,d_sizedbuf,returned_buflen);
    free(returned_buf);
    drpc_type_free(&drpc_return->returned);

    return massage;
}

struct drpc_return* massage_to_drpc_return(struct d_struct* massage){
    struct drpc_return* drpc_return = calloc(1,sizeof(*drpc_return));

    size_t unused = 0;
    char* updated_arguments_buf = NULL;
    d_struct_get(massage,"updated_arguments",&updated_arguments_buf,d_sizedbuf,&unused);

    size_t updated_arguments_len = 0;
    drpc_return->updated_arguments = buf_drpc_types(updated_arguments_buf,&updated_arguments_len);
    drpc_return->updated_arguments_len = (uint8_t)updated_arguments_len;

    char* returned_buf = NULL;
    d_struct_get(massage,"return",returned_buf,d_sizedbuf,&unused);

    buf_drpc(&drpc_return->returned,returned_buf);

    d_struct_free(massage);
    return drpc_return;
}
