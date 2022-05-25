/* Authors : Athanasia Despoina Sapountzi 2624
 *           Vasilis Theiou               2685
 * 14/12/19
 * */
#define CCR_DECLARE(label) int label##n1; int label##n2;\
    pthread_cond_t label##q1;\
    pthread_cond_t label##q2; \
    pthread_mutex_t label##mtx1; \
    pthread_mutex_t label##mtx2; 
 
#define CCR_INIT(label) pthread_mutex_init(&label##mtx1,NULL); \
	pthread_mutex_init(&label##mtx2,NULL);\
	pthread_cond_init(&label##q1,NULL); pthread_cond_init(&label##q2,NULL); \
	label##n1= 0; label##n2 = 0;
   
   
#define CCR_EXEC(label,cond,body) pthread_mutex_lock(&label##mtx1); \
	while(!cond){\
        label##n1++;\
		if(label##n2 > 0){\
			pthread_mutex_lock(&label##mtx2);\
			label##n2--; \
			pthread_cond_signal(&label##q2);\
			pthread_mutex_unlock(&label##mtx2);\
		}\
		else{\
			pthread_mutex_unlock(&label##mtx1);\
		}\
		pthread_mutex_lock(&label##mtx2);\
		if(label##n1 > 0){\
			pthread_cond_wait(&label##q1,&label##mtx2);\
		}\
		pthread_mutex_unlock(&label##mtx2);\
		label##n2++;\
		if(label##n1 > 0){\
			pthread_mutex_lock(&label##mtx2);\
			label##n1--;\
			pthread_cond_signal(&label##q1);\
			pthread_mutex_unlock(&label##mtx2);\
		}\
		else { \
			pthread_mutex_lock(&label##mtx2);\
			label##n2--;\
			pthread_cond_signal(&label##q2);\
			pthread_mutex_unlock(&label##mtx2);\
		}\
		pthread_mutex_lock(&label##mtx2);\
		if(label##n2 > 0){\
			pthread_cond_wait(&label##q2,&label##mtx2);\
		}\
		pthread_mutex_unlock(&label##mtx2);\
	}\
	body ;\
	if(label##n1 > 0){\
		pthread_mutex_lock(&label##mtx2);\
		label##n1--;\
		pthread_cond_signal(&label##q1);\
		pthread_mutex_unlock(&label##mtx2);\
	}\
	else if(label##n2 > 0){\
		pthread_mutex_lock(&label##mtx2);\
		label##n2--;\
		pthread_cond_signal(&label##q2);\
		pthread_mutex_unlock(&label##mtx2);\
	}\
	else{\
		pthread_mutex_unlock(&label##mtx1);\
		pthread_mutex_unlock(&label##mtx2);\
	}
