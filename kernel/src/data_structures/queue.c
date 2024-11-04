//
// Created by dustyn on 6/21/24.
//

#include "include/data_structures/queue.h"
#include <include/definitions.h>
#include <include/mem/kalloc.h>
#include <include/types.h>
#include <include/arch/arch_cpu.h>


/*
 *  The idea behind all of this here is to create a generic and flexible queue interface with many different modes.
 *
 *  FIFO (classical queue) mode
 *  LIFO (stack) mode
 *  Priority (scheduling) mode
 *  Circular (maybe packet buffering , not sure yet) - Will leave unimplemented for now
 *  Double ended (not sure yet) - Will leave unimplemented for now
 *
 *  Note for clarity : the direction of next and prev amongst nodes in a queue
 *
 *  HEAD->next->next->next->TAIL
 *  TAIL->prev->prev->prev->HEAD
 *
 */

/*
 *  Internal function prototypes
 */

static void dequeue_priority(struct queue* queue_head);
static void dequeue_fifo(struct queue* queue_head);
static void dequeue_lifo(struct queue* queue_head);

static void enqueue_priority(struct queue* queue_head, struct queue_node* new_node);
static void enqueue_fifo(struct queue* queue_head, struct queue_node* new_node);
static void enqueue_lifo(struct queue* queue_head, struct queue_node* new_node);
/*
 * Init a queue
 */
void queue_init(struct queue* queue_head, uint8 queue_mode, char* name) {
    queue_head->name = name;
    queue_head->head = NULL;
    queue_head->tail = NULL;
    queue_head->queue_mode = queue_mode;
}

/*
 *  Internal function to init a single node
 */
void _queue_node_init(struct queue_node* node, void* data, uint8 priority) {
    if (node == NULL) {
        //I'll use panics to easily figure out if these things happen and can work from there if it ever is an issue
        panic("_queue_node_init called with NULL node");
    }

    node->data = data;
    node->priority = priority;
    node->next = NULL;
    node->prev = NULL;
}

/*
 *  Generic enqueue, it will check the queue_mode field to tell what mode this is, and call the appropriate function.
 */

void enqueue(struct queue* queue_head, void* data_toenqueue, uint8 priority) {
    uint8 queue_head_empty = 0;
    if (queue_head == NULL) {
        //So i can investigate when it happens, should have null data field not the entire head structure null
        panic("Null head");
    }

    struct queue_node* new_node = kalloc(sizeof(struct queue_node));
    _queue_node_init(new_node, data_toenqueue, priority);


    switch (queue_head->queue_mode) {
    case QUEUE_MODE_FIFO:
        enqueue_fifo(queue_head, new_node);
        break;

    case QUEUE_MODE_LIFO:
        enqueue_lifo(queue_head, new_node);
        break;

    case QUEUE_MODE_PRIORITY:
        enqueue_priority(queue_head, new_node);
        break;

    case QUEUE_MODE_CIRCULAR:
        break;

    case QUEUE_MODE_DOUBLE_ENDED:
        break;

    default:
        break;
    }
}

/*
 *  Generic dequeue, it will check the queue_mode field to tell what mode this is, and call the appropriate function.
 */
void dequeue(struct queue* queue_head) {
    if (queue_head == NULL) {
        //So i can investigate when it happens, should have null data field not the entire head structure null
        panic("Null head");
    }

    switch (queue_head->queue_mode) {
    case QUEUE_MODE_FIFO:
        dequeue_fifo(queue_head);
        break;

    case QUEUE_MODE_LIFO:
        dequeue_lifo(queue_head);
        break;

    case QUEUE_MODE_PRIORITY:
        dequeue_priority(queue_head);
        break;

    case QUEUE_MODE_CIRCULAR:
        break;

    case QUEUE_MODE_DOUBLE_ENDED:
        break;

    default:
        break;
    }
}

/*
 * Your run-of-the-mill enqueue as-you-know-it function. Stick it last in the queue and update the pointers and node count appropriately
 */
static void enqueue_fifo(struct queue* queue_head, struct queue_node* new_node) {
    /*
     * Base cases
     */
    if (new_node == NULL) {
        panic("Null head enqueue fifo");
    }

    if (new_node->data == NULL) {
        panic("Null data enqueue fifo");
        kfree(new_node);
        return;
    }

    if (queue_head->node_count == 0) {
        queue_head->head = new_node;
        queue_head->tail = new_node;
        queue_head->node_count++;
        return;
    }

    if (queue_head->node_count == 1) {
        queue_head->head->next = new_node;
        new_node->prev = queue_head->head;
        queue_head->tail = new_node;
        queue_head->node_count++;
        return;
    }

    queue_head->tail->next = new_node;
    new_node->prev = queue_head->tail;
    queue_head->tail = new_node;
    queue_head->node_count++;
}

/* Your stack implementation of the queue, it goes to the head as opposed to the tail of the queue
 *
 *
 * Is it really a queue when using this mode? I don't know I'm not a philosopher. You should ask Theseus.
 *
 */

static void enqueue_lifo(struct queue* queue_head, struct queue_node* new_node) {
    /*
    * Base cases
    */

    if (new_node->data == NULL) {
        panic("Null head in enqueue lifo");
        kfree(queue_head);
        return;
    }

    if (queue_head->node_count == 0) {
        queue_head->head = new_node;
        queue_head->tail = new_node;
        queue_head->node_count++;
        return;
    }


    queue_head->head->prev = new_node;
    queue_head->head = new_node;
    queue_head->node_count++;
}
/*
 *  Enqueue for priority mode which is for scheduling. It will place based on the priority value of the
 *  node to be placed. Again, is it really a queue? Ask Theseus.
 */
static void enqueue_priority(struct queue* queue_head, struct queue_node* new_node) {
    if (new_node->data == NULL) {
        panic("Null head in enqueue priority");
        kfree(queue_head);
        return;
    }


    if (queue_head->node_count == 0) {
        queue_head->head = new_node;
        queue_head->tail = new_node;
        queue_head->node_count++;
        return;
    }

    if (queue_head->node_count == 1) {
        /*
         *  Switch mayh seem a little weird but I feel icky about any nested if statements and I prefer this
         */

        switch (queue_head->head->priority > new_node->priority) {
        case 1:
            queue_head->head->prev = new_node;
            new_node->next = queue_head->head;
            queue_head->head = new_node;
            break;

        case 0:
            queue_head->head->next = new_node;
            new_node->prev = queue_head->head;
            queue_head->tail = new_node;
            break;
        }

        queue_head->node_count++;
        return;
    }

    //This case might save a full traversal which can be meaningful in the case of a large queue
    if (queue_head->tail->priority >= new_node->priority) {
        queue_head->tail->next = new_node;
        new_node->prev = queue_head->tail;
        queue_head->tail = new_node;
        queue_head->node_count++;
        return;
    }

    //Base case where it can be inserted right in front
    if (queue_head->head->priority <= new_node->priority) {
        new_node->next = queue_head->head->next;
        queue_head->head->next = new_node;
        new_node->prev = queue_head->head;
        queue_head->node_count++;
        return;
    }

    struct queue_node* pointer = queue_head->head;

    //Otherwise walk the queue and find an appropriate place to insert
    for (;;) {
        if (pointer == NULL) {
            break;
        }
        //Since we did a check above checking if the tail or head can be swapped, we shouldn't need to worry about updating head or tail here since this surely
        //must be somewhere in between
        if (pointer->priority < new_node->priority) {
            new_node->next = pointer;
            new_node->prev = pointer->prev;
            pointer->prev = new_node;
            queue_head->node_count++;
            return;
        }


        pointer = pointer->next;
    }
}
/*
 *  Dequeue function for your typical queue mode. Remove the head, update pointers accordingly
 */
static void dequeue_fifo(struct queue* queue_head) {
    struct queue_node* pointer = queue_head->head;

    if (pointer == NULL) {
        return;
    }

    queue_head->head = pointer->next;
    queue_head->head->prev = NULL;
    queue_head->node_count--;

    if (queue_head->node_count == 1) {
        queue_head->tail = queue_head->head;
    }

    kfree(pointer);
}

/*
 *  Dequeue mode for your stack mode. Pop off the head.
 *  Update pointers accordingly
 */
static void dequeue_lifo(struct queue* queue_head) {
    struct queue_node* pointer = queue_head->head;

    if (pointer == NULL) {
        return;
    }

    if (queue_head->node_count == 1) {
        kfree(pointer);
        queue_head->head = NULL;
        queue_head->tail = NULL;
        queue_head->node_count--;
        return;
    }

    pointer->next->prev = NULL;
    queue_head->head = pointer->next;
    queue_head->node_count--;

    kfree(pointer);
}
/*
 *  Dequeue for priority. This one always just copies the normal queue behaviour since it was written
 *  with scheduling in mind.
 */
static void dequeue_priority(struct queue* queue_head) {
    struct queue_node* pointer = queue_head->head;

    if (pointer == NULL) {
        return;
    }

    queue_head->head = pointer->next;
    queue_head->head->prev = NULL;
    queue_head->node_count--;

    if (queue_head->tail == pointer) {
        queue_head->tail = queue_head->head;
    }

    kfree(pointer);
}
