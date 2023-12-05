#include <stdlib.h>
#include <stdio.h>
#include <signal.h>
#include <sys/types.h>
#include <sys/wait.h>
#include <unistd.h>
#include <time.h>
#include <string.h>
#include <sys/msg.h>
#include <sys/shm.h>
#include <sys/ipc.h>
#include <wait.h>
#include <sys/stat.h>


struct uzenet {
    long mtype;
    int view;
};


void handler(int signumber){
    printf("Signal with number %i has arrived\n",signumber);
}

int main(int argc, char **argv) {
    
    signal(SIGTERM,handler);  
    srand(time(NULL));
    //uzenetsor
    int uzenetsor, status;
    key_t kulcs;
    kulcs = ftok(argv[0], 58008);
    msgctl(uzenetsor, IPC_RMID, NULL);
    uzenetsor = msgget( kulcs, 0600 | IPC_CREAT );
    if (uzenetsor < 0) {
        perror("msgget");
        exit(EXIT_FAILURE);
    }
    // pipe 
    int pipe_fb_rockstar[2];
    if( pipe(pipe_fb_rockstar) == -1 ) {
        perror("pipe error");
        exit(EXIT_FAILURE);
    }
    // forkok 
    pid_t fb_child=fork();
    if (fb_child>0) {
        key_t mem_kulcs;
        int oszt_mem_id;
        char *s;
        kulcs = ftok(argv[0], 58008);
        oszt_mem_id = shmget(kulcs, 500, IPC_CREAT|S_IRUSR|S_IWUSR);
        s = shmat(oszt_mem_id, NULL, 0); 
        pid_t yt_child = fork();
        if (yt_child > 0) {
            //parent
            sleep(1); 
            kill(yt_child, SIGTERM);
            kill(fb_child, SIGTERM);
            struct uzenet views[2]; // 0 - yt | 1 - fb
            sleep(1);
            status = msgrcv( uzenetsor, &views[0], sizeof(struct uzenet) - sizeof(long), 0, 1);
            status = msgrcv( uzenetsor, &views[1], sizeof(struct uzenet) - sizeof(long), 0, 2);
            printf("yt nezettseg: %d, fb nezettseg: %d\n", views[0].view, views[1].view);
            // pipe 
            close(pipe_fb_rockstar[1]);
            char pipe_buffer[1024];
            int summed_comments = 0;
            int to_receive;
            for ( int i = 0; i < 2; i++) {
                read(pipe_fb_rockstar[0], &to_receive, sizeof(int));
                read(pipe_fb_rockstar[0], &pipe_buffer, to_receive);
                printf("%s\n", pipe_buffer);
                summed_comments += strlen(pipe_buffer);
            }
            close(pipe_fb_rockstar[0]);
            if(summed_comments > 30) {
                printf("unnep van biza\n");
            } else {
                printf("tanacskozni kell az EA marketinggel\n");
            }
            // oszt mem
            printf("yt-tol jott: %s", s);
            shmdt(s); 
            wait(NULL);
            wait(NULL);
            msgctl( uzenetsor, IPC_RMID, NULL);
            shmctl(oszt_mem_id, IPC_RMID, NULL);
        }
        else {
        // yt_child
            pause();
            struct uzenet uz = { 5, rand() };
            printf("yt: %d\n", uz.view); 
            msgsnd( uzenetsor, &uz, sizeof(struct uzenet) - sizeof(long), 1);
            // oszt mem
            char buffer[] = "tartalom letiltva\n";
            strcpy(s, buffer);
            shmdt(s); 
        }  
       
        /* 
        pause();  
        printf("Signal arrived\n",SIGTERM);
        int status;
        wait(&status);
        printf("Parent process ended\n"); */
    }
    else {
        // fb_child 
        pause();
        srand(time(NULL));
        struct uzenet uz = { 5, rand() };
        printf("fb: %d\n", uz.view); 
        msgsnd( uzenetsor, &uz, sizeof(struct uzenet) - sizeof(long), 2);
        char * comments[3] = {
           "UUUU POG nagoyn jo, alig varom mar",
           "Pff CoD go BRRRRRR",
           "Gepigeny?" };
        close(pipe_fb_rockstar[0]);
        int index;
        int size;
        for(int i = 0; i < 2; i++) {
            index = rand() % 3;
            size = strlen(comments[index]) + 1;
            write(pipe_fb_rockstar[1], &size, sizeof(int));  
            write(pipe_fb_rockstar[1], comments[index], size);
        }
        close(pipe_fb_rockstar[1]); 
    }
    return 0;
}
