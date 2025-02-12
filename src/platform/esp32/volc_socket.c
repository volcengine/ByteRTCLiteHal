/*
 * Copyright (2025) Beijing Volcano Engine Technology Ltd.
 * SPDX-License-Identifier: MIT
 */

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
    int times = 0;
    do {
        r = recvmsg(__fd, &msg, 0);
        times++;
        if (times % 5 == 0) {
            usleep(0);
        }
    } while(r < 0 && errno == EINTR && times < 30);
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
    int times = 0;
    do {
        r =  sendto(__fd,data,size,0,__addr,__len);
        times++;
        if (times % 5 == 0) {
            usleep(0);
        }
    } while(r < 0 && errno == EINTR && times < 30);

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
    fds[0] = -1;
    fds[1] = -1;
    int err = 0,opt = 0,write_fd = -1,read_fd = -1;
    read_fd =  volc_socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    VOLC_CHK(read_fd > 0,VOLC_STATUS_INVALID_ARG);

    struct sockaddr_in servaddr;
    uint32_t addr_len = sizeof(servaddr);
    memset(&servaddr, 0, sizeof(servaddr));
    servaddr.sin_family = AF_INET;
    servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);
    servaddr.sin_port = 0;

    // int bind_ret = bind(read_fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    int bind_ret = volc_bind(read_fd, (struct sockaddr*)&servaddr, sizeof(servaddr));
    VOLC_CHK(bind_ret >= 0,VOLC_STATUS_SOCKET_BIND_FAILED);

    // get port bind by system
    int getsockname_ret = getsockname(read_fd, (struct sockaddr*)&servaddr, &addr_len);
    VOLC_CHK(getsockname_ret >= 0,VOLC_STATUS_SOCKET_GETSOCKETNAME_FAILED);

    if(servaddr.sin_addr.s_addr == htonl(INADDR_ANY))
        servaddr.sin_addr.s_addr = htonl(INADDR_LOOPBACK);

    write_fd = volc_socket(AF_INET,SOCK_DGRAM,IPPROTO_UDP);
    int conn_ret = volc_connect(write_fd, (struct sockaddr*)&servaddr, addr_len);
    VOLC_CHK(conn_ret >= 0,VOLC_STATUS_INVALID_ARG);

    volc_set_nonblocking(write_fd);
    volc_set_nonblocking(read_fd);
    fds[0] = read_fd;
    fds[1] = write_fd;
err_out_label:  
    if(VOLC_STATUS_FAILED(ret)) {
        if (read_fd > 0) {
            close(read_fd);
        }
        if (write_fd > 0) {
            close(write_fd);
        }
    }
    return ret;
}

int volc_poll (struct pollfd *__fds, int __nfds, int __timeout) {
    return poll(__fds, __nfds, __timeout);
}

int volc_write (int __fd,void* data, size_t size) {
    uint32_t ret_status = VOLC_STATUS_SUCCESS;
    int r = 0;
    int times = 0;
    do {
        errno = 0;
        r = write(__fd, data,size);
        times++;
        if (times % 5 == 0) {
            usleep(0);
        }
    } while (r == -1 && errno == EINTR && times < 30);
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
    int times = 0;
    do {
        errno = 0;
        r = read(__fd, data,size);
        times++;
        if (times % 5 == 0) {
            usleep(0);
        }
    } while (r == -1 && errno == EINTR && times < 30);

    return r; 
}

int volc_sockopt_set(int __fd, int __level, int __optname,const void *__optval, int __optlen) {
   return setsockopt(__fd,__level,__optname,__optval, (socklen_t)__optlen);
}

int volc_sockopt_get(int __fd, int __level, int __optname,void * __optval, int * __optlen) {
    return getsockopt(__fd,__level,__optname,__optval, (socklen_t *)__optlen);
}