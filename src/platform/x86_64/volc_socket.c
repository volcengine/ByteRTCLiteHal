#include "volc_socket.h"

#include <errno.h>
#include <fcntl.h>
#include <stdio.h>
#include <unistd.h>

#include "volc_type.h"

int volc_socket (int __domain, int __type, int __protocol) {
    return socket(__domain, __type, __protocol);
}

int volc_bind (int __fd, struct sockaddr_in* __addr, int __len){
    return bind(__fd, (struct sockaddr*)__addr, (socklen_t)__len);
}

int volc_listen (int __fd, int __n) {
    return listen(__fd, __n);
}

int volc_accept (int __fd, struct sockaddr_in* __addr, int * __addr_len){
    return accept(__fd, (struct sockaddr*)__addr, (socklen_t*)__addr_len);
};

int volc_connect (int __fd, struct sockaddr_in* __addr, int __len) {
    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    int r = 0;
    do {
        errno = 0;
        r = connect(__fd, (struct sockaddr*)__addr, (socklen_t)__len);
    } while (r == -1 && errno == EINTR);

    if(r == -1 && errno != EINPROGRESS)  {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    }

    if(r == -1 && errno == EINPROGRESS) {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    }

    return ret_status; 
}

ssize_t volc_recv_msg (int __fd, void* data, size_t size,struct sockaddr_in * p_addr,uint32_t* p_status){
    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    struct msghdr msg = {0};
    msg.msg_flags = 0;
    struct iovec iov ={.iov_base = data,.iov_len = size};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    struct sockaddr_storage peer;
    if(p_addr != NULL){
        msg.msg_name = (struct sockaddr *)&peer;
        msg.msg_namelen = sizeof(peer);
    }

    int r = 0;
    do{
        r = recvmsg(__fd, &msg, 0);
    } while(r < 0 && errno == EINTR);
    if( r> 0) {
        ret_status = VOLC_STATUS_SUCCESS;
        if(p_addr != NULL) {
            *p_addr = *(struct sockaddr_in *)&peer;
        }
    }else if(errno == EAGAIN || errno == EWOULDBLOCK) {
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    } else if(r < 0) {
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    }
    if(p_status  != NULL) {
        *p_status = ret_status;
    };
    return r;
};

ssize_t volc_send_msg (int __fd, void* data, size_t size ,struct sockaddr_in * __addr, int __len,uint32_t *p_status){
    struct msghdr msg = {0};
    msg.msg_flags = 0;
    struct iovec iov ={.iov_base = data,.iov_len = size};
    msg.msg_iov = &iov;
    msg.msg_iovlen = 1;
    if(__addr != NULL) {
        msg.msg_name = (struct sockaddr *)__addr;
        msg.msg_namelen = (socklen_t)__len;
    }
    int r = 0;
    do{
        r =  sendto(__fd,data,size,0,__addr, (socklen_t)__len);
    }while(r < 0 && errno == EINTR);

    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    if(r > 0) {
        ret_status = VOLC_STATUS_SUCCESS;
    }else if(errno == EAGAIN || errno == EWOULDBLOCK) {
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    } else if(r < 0) {
        ret_status = VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    };

    if(p_status  != NULL) {
        *p_status = ret_status;
    };
CleanUp:
    return r;
}

int volc_close (int __fd){
    return close(__fd);
};

int volc_set_nonblocking(int __fd) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    int flags = fcntl(__fd, F_GETFL, 0);
    VOLC_CHK(flags != -1, VOLC_STATUS_INVALID_ARG);
    flags = fcntl(__fd, F_SETFL, flags | O_NONBLOCK);
    VOLC_CHK(flags != -1, VOLC_STATUS_INVALID_ARG);
err_out_label:
    return ret;
}

int volc_make_pipe(int fds[2]) {
    uint32_t ret = VOLC_STATUS_SUCCESS;
    int temp[2] = {0};
    VOLC_CHK(pipe(temp) == 0, VOLC_STATUS_INVALID_ARG);
    fds[0] = temp[0];
    fds[1] = temp[1];
    volc_set_nonblocking(fds[0]);
    volc_set_nonblocking(fds[1]);
err_out_label:  
    if(VOLC_STATUS_FAILED(ret)) {
        close(temp[0]);
        close(temp[1]);
    }
    return ret;
}

int volc_poll (struct pollfd *__fds, int __nfds, int __timeout) {
    return poll(__fds, __nfds, __timeout);
}

int volc_write (int __fd,void* data, size_t size) {
    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    int r = 0;
    do {
        errno = 0;
        r = write(__fd, data,size);
    } while (r == -1 && errno == EINTR);
    if(r == size) {
        ret_status = VOLC_STATUS_SUCCESS;
    }

    if(r == -1 && errno != EINPROGRESS)  {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_FAILED;
    }

    if(r == -1 && errno == EINPROGRESS) {
        ret_status =  VOLC_STATUS_EVLOOP_PERFORM_NEED_RETRY;
    }

    return ret_status; 
}

int volc_read (int __fd,void* data, size_t size) {
    int r = 0;
    do {
        errno = 0;
        r = read(__fd, data,size);
    } while (r == -1 && errno == EINTR);

    return r; 
}

int volc_sockopt_set(int __fd, int __level, int __optname,const void *__optval, int __optlen) {
   return setsockopt(__fd,__level,__optname,__optval, (socklen_t)__optlen);
}

int volc_sockopt_get(int __fd, int __level, int __optname,void * __optval, int * __optlen) {
    return getsockopt(__fd,__level,__optname,__optval, (socklen_t *)__optlen);
}