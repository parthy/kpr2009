/*
 * (c) 2009 Technische Universität Dresden
 * This file is part of TUD:OS and distributed under the terms of the
 * GNU Lesser General Public License 2.1.
 * Please see the COPYING-LGPL-2.1 file for details.
 */
#include <sys/socket.h>
#include <errno.h>
#include <stdio.h>
#include <netdb.h>
#include <unistd.h>

int socket(int domain, int type, int protocol)
{
  printf("Unimplemented: %s(%d, %d, %d)\n", __func__, domain, type, protocol);
  errno = -EINVAL;
  return -1;
}

int connect(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  printf("Unimplemented: %s(%d, %p, %d)\n", __func__, sockfd, addr, addrlen);
  errno = -ECONNREFUSED;
  return -1;
}

int accept(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  printf("Unimplemented: %s(%d, %p, %p)\n", __func__, sockfd, addr, addrlen);
  errno = -EBADF;
  return -1;
}

int bind(int sockfd, const struct sockaddr *addr, socklen_t addrlen)
{
  printf("Unimplemented: %s(%d, %p, %d)\n", __func__, sockfd, addr, addrlen);
  errno = -EINVAL;
  return -1;
}

int getsockopt(int sockfd, int level, int optname,
               void *optval, socklen_t *optlen)
{
  printf("Unimplemented: %s(%d, %d, %d, %p, %p)\n",
         __func__, sockfd, level, optname, optval, optlen);
  errno = -EINVAL;
  return -1;
}

int setsockopt(int sockfd, int level, int optname,
               const void *optval, socklen_t optlen)
{
  printf("Unimplemented: %s(%d, %d, %d, %p, ...)\n",
         __func__, sockfd, level, optname, optval);
  (void)optlen;
  errno = -EINVAL;
  return -1;
}

int listen(int sockfd, int backlog)
{
  printf("Unimplemented: %s(%d, %d)\n", __func__, sockfd, backlog);
  errno = -EBADF;
  return -1;
}

ssize_t recv(int sockfd, void *buf, size_t len, int flags)
{
  printf("Unimplemented: %s(%d, %p, %zd, %d)\n", __func__, sockfd, buf, len, flags);
  errno = -EBADF;
  return -1;
}

ssize_t send(int sockfd, const void *buf, size_t len, int flags)
{
  printf("Unimplemented: %s(%d, %p, %zd, %d)\n", __func__, sockfd, buf, len, flags);
  errno = -EBADF;
  return -1;
}

ssize_t sendto(int sockfd, const void *buf, size_t len, int flags,
               const struct sockaddr *dest_addr, socklen_t addrlen)
{
  printf("Unimplemented: %s(%d, %p, %zd, %d, %p, %d)\n", __func__,
         sockfd, buf, len, flags, dest_addr, addrlen);
  errno = -EBADF;
  return -1;
}


int getsockname(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  printf("Unimplemented: %s(%d, %p, %p)\n", __func__, sockfd, addr, addrlen);
  errno = -EBADF;
  return -1;
}

int getpeername(int sockfd, struct sockaddr *addr, socklen_t *addrlen)
{
  printf("Unimplemented: %s(%d, %p, %p)\n", __func__, sockfd, addr, addrlen);
  errno = -EBADF;
  return -1;
}

int getnameinfo(const struct sockaddr *sa, socklen_t salen,
                char *host, socklen_t hostlen,
                char *serv, socklen_t servlen, unsigned int flags)
{
  printf("Unimplemented: %s(%p, %d, %p, %d, %p, %d, %d)\n", __func__,
         sa, salen, host, hostlen, serv, servlen, flags);
  return EAI_FAIL;
}

int gethostent_r(struct hostent *ret, char *buf, size_t buflen,
                 struct hostent **result, int *h_errnop)
{
  printf("Unimplemented: %s(%p, %p, %zd, %p, %p)\n", __func__,
         ret, buf, buflen, result, h_errnop);
  errno = -EINVAL;
  return -1;
}


int gethostname(char *name, size_t len)
{
  printf("Unimplemented: %s(%s, %zd)\n", __func__,
         name, len);
  errno = -EPERM;
  return -1;
}
