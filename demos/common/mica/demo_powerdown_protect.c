#include <prt_typedef.h>
#include <mica_service.h>
#include <pthread.h>
#include <semaphore.h>
#include <print.h>
#include <demo.h>

static sem_t active_sem;
static sem_t fin_notify_sem;

void poweroff_protect_callback(void)
{
    sem_post(&active_sem);
    sem_wait(&fin_notify_sem);
}

void *user_thread_function(void *arg)
{
    uint8_t data[128] = {"ABCDEFGHIJKLMNOPQRSTUVWXYZABCDEFGHIJKLMNOPQRSTUVWXYZ"};
    sem_wait(&active_sem);
    for (int i = 0; i < 128; i++)
    {
        write_powerdown_prot_data(data, sizeof(data));
    }
    sem_post(&fin_notify_sem);
    return NULL;
}

void demo_powerdown_protect(void)
{
    int ret;
    pthread_t tid;
    pthread_attr_t attr;

    ret = mica_service_init();
    if (ret)
    {
        PRT_Printf("mica service init failed. ret = %d\n", ret);
        return;
    }

    ret = sem_init(&active_sem, 0, 0);
    if (ret)
    {
        PRT_Printf("sem_init active_sem failed. ret = %d\n", ret);
        mica_service_uninit();
        return;
    }

    ret = sem_init(&fin_notify_sem, 0, 0);
    if (ret)
    {
        PRT_Printf("sem_init fin_notify_sem failed. ret = %d\n", ret);
        mica_service_uninit();
        sem_destroy(&active_sem);
        return;
    }

    ret = powerdown_prot_service_init();
    if (ret)
    {
        PRT_Printf("powerdown_prot_service_init failed. ret = %d\n", ret);
        goto err;
    }
    set_powerdown_prot_ipi_handler(poweroff_protect_callback);

    pthread_attr_init(&attr);
    pthread_attr_setdetachstate(&attr, PTHREAD_CREATE_DETACHED);

    ret = pthread_create(&tid, &attr, user_thread_function, NULL);
    if (ret)
    {
        PRT_Printf("pthread_create user_thread_function failed. ret = %d\n", ret);
        goto err;
    }

    return;

err:
    sem_destroy(&active_sem);
    sem_destroy(&fin_notify_sem);
    mica_service_uninit();
}
