
typedef struct {
  void *context;
  void *subscriber;
  char *group_hash;
  char *user_hash;
  char *host;
  int port;
} subObject;

subObject *make_sub_object(void);
void free_sub_object(subObject *sub_obj);
subObject *subscribe_forwarder(subObject *sub_obj);
void receive_data(void *sub_obj);
