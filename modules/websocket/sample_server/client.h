
struct session_data {
	int fd;
};

struct pthread_routine_data {
	struct lws_context *context;
	struct lws *wsi;
};
