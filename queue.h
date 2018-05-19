struct tdata{
  int csockd; // client Socket Descriptor
  int request_id; //Req ID for this client
  struct tdata* link;
};

struct qnode{
	int key;
	int id;
	struct qnode *next;
};


struct work_queue {
	struct qnode *front;
	struct qnode *rear;
} ;

struct qnode *new_node(int k, int id);
struct work_queue wq;

void addq(int k, int id, struct work_queue *q);
struct qnode* removeq(struct work_queue *q);

void addq(int k, int id, struct work_queue *q);
struct qnode* removeq(struct work_queue *q);

 




