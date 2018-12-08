#ifndef _P_PD_CLASSIFIER_H
#define _P_PD_CLASSIFIER_H

#include "run_environment.h";

struct pd_classifier_t {
        struct classifier_t classifier;
        uint64_t n;
        uint64_t k;
        uint64_t n_factorial;
        uint64_t n_choose_k;
        perm_t   permutation;
        perm_t   pattern;
}; 

int pd_classifier_init(struct run_environment_t *env, perm_t perm, perm_t patt);

#endif
