//
// Created by 田地 on 2021/3/23.
//

#ifndef PA3_UTILS_H
#define PA3_UTILS_H

#include <iostream>
#include <string>

#define LOG_LEVEL 3

#define TEMP_FAILURE_RETRY(expr)\
    ({  long int __result;\
        do __result = (long int)(expr);\
        while(__result == -1 && errno == EINTR);\
        __result;})\

/**
 * compare sockaddrs
 * @param x
 * @param y
 * @return 0 iff x == y
 * @throws std::runtime_error thrown if sa_family is unsupported
 */
int CmpSockaddr(struct sockaddr *x, struct sockaddr *y);


inline void DEBUG(const std::string& msg) {
#ifdef LOG_LEVEL
    if (LOG_LEVEL < 4)
        return ;
#endif
    std::cout<< "[DEBUG] " << msg <<std::endl;
}

inline void INFO(const std::string& msg) {
#ifdef LOG_LEVEL
    if (LOG_LEVEL < 3)
        return ;
#endif
    std::cout<< "[INFO] " << msg <<std::endl;
}

inline void WARN(const std::string& msg) {
#ifdef LOG_LEVEL
    if (LOG_LEVEL < 2)
        return ;
#endif
    std::cerr<< "[WARN] " << msg <<std::endl;
}

inline void ERROR(const std::string& msg) {
#ifdef LOG_LEVEL
    if (LOG_LEVEL < 1)
        return ;
#endif
    std::cerr<< "[ERROR] " << msg <<std::endl;
}

inline void DEBUG(const std::string& id, const std::string& msg) {
#ifdef LOG_LEVEL
    if (LOG_LEVEL < 4)
        return ;
#endif
    std::cout<< "[DEBUG] " << "[ID " << id << "] " << msg <<std::endl;
}

inline void INFO(const std::string& id, const std::string& msg) {
#ifdef LOG_LEVEL
    if (LOG_LEVEL < 3)
        return ;
#endif
    std::cout<< "[INFO] " << "[ID " << id << "] " << msg <<std::endl;
}

inline void WARN(const std::string& id, const std::string& msg) {
#ifdef LOG_LEVEL
    if (LOG_LEVEL < 2)
        return ;
#endif
    std::cerr<< "[WARN] " << "[ID " << id << "] " << msg <<std::endl;
}

inline void ERROR(const std::string& id, const std::string& msg) {
#ifdef LOG_LEVEL
    if (LOG_LEVEL < 1)
        return ;
#endif
    std::cerr<< "[ERROR] " << "[ID " << id << "] " << msg <<std::endl;
}

#endif //PA3_UTILS_H
