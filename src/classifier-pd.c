/* Get a label for the run */
static
int __get_run_label(int n, perm_t pattern)
{
        char       buff_lg[4096];
        char       buff_sm[16];
        char      *str_pattern;
        time_t     time_unix;
        struct tm *time_fancy;

        if (NULL == run) {
                printf("error in setup_run_label()\n");
                return -1;
        }
        if (-1 == time(&time_unix)) {
                printf("error in time()\n");
                return -1;
        }
        if (NULL == (time_fancy = localtime(&time_unix))) {
                printf("error in localtime()\n");
                return -1;
        }
        if (NULL == (str_pattern = perm_get_string(run->pattern))) {
                printf("error in perm_get_string()\n");
                return -1;
        }

        strftime(buff_sm, 15, "%Y%m%d", time_fancy);
        snprintf(buff_lg, 4096, "%d-%s-%s", (int)run->n, str_pattern, buff_sm);

        return strdup(buff_lg);
}


int pd_classifier_init(struct run_environment_t *env, perm_t perm, perm_t patt)
{
        if (env == NULL) {
                return -1;
        }

        struct pd_classifier_t *pd = calloc(1, sizeof(struct pd_classifier_t));

        pd->permutation = permutation;
        pd->pattern     = pattern;

        pd->n           = perm_length(pd->permutation);
        pd->k           = perm_length(pd->pattern);
        pd->n_factorial = math_factorial(pd->n);
        pd->n_choose_k  = math_nchoosek(pd->n, pd->k);

        char *run_label = NULL;

        if (NULL == (run_label = __get_run_label(env->n, env->pattern))) {
                printf("error in __get_run_label()\n");
                return -1;
        }

        if (-1 == env_setup(env, run_label, pd)) {
                printf("error in env_setup()\n");
                return -1;
        }

        return 0;
}
