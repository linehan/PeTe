#include <time.h>
#include "run_environment.h"

/* Creates the run directory */
static 
int __setup_run_directory(char *run_label)
{
        if (NULL == run_label) {
                printf("'run_label' is NULL\n");
                return -1;
        }
        if (-1 == mkdir(run_label)) {
                printf("error in mkdir()\n");
                return -1;
        }

        return 0;
}

/* Creates the log file */
static 
FILE *__setup_log_file(char *run_label)
{
        FILE *log;
        char buf[4096];

        if (NULL == run_label) {
                printf("'run_label' is NULL\n");
                return -1;
        }

        if (0 > snprintf(buf, 4096, "%s/monitor.log", run_label)) {
                printf("couldn't snprintf()\n");
                return NULL;
        }

        if (NULL == (log = fopen(buf, "w+"))) {
                printf("couldn't fopen()\n");
                return NULL;
        }

        return log;
}


int env_attach_workers(
        struct run_environment *env,
        uint64_t max_workers,
        uint64_t num_batches,
)
{

}

int env_attach_classifier(
        struct run_environment_t *env, 
        char *run_label,
        struct classifier_t *classifier
)
{
        if (NULL == env) {
                printf("'env' is NULL\n");
                return -1;
        }
        if (NULL == run_label) {
                printf("'run_label' is NULL\n");
                return -1;
        }
        if (NULL == classifier) {
                printf("'classifier' is NULL\n");
                return -1;
        }
        if (NULL == (env->run_label = strdup(run_label))) {
                printf("error in ()\n");
                return -1;
        }
        if (-1 == __setup_run_directory(env->run_label)) {
                printf("error in __setup_run_directory()\n");
                return -1;
        }
        if (NULL == (env->log_file = __setup_log_file(env->run_label))) {
                printf("error in __setup_log_file()\n");
                return -1;
        }

        env->classifier = classifier; 

        return 0;
}
