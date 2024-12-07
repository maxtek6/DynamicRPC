#include "drpc_protocol.h"
#include "drpc_struct.h"
#include "drpc_types.h"

#include <netinet/in.h>
#include <arpa/inet.h>
#include <assert.h>
#include <stdint.h>
#include <stdlib.h>
#include <string.h>
void drpc_call_free(struct drpc_call* call){
    free(call->fn_name);

    drpc_types_free(call->arguments,call->arguments_len);

}
void drpc_return_free(struct drpc_return* ret){
    drpc_type_free(&ret->returned);
    drpc_types_free(ret->updated_arguments,ret->updated_arguments_len);
}

struct d_struct* drpc_call_to_message(struct drpc_call* call){
    struct d_struct* message = new_d_struct();

    size_t arguments_buflen = drpc_types_buflen(call->arguments,call->arguments_len);
    char* arguments_buf = malloc(arguments_buflen); assert(arguments_buf);
    drpc_types_buf(call->arguments,call->arguments_len,arguments_buf);

    d_struct_set(message,"packed_arguments",arguments_buf,d_sizedbuf,arguments_buflen);
    free(arguments_buf);

    d_struct_set(message,"fn_name", call->fn_name, d_str);

    return message;
}

struct drpc_call* message_to_drpc_call(struct d_struct* message){
    struct drpc_call* call = calloc(1,sizeof(*call));

    size_t unused = 0;
    char* packed_arguments = NULL;
    if(d_struct_get(message,"packed_arguments",&packed_arguments,d_sizedbuf,&unused) != 0){
        free(call);
        return NULL;
    }

    size_t unpacked_len = 0;
    call->arguments = buf_drpc_types(packed_arguments,&unpacked_len);
    call->arguments_len = (uint8_t)unpacked_len;

    if(d_struct_get(message,"fn_name",&call->fn_name,d_str) != 0){
        drpc_types_free(call->arguments,call->arguments_len);
        free(call);
        return NULL;
    }
    d_struct_unlink(message,"fn_name",d_str);
    return call;
}

struct d_struct* drpc_return_to_message(struct drpc_return* drpc_return){
    struct d_struct* message = new_d_struct();

    size_t arguments_buflen = drpc_types_buflen(drpc_return->updated_arguments,drpc_return->updated_arguments_len);
    char* arguments_buf = malloc(arguments_buflen); assert(arguments_buf);
    drpc_types_buf(drpc_return->updated_arguments,drpc_return->updated_arguments_len,arguments_buf);

    d_struct_set(message,"updated_arguments",arguments_buf,d_sizedbuf,arguments_buflen);
    free(arguments_buf);

    size_t returned_buflen = drpc_buflen(&drpc_return->returned);
    char* returned_buf = malloc(returned_buflen);
    drpc_buf(&drpc_return->returned,returned_buf);

    d_struct_set(message,"return",returned_buf,d_sizedbuf,returned_buflen);
    free(returned_buf);


    return message;
}

struct drpc_return* message_to_drpc_return(struct d_struct* message){
    struct drpc_return* drpc_return = calloc(1,sizeof(*drpc_return));

    size_t unused = 0;
    char* updated_arguments_buf = NULL;
    if(d_struct_get(message,"updated_arguments",&updated_arguments_buf,d_sizedbuf,&unused) != 0){
        free(drpc_return);
        return NULL;
    }

    size_t updated_arguments_len = 0;
    drpc_return->updated_arguments = buf_drpc_types(updated_arguments_buf,&updated_arguments_len);
    drpc_return->updated_arguments_len = (uint8_t)updated_arguments_len;

    char* returned_buf = NULL;
    if(d_struct_get(message,"return",&returned_buf,d_sizedbuf,&unused) != 0){
        drpc_types_free(drpc_return->updated_arguments,drpc_return->updated_arguments_len);
        free(drpc_return);
        return NULL;
    }

    buf_drpc(&drpc_return->returned,returned_buf);
    return drpc_return;
}



//Code below was EDITED by chatGPT

int drpc_send_message(struct drpc_message* msg, int fd) {
    struct d_struct* message = new_d_struct();

    uint8_t type = msg->message_type;
    d_struct_set(message, "msg_type", &type, d_uint8);

    if (msg->message) {
        d_struct_set(message, "msg", msg->message, d_struct);
    }

    size_t message_len = 0;
    char* send_buf = d_struct_buf(message, &message_len);
    uint64_t send_len = message_len;

    d_struct_unlink(message, "msg", d_struct);

    // Send the length of the message
    if (send(fd, &send_len, sizeof(uint64_t), MSG_NOSIGNAL) != sizeof(uint64_t)) {
        d_struct_free(message);
        free(send_buf);
        return 1;  // Error sending length
    }

    // Send the message in chunks
    size_t total_sent = 0;
    while (total_sent < message_len) {
        ssize_t bytes_sent = send(fd, send_buf + total_sent, message_len - total_sent, MSG_NOSIGNAL);
        if (bytes_sent <= 0) {
            d_struct_free(message);
            free(send_buf);
            return 1;  // Error sending message
        }
        total_sent += bytes_sent;
    }

    d_struct_free(message);
    free(send_buf);
    return 0;  // Success
}

int drpc_recv_message(struct drpc_message* msg, int fd) {
    uint64_t len64 = 0;

    // Receive the length of the incoming message
    if (recv(fd, &len64, sizeof(uint64_t), MSG_NOSIGNAL) != sizeof(uint64_t)) {
        return 1;  // Error receiving length
    }

    // Allocate buffer for the incoming message
    char* buf = malloc(len64);assert(buf);

    // Receive the message in chunks
    size_t total_received = 0;
    while (total_received < len64) {
        ssize_t bytes_received = recv(fd, buf + total_received, len64 - total_received, MSG_NOSIGNAL);
        if (bytes_received <= 0) {
            free(buf);
            return 1;  // Error receiving message
        }
        total_received += bytes_received;
    }

    struct d_struct* container = new_d_struct();
    buf_d_struct(buf, container);
    free(buf);

    uint8_t utype = 0;

    d_struct_get(container, "msg_type", &utype, d_uint8);
    msg->message_type = utype;

    if (d_struct_get(container, "msg", &msg->message, d_struct) == 0) {
        d_struct_unlink(container, "msg", d_struct);
    }

    d_struct_free(container);
    return 0;  // Success
}

