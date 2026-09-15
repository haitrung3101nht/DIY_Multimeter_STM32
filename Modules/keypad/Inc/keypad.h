#ifndef KEYPAD_H
#define KEYPAD_H

void Keypad_Init(void);

/* Gọi thường xuyên. Trả về ký tự phím mới nhấn, hoặc '\0'. */
char Keypad_GetKey(void);

#endif