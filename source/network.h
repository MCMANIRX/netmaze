#include <ogcsys.h>
#include <gccore.h>
#include <network.h>


static	lwp_t httd_handle = (lwp_t)NULL;
void *httpd (void *arg);

int initServer();