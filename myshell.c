#include <stdio.h>
#include <stdlib.h>
#include <readline/readline.h>
#include <readline/history.h>
#include <sys/types.h>
#include <unistd.h>
#include <sys/wait.h>

char **command_completion(const char *text, int start, int end);
static char* rl_gets();

int cmd_help(char *argv);
int cmd_history(char *argv);
int cmd_q(char *argv);


static struct {
    const char *name;
    const char *description;
    int (*handler) (char *);
} cmd_table [] = {
    { "help",    "Display information about all supported commands", cmd_help},
    { "history", "Display history input", cmd_history},
    { "q",       "quit this shell", cmd_q},
    /* TODO: Add more commands */
};

int main() {
    // 初始化 readline 历史记录
    using_history();
    rl_attempted_completion_function = command_completion;
    char *str, str2[200];
    while(1) {
        str = rl_gets();
        strcpy(str2, str);
back_flag:        
        if(str == NULL) break;

        char *str_end = str + strlen(str);
        char *cmd = strtok(str, " ");

        if (cmd == NULL) { continue; }

        if (str[0] == '!') {
            int index = 0;
            if(*(str + 1) == '!')
            {
                index = history_length - 2;
            } 
            else {
                index = atoi(str  + 1);
            }
            HIST_ENTRY **history_entry = history_list();
            char *buffer;
            buffer = strtok(history_entry[index]->line, " ");
            while(strcmp(buffer, "!!") == 0) {
                index --;
                buffer = strtok(history_entry[index]->line, " ");
            }
            str = buffer;
            goto back_flag;
        }
        *(cmd + strlen(cmd)) = '\0';
        char *args = cmd + strlen(cmd) + 1;
        if (args >= str_end) {
            args = NULL;
        }

        int i;
        for (i = 0; i < (sizeof(cmd_table) / sizeof(cmd_table[i])); i ++) {
            if (strcmp(cmd, cmd_table[i].name) == 0) {
                cmd_table[i].handler(args);
                break;
            }
        }
        if (i == (sizeof(cmd_table) / sizeof(cmd_table[i]))) 
        { 
            printf(" '%s' Not in internal commands and will run in bash\n", str2);
            char *token = strtok(str2, " ");
            char *sub_args[200] = {NULL};
            int sub_args_index = 0;

            while (token != NULL) {
                sub_args[sub_args_index ++] = token;
                token = strtok(NULL, " ");
            }

            int is_parallel = 0;
            if(strcmp("&", sub_args[sub_args_index - 1]) == 0)
            {
                is_parallel = 1;
                sub_args_index --;
            } 
            sub_args[sub_args_index] = NULL;
            
            pid_t pid = fork();
            if (pid < 0) {
                fprintf(stderr, "创建子进程失败\n");
                return 1;
            } else if (pid == 0) {
                // 子进程执行命令
                execvp(sub_args[0], sub_args);
                printf("子进程结束\n");
                return 1;
            } else {
                if(is_parallel) {
                    printf("is_parallel \n");
                    continue;
                }
                else{
                    printf("is waiting for sub-process\n");
                    int status;
                    waitpid(pid, &status, 0);  // 阻塞等待
                    // wait(NULL);
                    continue;
                }
            }
            
        }
    }
    return 0;
}

int cmd_q(char *argv) {
    printf("This shell will exit.\n");
    exit(0);
}

int cmd_help(char *argv) 
{
    printf("internal commands:\n");
    for(int i = 0; i < (sizeof(cmd_table) / sizeof(cmd_table[i])); i ++) {
        printf("%d %10s %s\n", i, cmd_table[i].name, cmd_table[i].description);
    }
}

int cmd_history(char *argv)
{
    printf("Command history:\n");
    HIST_ENTRY **history_entry = history_list();
    if (history_entry) {
        for (int i = 0; i < history_length; i++) {
            printf("%ld %d: %s\n", history_get_time(history_entry[i]), i, history_entry[i]->line);
        }
    } else {
        printf("No history available.\n");
    }
}

static char* rl_gets() {
    static char *line_read = NULL;
  
    if (line_read) {
      free(line_read);
      line_read = NULL;
    }
  
    line_read = readline("osh> ");
  
    if (line_read && *line_read) {
      add_history(line_read);
    }
  
    return line_read;
  }

char *command_generator(const char *text, int state) {
    static int list_index, len;
    const char *name;

    // 如果是第一次调用，初始化索引和文本长度
        if (!state) {
        list_index = 0;
        len = strlen(text);
    }

    // 遍历命令列表，查找匹配的命令
    while ((name = cmd_table[list_index++].name)) {
        if (strncmp(name, text, len) == 0) {
            return strdup(name);  // 返回匹配的命令
        }
    }

    // 没有更多匹配项，返回 NULL
    return NULL;
}

char **command_completion(const char *text, int start, int end) {
    rl_attempted_completion_over = 1;
    return rl_completion_matches(text, command_generator);
}