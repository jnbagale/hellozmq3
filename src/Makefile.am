

INCLUDES = -Wall -g  $(LIBZMQ_CFLAGS)  $(UUID_CFLAGS) 

bin_PROGRAMS =  broker node

broker_SOURCES = broker.c forwarder.c forwarder.h message.c message.h config.h

broker_LDADD =  $(LIBZMQ_LIBS)  $(UUID_LIBS)

node_SOURCES = node.c subscriber.c subscriber.h publisher.c publisher.h message.c message.h zhelpers.h config.h

node_LDADD =  $(LIBZMQ_LIBS)  $(UUID_LIBS)


