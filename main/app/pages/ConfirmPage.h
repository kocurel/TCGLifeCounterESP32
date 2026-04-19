#ifndef CONFIRM_PAGE_H
#define CONFIRM_PAGE_H

typedef void (*ConfirmCallback)(void);
typedef void (*CancelCallback)(void);

void ConfirmPage_enter(const char* message, ConfirmCallback on_confirm,
                       CancelCallback on_cancel);
#endif  // CONFIRM_PAGE_H