#include <mica_service.h>
#include <prt_log.h>
#include <unistd.h>
#include <print.h>

void log_demo()
{
    int ret;

    ret = mica_service_init();
    if(ret)
    {
        PRT_Printf("mica service init failed. ret = %d\n", ret);
        return;
    }

    while(1)
    {
        PRT_LogSetFilter(OS_LOG_NONE);
        PRT_LogFormat(OS_LOG_DEBUG, OS_LOG_F0, "UniProton DEBUFG Log");
        sleep(2);
        PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "UniProton INFO Log");
        sleep(2);
        PRT_LogFormat(OS_LOG_NOTICE, OS_LOG_F2, "UniProton NOTICE Log");
        sleep(2);
        PRT_LogFormat(OS_LOG_WARN, OS_LOG_F3, "UniProton WARN Log");
        sleep(2);
        PRT_LogFormat(OS_LOG_ERR, OS_LOG_F4, "UniProton ERR Log");
        sleep(2);

        PRT_LogSetFilter(OS_LOG_INFO);

        PRT_LogFormat(OS_LOG_DEBUG, OS_LOG_F0, "UniProton DEBUFG Log should not be printed");
        sleep(2);
        PRT_LogFormat(OS_LOG_INFO, OS_LOG_F1, "UniProton INFO Log should not be printed");
        sleep(2);
        PRT_LogFormat(OS_LOG_NOTICE, OS_LOG_F2, "UniProton NOTICE Log");
        sleep(2);
        PRT_LogFormat(OS_LOG_WARN, OS_LOG_F3, "UniProton WARN Log");
        sleep(2);
        PRT_LogFormat(OS_LOG_ERR, OS_LOG_F4, "UniProton ERR Log");
        sleep(2);
    }
}
