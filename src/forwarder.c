// License: GPLv3
// Copyright 2012 Jiva Bagale
// jnbagale@gmail.com

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

#define ZMQ_POLL_MSEC 1 //  zmq_poll is msec

static int zmq_custom_broker(brokerObject *broker_obj)
{

  printf("ready to receive message\n");

  // fork a child process to send heartbeat messages

  pid_t heartbeat_pid = fork();  /* fork a new child process for sending the heartbeat message */
  
  if(heartbeat_pid >= 0) {   /* successfully forked a new process */
    if(heartbeat_pid == 0) { /* runs in child process */
      while(1) {
	printf("sending heartbeat message to subscribers\n");
	sleep(5);
      }
    }  /* runs in parent process if else {} statement is used here */
  }
  else { /* fork failed */
    printf("failed to fork child process");
    return -1;
  }

  zmq_pollitem_t items [] = {
    { broker_obj->frontend, 0, ZMQ_POLLIN, 0 },
    { broker_obj->backend,  0, ZMQ_POLLIN, 0 }
  };

  while (1)
    {
      zmq_msg_t message;
      int more = 1;

      zmq_poll(items, 2, -1);

      if (items[0].revents & ZMQ_POLLIN) {
	while (1) {
	  zmq_msg_init(&message);
	  zmq_msg_recv(&message, broker_obj->frontend, 0);
	  size_t more_size = sizeof(more);

	  char *message_str = NULL;
	  int size = zmq_msg_size (&message);

	  if(size > 0) {
	    if((message_str = malloc(size + 1)) != NULL){
	      memcpy (message_str, zmq_msg_data (&message), size);
	      printf("Message string: %s\n", message_str);
	    }
	  }

	  zmq_getsockopt(broker_obj->frontend, ZMQ_RCVMORE, &more, &more_size);
	  zmq_msg_send(&message, broker_obj->backend, more ? ZMQ_SNDMORE : 0);
	  zmq_msg_close(&message);
	  if (!more) {
	    break;
	  }
	}
      }

      if (items[1].revents & ZMQ_POLLIN) {
	while (1) {
	  zmq_msg_init(&message);
	  zmq_msg_recv(&message, broker_obj->backend, 0);
	  size_t more_size = sizeof(more);
	  zmq_getsockopt(broker_obj->backend, ZMQ_RCVMORE, &more, &more_size);
	  zmq_msg_send(&message, broker_obj->frontend, more ? ZMQ_SNDMORE : 0);
	  zmq_msg_close(&message);
	  if (!more) {
	    break;
	  }
	}
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
  broker_obj->frontend = zmq_socket (broker_obj->context, ZMQ_SUB);
  broker_obj->backend = zmq_socket (broker_obj->context, ZMQ_XPUB);
 

  // pass subscription to upstream publishers
  zmq_setsockopt (broker_obj->backend, ZMQ_XPUB_VERBOSE, &xpub_verbose, sizeof(xpub_verbose));
  
  // accept all messages from the publishers
  zmq_setsockopt(broker_obj->frontend, ZMQ_SUBSCRIBE, "", 0);


  // bind sockets for both ends
  zmq_bind (broker_obj->frontend,  frontend_endpoint);
  zmq_bind (broker_obj->backend,   backend_endpoint);

  printf("\nForwarder device is receiving from publishers at %s \nand\n",  frontend_endpoint);
  printf("\nForwarder device is sending to subscribers at %s\n",  backend_endpoint);

  // free up memory 
  free(frontend_endpoint);
  free(backend_endpoint);

  //  Start the forwarder  proxy device
  //zmq_proxy (broker_obj->frontend, broker_obj->backend, NULL);
  zmq_custom_broker(broker_obj);
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
