// License: GPLv3
// Copyright 2012 Jiva Bagale
// jnbagale@gmail.com

#include <stdio.h>
#include <stdlib.h>
#include <string.h>
#include <unistd.h>
#include <time.h>
#include <zmq.h>

#include "config.h"
#include "message.h"
#include "publisher.h"

pubObject *make_pub_object(void)
{
  pubObject *pub_obj;

  if ((pub_obj = (pubObject *)malloc(sizeof(pubObject))) == NULL) {
    //printf("failed to malloc pubObject!");
    exit(EXIT_FAILURE);
  }

  pub_obj->publish = 1;
  return pub_obj;
}

pubObject *publish_forwarder(pubObject *pub_obj)
{
  char *forwarder_address =  malloc(1000);
  
  sprintf(forwarder_address, "tcp://%s:%d", pub_obj->host, pub_obj->port);

  /* Prepare zeromq context and publisher */
  pub_obj->context = zmq_ctx_new ();
  pub_obj->publisher = zmq_socket (pub_obj->context, ZMQ_PUB);
  zmq_connect(pub_obj->publisher, forwarder_address);
  
  printf("Now sending data to forwarder at %s for group %s\n", forwarder_address, pub_obj->group_id);
  free(forwarder_address);
  return pub_obj;
}

void *send_data(void *pub_obj)
{
  int count = 0;
  char data [100];

  pubObject *pub_obj1 =  (pubObject *)pub_obj;

  while (1) {

    // Send message to all subscribers of default group: world
    //sprintf (data,"%s %s %d", pub_obj1->group_id, pub_obj1->user_id, count);
    // send_message (pub_obj1->publisher, data, strlen(data), 0);

    // sending as multi-part messages
    //////////// Topic /////////////
    send_message (pub_obj1->publisher, pub_obj1->group_id, strlen(pub_obj1->group_id), ZMQ_SNDMORE);
    //////////// Sub Topic ////////
    send_message (pub_obj1->publisher, pub_obj1->user_id, strlen(pub_obj1->user_id), ZMQ_SNDMORE);
    //////////// Data /////////////
    sprintf (data,"%d", count);
    send_message (pub_obj1->publisher, data, strlen(data), 0);

    printf("Sent : %s %s %s \n", pub_obj1->group_id, pub_obj1->user_id, data);
    count++;
    sleep(10);
  }
}

void free_pub_object(pubObject *pub_obj)
{
  zmq_close (pub_obj->publisher);
  zmq_ctx_destroy(pub_obj->context);
  free(pub_obj->group_id);
  free(pub_obj->user_id);
  free(pub_obj->host);
  free(pub_obj);  
}
