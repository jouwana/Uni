#ifndef THREAD_SAFE_LIST_H_
#define THREAD_SAFE_LIST_H_

#include <pthread.h>
#include <iostream>
#include <iomanip> // std::setw

using namespace std;

template <typename T>
class List 
{
	//private section is at the bottom, include a initLock
	
    public:
        /**
         * Constructor
         */
        List()
        {
            //make empty head
            this->head = NULL;
            /// initLock is only for first ever insert (initialization)
            this->initLock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
            if(initLock == NULL)
            {
                std::cerr << "malloc: failed";
                exit(-1);
            }
		
	    // if you get an error - delete the next "if" section
	    if (pthread_mutex_init(initLock, NULL) != 0)
            {

                free(initLock);

           	return nullptr;
            }


//            this->head = makeNode(NULL,NULL);
//            if(this->head == nullptr) {
//                std::cerr << "makeNode: failed";
//                exit(-1);
//            }
//            this->head->ishead = true;
        }

        /**
         * Destructor
         */
        virtual ~List(){
                List_delete(this);
                pthread_mutex_destroy(initLock);
                free(initLock);
             }

        class Node {
         public:
          T data;
          Node *next;
          pthread_mutex_t* nodeLock; // so as to not use & when locking
          bool ishead;
        };

        /**
         * Insert new node to list while keeping the list ordered in an ascending order
         * If there is already a node has the same data as @param data then return false (without adding it again)
         * @param data the new data to be added to the list
         * @return true if a new node was added and false otherwise
         */
        bool insert(const T& data) {
            Node *prev = this->head;

            /// NOT GLOBAL, WILL BE UNLOCKED IF HEAD != NULL  !!!
            pthread_mutex_lock(initLock);
            if(prev == NULL)
            {

                //empty list never initialized
                Node *newNode = makeNode(data, NULL);
                if (newNode == nullptr) {
                    pthread_mutex_unlock(initLock);
                    return false;
                }
                Node *newNode2 = makeNode(data, NULL);
                if (newNode2 == nullptr) {
                    free(newNode->nodeLock);
                    free(newNode);
                    pthread_mutex_unlock(initLock);
                    return false;
                }
                this->head = newNode;
                this->head->ishead=true;
                this->head->next = newNode2;
                __add_hook();
                pthread_mutex_unlock(initLock);
                return true;
            }
            pthread_mutex_unlock(initLock);
            /// initLock UNLOCKED -> HAS OVER GONE FIRST INITIALIZATION!!!

            //header is not null
            pthread_mutex_lock(prev->nodeLock);
            if (prev->next == NULL) {
                //empty list
                Node *newNode = makeNode(data, NULL);
                if (newNode == nullptr) {
                    pthread_mutex_unlock(prev->nodeLock);
                    return false;
                }
                prev->next = newNode;
                 __add_hook();
                pthread_mutex_unlock(prev->nodeLock);
                return true;
            }


            //find correct place for node and add it if it doesnt exist

            Node *elem = prev->next;
            pthread_mutex_lock(elem->nodeLock);

            while (elem->next != NULL && elem->next->data <= data) {
                if (elem->data == data) {
                    //node exists
                    //unlock and return false
                    pthread_mutex_unlock(prev->nodeLock);
                    pthread_mutex_unlock(elem->nodeLock);
                    return false;
                }

                pthread_mutex_unlock(prev->nodeLock);
                prev = elem;
                elem = elem->next;
                pthread_mutex_lock(elem->nodeLock);
            }

            //make sure last node is not data
            if (elem->data == data) {
                pthread_mutex_unlock(prev->nodeLock);
                pthread_mutex_unlock(elem->nodeLock);
                return false;
            }


            //make sure the first node isnt bigger than data
            if(elem->data > data && elem == head->next)
            {
                Node *newNode = makeNode(data, elem);
                if (newNode == nullptr) {
                    pthread_mutex_unlock(prev->nodeLock);
                    pthread_mutex_unlock(elem->nodeLock);
                    return false;
                }
                prev->next = newNode;
                __add_hook();
                pthread_mutex_unlock(prev->nodeLock);
                pthread_mutex_unlock(elem->nodeLock);
                return true;

            }

            //if elem isnt the last node in the list
            if (elem->next != NULL) {
                //update prev and elem
                pthread_mutex_unlock(prev->nodeLock);
                prev = elem;
                elem = elem->next;
                pthread_mutex_lock(elem->nodeLock);


                //put it inbetween prev and elem
                Node *newNode = makeNode(data, elem);
                if (newNode == nullptr) {
                    pthread_mutex_unlock(prev->nodeLock);
                    pthread_mutex_unlock(elem->nodeLock);
                    return false;
                }
                prev->next = newNode;
                 __add_hook();
                //unlock
                pthread_mutex_unlock(prev->nodeLock);
                pthread_mutex_unlock(elem->nodeLock);
                return true;
            }

            //if elem is the last node in the list
            else //add it after elem
            {
                //dont need the lock on prev
                pthread_mutex_unlock(prev->nodeLock);

                Node *newNode = makeNode(data, NULL);
                if (newNode == nullptr) {
                    pthread_mutex_unlock(elem->nodeLock);
                    return false;
                }
                elem->next = newNode;
                __add_hook();
                pthread_mutex_unlock(elem->nodeLock);
                return true;
            }
        }


        /**
         * Remove the node that its data equals to @param value
         * @param value the data to lookup a node that has the same data to be removed
         * @return true if a matched node was found and removed and false otherwise
         */
        bool remove(const T& value) {
			Node* prev = this->head;
			if(prev == NULL)
            {
			    //empty list never initialized
                return false;
            }
			pthread_mutex_lock(prev->nodeLock);

			if(prev->next == NULL)
            {
			    //empty list
			    pthread_mutex_unlock(prev->nodeLock);
			    return false;
            }

			Node* elem = prev->next;
			pthread_mutex_lock(elem->nodeLock);

			while(elem->next != NULL && elem->data<= value)
            {
			    if(elem->data == value)
                {
			        //found data
			        prev->next = elem->next;
			        __remove_hook();
			        //unlock and deallocate
			        pthread_mutex_unlock(elem->nodeLock);
			        pthread_mutex_destroy(elem->nodeLock);
			        free(elem->nodeLock);
			        free(elem);
			        pthread_mutex_unlock(prev->nodeLock);
			        return true;
                }
			    pthread_mutex_unlock(prev->nodeLock);
			    prev = elem;
			    elem=elem->next;
			    pthread_mutex_lock(elem->nodeLock);
            }

			//check if last node is not val
            //elem will never be NULL

            if(elem->data == value)
            {
                //found data
                prev->next = elem->next;
                __remove_hook();

                //unlock and deallocate
                pthread_mutex_unlock(elem->nodeLock);
                pthread_mutex_destroy(elem->nodeLock);
                free(elem->nodeLock);
                free(elem);
                pthread_mutex_unlock(prev->nodeLock);
                return true;
            }
			//reached the end and didnt find it
			//unlock
			pthread_mutex_unlock(prev->nodeLock);
            pthread_mutex_unlock(elem->nodeLock);
			return false;
        }

        /**
         * Returns the current size of the list
         * @return the list size
         */
        unsigned int getSize() {
			int size = 0;
			//lock list while counting
			Node* prev = this->head;
			if(prev == NULL)
            {
			    //empty list never initialized
			    return size;
            }
			pthread_mutex_lock(prev->nodeLock);
			if(prev->next == NULL)
            {
			    //list is empty
			    pthread_mutex_unlock(prev->nodeLock);
			    return size;
            }
			Node* elem = prev->next;
			pthread_mutex_lock(elem->nodeLock);
			size++;

			//go over list
			while(elem->next!=NULL)
            {
			    size++;
			    pthread_mutex_unlock(prev->nodeLock);
			    prev=elem;
			    elem=elem->next;
			    pthread_mutex_lock(elem->nodeLock);
            }

			//reached lsat node
			pthread_mutex_unlock(prev->nodeLock);
			pthread_mutex_unlock(elem->nodeLock);
			return size;
        }

		// Don't remove
        void print() {
          //pthread_mutex_lock(&list_mutex);
          if(head == NULL)
          {
              cout << "";
              return;
          }
          ///head is an empty node pointing to the start of list
          Node* temp = head->next;
          if (temp == NULL)
          {
            cout << "";
          }
          else if (temp->next == NULL)
          {
            cout << temp->data;
          }
          else
          {
            while (temp != NULL)
            {
              cout << right << setw(3) << temp->data;
              temp = temp->next;
              cout << " ";
            }
          }
          cout << endl;
          //pthread_mutex_unlock(&list_mutex);
        }

		// Don't remove
        virtual void __add_hook() {}
		// Don't remove
        virtual void __remove_hook() {}

    private:
        Node* head;

        //lock for intilization purposes (or first ever insert)
        pthread_mutex_t* initLock;
    // TODO: Add your own methods and data members



    //helper function that makes and returns node
    Node* makeNode(T _data, Node* _next) {
        Node *node = (Node*)malloc(sizeof(Node));
        if (node == NULL)
            return nullptr;
        node->nodeLock = (pthread_mutex_t*)malloc(sizeof(pthread_mutex_t));
        if(node->nodeLock == NULL)
        {
            free(node);
            return nullptr;
        }
        if (pthread_mutex_init(node->nodeLock, NULL) != 0)
        {
            free(node->nodeLock);
            free(node);
            return nullptr;
        }

        else {
            node->data = _data;
            node->next = _next;
            node->ishead = false;
        }
        return node;
    }

    //helper function to delete the list
    void List_delete(List* list)
    {
        Node* elem = list->head;
        pthread_mutex_lock(elem->nodeLock);
        if(elem->next == NULL)
        {
            //we have an empty list
            //unlock nodelock
            pthread_mutex_unlock(elem->nodeLock);
            pthread_mutex_destroy(elem->nodeLock);

            //deallocate memory
            free(elem->nodeLock);
            free(elem);

            return;
        }
        //if list not empty
        while(elem->next!=NULL)
        {
            //lock everything
            pthread_mutex_lock(elem->next->nodeLock);
            elem=elem->next;
        }

        //delete all elements
        while(list->head!=NULL)
        {
            elem=list->head;
            list->head = elem->next;

            //unlock nodelock
            pthread_mutex_unlock(elem->nodeLock);
            pthread_mutex_destroy(elem->nodeLock);

            //deallocate memory
            free(elem->nodeLock);
            free(elem);
        }

        return;
    }
};

#endif //THREAD_SAFE_LIST_H_
