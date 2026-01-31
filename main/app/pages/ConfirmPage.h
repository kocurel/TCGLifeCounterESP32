#ifndef CONFIRM_PAGE_H
#define CONFIRM_PAGE_H

// ConfirmPage.h
typedef void (*ConfirmCallback)(void);
void ConfirmPage_enter(const char* message, ConfirmCallback on_confirm,
                       void (*on_cancel)(void));
#endif  // CONFIRM_PAGE_H