#ifndef __XAC_LOTTERY_LOG_H_
#define __XAC_LOTTERY_LOG_H_

bool Logging_Init();
bool Logging_Write(const char* fmt, ...);
bool Logging_Close();

#endif
