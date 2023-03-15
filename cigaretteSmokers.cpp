#include <bits/stdc++.h>
#include <pthread.h>
#include <mutex>
#include <condition_variable>
#include <pthread.h>

using namespace std;

pthread_mutex_t mtx;
pthread_cond_t cv;
bool is_tobacco = false;
bool is_paper = false;
bool is_match = false;

void* agent(void*) {
    while (true) {
        pthread_mutex_lock(&mtx);
        while (is_tobacco && is_paper && is_match) {
            pthread_cond_wait(&cv, &mtx);
        }
        int rand_num = rand() % 3;
        if (rand_num == 0) {
            is_tobacco = true;
            is_paper = true;
            cout << "Agent puts tobacco and paper on the table.\n" << endl;
        } else if (rand_num == 1) {
            is_paper = true;
            is_match = true;
            cout << "Agent puts paper and matches on the table.\n" << endl;
        } else {
            is_tobacco = true;
            is_match = true;
            cout << "Agent puts tobacco and matches on the table.\n" << endl;
        }
        pthread_cond_broadcast(&cv);
        pthread_mutex_unlock(&mtx);
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

void* smoker_tobacco(void*) {
    while (true) {
        pthread_mutex_lock(&mtx);
        while (!is_paper || !is_match) {
            pthread_cond_wait(&cv, &mtx);
        }
        is_paper = false;
        is_match = false;
        cout << "Smoker with tobacco picks up paper and matches." << endl;
        cout<<"-----------------------------------\n";
        pthread_cond_broadcast(&cv);
        pthread_mutex_unlock(&mtx);
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

void* smoker_paper(void*) {
    while (true) {
        pthread_mutex_lock(&mtx);
        while (!is_tobacco || !is_match) {
            pthread_cond_wait(&cv, &mtx);
        }
        is_tobacco = false;
        is_match = false;
        cout << "Smoker with paper picks up tobacco and matches." << endl;
        cout<<"-----------------------------------\n";
        pthread_cond_broadcast(&cv);
        pthread_mutex_unlock(&mtx);
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

void* smoker_match(void*) {
    while (true) {
        pthread_mutex_lock(&mtx);
        while (!is_tobacco || !is_paper) {
            pthread_cond_wait(&cv, &mtx);
        }
        is_tobacco = false;
        is_paper = false;
        cout << "Smoker with match picks up tobacco and paper." << endl;
        cout<<"-----------------------------------\n";
        pthread_cond_broadcast(&cv);
        pthread_mutex_unlock(&mtx);
        this_thread::sleep_for(chrono::milliseconds(1000));
    }
}

int main() {
    pthread_t t1, t2, t3, t4;
    pthread_mutex_init(&mtx, NULL);
    pthread_cond_init(&cv, NULL);
    pthread_create(&t1, NULL, &agent, NULL);
    pthread_create(&t2, NULL, &smoker_tobacco, NULL);
    pthread_create(&t3, NULL, &smoker_paper, NULL);
    pthread_create(&t4, NULL, &smoker_match, NULL);
    pthread_join(t1, NULL);
    pthread_join(t2, NULL);
    pthread_join(t3, NULL);
    pthread_join(t4, NULL);
    return 0;
}