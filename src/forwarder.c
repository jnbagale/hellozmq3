// License: GPLv3
// Copyright 2012 The Clashing Rocks
// team@theclashingrocks.org

/* ZeroMQ Forwarder which receives data from publishers and sends it back to subscribers */
/* Binds XPUB socket to given host address or default tcp://127.0.0.1:8100 */
/* Binds XSUB socket to given host address or default tcp://127.0.0.1:5556*/

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <zmq.h>

#include "config.h"
#include "message.h"
#include "forwarder.h"

static int zmq_custom_broker(brokerObject *broker_obj)
{
  while (1)
    {
      int hasMore = 1;
 
      while (hasMore)
	{
	  int size;
	  int64_t more;
	  char *message = NULL;
	  size_t more_size = sizeof (more);

	  /* receive message from publishers */
	  message = receive_message(broker_obj->frontend, &size);

	  /* check if more message is present on the socket */
	  zmq_getsockopt (broker_obj->frontend, ZMQ_RCVMORE, &more, &more_size);

	  /* forward the message to the subscribers */
	  send_message(broker_obj->backend, message, size, more ? 1 : 0);
	  hasMore = more;

	  free(message);
	}
    }

  return -1;
} 

brokerObject *make_broker_object(void)
{
  brokerObject *broker_obj;


  if ((broker_obj = (brokerObject *)malloc(sizeof(brokerObject))) == NULL) {
    //printf("failed to malloc brokerObject!");
    exit(EXIT_FAILURE);
  }

  return broker_obj;
}

void start_forwarder(brokerObject *broker_obj)
{
  int xpub_verbose = 1;
  char *frontend_endpoint;
  char *backend_endpoint;

  frontend_endpoint = malloc(1000);
  backend_endpoint = malloc(1000);

  sprintf(frontend_endpoint, "tcp://%s:%d", broker_obj->host, broker_obj->pub_port);
  sprintf(backend_endpoint, "tcp://%s:%d", broker_obj->host, broker_obj->sub_port);

  //  prepare ZeroMQ context and sockets
  broker_obj->context = zmq_ctx_new ();
  broker_obj->frontend = zmq_socket (broker_obj->context, ZMQ_XSUB);
  broker_obj->backend = zmq_socket (broker_obj->context, ZMQ_XPUB);
 

  // pass subscription to upstream publishers
  zmq_setsockopt (broker_obj->backend, ZMQ_XPUB_VERBOSE, &xpub_verbose, sizeof(xpub_verbose));

  // bind sockets for both ends
  zmq_bind (broker_obj->frontend,  frontend_endpoint);
  zmq_bind (broker_obj->backend, backend_endpoint);

  printf("\nForwarder device is receiving at %s\n", frontend_endpoint);
  printf("\nForwarder device is sending from %s\n", backend_endpoint);

  // free up memory 
  free(frontend_endpoint);
  free(backend_endpoint);

  //  Start the forwarder  proxy device
  zmq_proxy (broker_obj->frontend, broker_obj->backend, NULL);
  // zmq_custom_broker(broker_obj);
}

void free_broker_object(brokerObject *broker_obj)
{
  zmq_close(broker_obj->backend);
  zmq_close(broker_obj->frontend);
  zmq_ctx_destroy(broker_obj->context);
  free(broker_obj->group);
  free(broker_obj->host);
  free(broker_obj);  
}
/* End of forwarder.c */
