#ifndef _P_COMMON_H
#define _P_COMMON_H

/* Parent 'class' for classifier modules */
struct classifier_t {
        uint64_t id; 
};

/* Specifies data in the run environment */
struct run_environment_t {
        uint64_t max_workers;
        uint64_t num_batches;
        char    *run_label;
        FILE    *log_file;
        struct classifier_t *classifier;
};

int env_setup(
        struct run_environment_t *env, 
        char                     *run_label,
        struct classifier_t      *classifier
);

//int env_setup(
        //struct run_environment_t *env, 
        //uint64_t                  max_workers,
        //uint64_t                  num_batches,
        //char                     *run_label,
        //char                     *log_file,
        //struct classifier_t      *classifier
//);
        

#endif
