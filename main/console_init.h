#ifndef CONSOLE_INIT_H
#define CONSOLE_INIT_H
#define PROMPT_STR CONFIG_IDF_TARGET

void initialize_console();
void console_task(void* pvParameters);
#endif  // CONSOLE_INIT_H