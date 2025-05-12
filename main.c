#include "mongoose.h"
#include "input.h"
#include <stdio.h>
#include <stdlib.h>
#include <string.h>

enum { ERR_OK = 0,
     ERR_FILE_NOT_FOUND = 1 };

typedef struct {
    char text[256];
    char priority[10];
    int order;
} Task;

// Обработка GET-запроса (главная страница)
static int handle_get_root(struct mg_connection *c, char **response, const char **ctype) {
    *response = read_file("login.html");
    if (*response) {
        *ctype = "Content-Type: text/html\r\n";
        return ERR_OK;
    }
    return ERR_FILE_NOT_FOUND;
}

// Обработка POST-запроса (отправка задач)
static int handle_post_submit(struct mg_connection *c, struct mg_http_message *hm, char **response, const char **ctype) {
    *response = NULL;
    char *template = read_file("table.html");
    if (!template) return ERR_FILE_NOT_FOUND;

    Task tasks[3] = {0};
    int task_count = 0;

    // Парсинг данных формы
    for (int i = 1; i <= 3; i++) {
        char task[256] = {0}, priority[10] = {0};
        char name[16];
        
        snprintf(name, sizeof(name), "task%d", i);
        mg_http_get_var(&hm->body, name, task, sizeof(task));
        
        snprintf(name, sizeof(name), "priority%d", i);
        mg_http_get_var(&hm->body, name, priority, sizeof(priority));

        if (strlen(task) > 0) {
            strncpy(tasks[task_count].text, task, sizeof(tasks[0].text));
            strncpy(tasks[task_count].priority, priority, sizeof(tasks[0].priority));
            tasks[task_count].order = strcmp(priority, "high") == 0 ? 3 : 
                                     strcmp(priority, "medium") == 0 ? 2 : 1;
            task_count++;
        }
    }

    if (task_count == 0) {
        free(template);
        return ERR_FILE_NOT_FOUND;
    }

    // Сортировка пузырьком
    for (int i = 0; i < task_count-1; i++) {
        for (int j = 0; j < task_count-i-1; j++) {
            if (tasks[j].order < tasks[j+1].order) {
                Task temp = tasks[j];
                tasks[j] = tasks[j+1];
                tasks[j+1] = temp;
            }
        }
    }

    // Генерация HTML-строк
    char rows[2048] = "";
    for (int i = 0; i < task_count; i++) {
        char row[512];
        snprintf(row, sizeof(row), "<tr class='%s'><td>%s</td><td>%s</td></tr>",
                tasks[i].priority, tasks[i].text, tasks[i].priority);
        strncat(rows, row, sizeof(rows)-1);
    }

    // Вставка в шаблон
    const char *placeholder = "<!-- TASK_ROWS -->";
    char *pos = strstr(template, placeholder);
    if (!pos) {
        free(template);
        return ERR_FILE_NOT_FOUND;
    }

    size_t len = (pos - template) + strlen(rows) + strlen(pos + strlen(placeholder)) + 1;
    *response = malloc(len);
    if (!*response) {
        free(template);
        return ERR_FILE_NOT_FOUND;
    }

    snprintf(*response, len, "%.*s%s%s", 
            (int)(pos - template), template, rows, pos + strlen(placeholder));
    
    free(template);
    *ctype = "Content-Type: text/html\r\n";
    return ERR_OK;
}

// Обработка запроса CSS
static int handle_css_request(struct mg_connection *c, char **response, const char **ctype) {
    *response = read_file("login.css");
    if (*response) {
        *ctype = "Content-Type: text/css\r\n";
        return ERR_OK;
    }
    return ERR_FILE_NOT_FOUND;
}

// Обработчик событий
static void fn(struct mg_connection *c, int ev, void *ev_data) {
    
    if (ev == MG_EV_HTTP_MSG) {
        printf("+");
        struct mg_http_message *hm = (struct mg_http_message *)ev_data;
        const char *ctype;
        char *response = NULL;
        int status = 500;
        int err = ERR_FILE_NOT_FOUND;

        
        if (mg_strcmp(hm->uri, mg_str("/")) == 0) {
            err = handle_get_root(c, &response, &ctype);
        } 
        else if (mg_strcmp(hm->uri, mg_str("/submit")) == 0) {
            err = handle_post_submit(c, hm, &response, &ctype);
        }
        else if (mg_strcmp(hm->uri, mg_str("/login.css")) == 0) {
            err = handle_css_request(c, &response, &ctype);
        }
        else {
            err = handle_get_root(c, &response, &ctype);
        }
        printf("%d\n",err);

        status = (err == ERR_OK) ? 200 : 500;
        mg_http_reply(c, status, ctype, "%s", response ? response : "");
        free(response);
    }
}

int main() {
    struct mg_mgr mgr;
    mg_mgr_init(&mgr);
    mg_http_listen(&mgr, "http://127.0.0.1:8081", fn, NULL);
    printf("Server running on http://0.0.0.0:8081\n");
    while (true) mg_mgr_poll(&mgr, 1000);
    return 0;
}
