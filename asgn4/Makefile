PROG=	secret	
SRCS=	secret.c

DPADD+=	${LIBDRIVER} ${LIBSYS}
LDADD+=	-ldriver -lsys

MAN=

BINDIR?= /usr/sbin

down:
	service down secret 

up:
	service up /usr/src/drivers/secrets/secret -dev /dev/Secret

.include <bsd.prog.mk>
