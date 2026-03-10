#include "shell.h"

/* Global job list */
static Job jobs[MAX_JOBS];
static int job_count = 0;

void init_jobs(void)
{
    job_count = 0;
    memset(jobs, 0, sizeof(jobs));
}

int add_job(pid_t pid, JobState state, const char *command)
{
    if (job_count >= MAX_JOBS) {
        fprintf(stderr, "Job list is full\n");
        return -1;
    }

    jobs[job_count].job_id = job_count + 1;
    jobs[job_count].pid = pid;
    jobs[job_count].state = state;
    strncpy(jobs[job_count].command, command, sizeof(jobs[job_count].command) - 1);
    jobs[job_count].command[sizeof(jobs[job_count].command) - 1] = '\0';

    job_count++;
    return jobs[job_count - 1].job_id;
}

int remove_job(pid_t pid)
{
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            int job_id = jobs[i].job_id;
            for (int j = i; j < job_count - 1; j++) {
                jobs[j] = jobs[j + 1];
            }
            job_count--;
            return job_id;
        }
    }
    return -1;
}

Job *find_job_by_id(int job_id)
{
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].job_id == job_id) {
            return &jobs[i];
        }
    }
    return NULL;
}

Job *find_job_by_pid(pid_t pid)
{
    for (int i = 0; i < job_count; i++) {
        if (jobs[i].pid == pid) {
            return &jobs[i];
        }
    }
    return NULL;
}

int update_job_state(pid_t pid, JobState new_state)
{
    Job *job = find_job_by_pid(pid);
    if (job) {
        job->state = new_state;
        return 0;
    }
    return -1;
}

Job *list_jobs(void)
{
    return jobs;
}

int get_job_count(void)
{
    return job_count;
}
