#ifndef THREAD_POOL_H
#define THREAD_POOL_H

#include <atomic>
#include <condition_variable>
#include <functional>
#include <mutex>
#include <queue>
#include <thread>

class thread_pool
{
public:

    thread_pool( const size_t thread_count )
        : m_jobs_left( 0 )
        , m_bailout( false )
        , m_thread_count( thread_count )
    {
        for( unsigned i = 0; i < m_thread_count; ++i )
            m_threads.emplace_back( std::thread( [this/*,i*/]{ this->task(); } ) );
    }

    /**
     *  JoinAll on deconstruction
     */
    ~thread_pool() {
        join_all();
    }

    /**
     *  Get the number of threads in this pool
     */
    size_t size() const {
        return m_thread_count;
    }

    /**
     *  Get the number of jobs left in the queue.
     */
    size_t jobs_remaining() {
        std::lock_guard<std::mutex> guard( m_queue_mutex );
        return m_queue.size();
    }

    /**
     *  Add a new job to the pool. If there are no jobs in the queue,
     *  a thread is woken up to take the job. If all threads are busy,
     *  the job is added to the end of the queue.
     */
    void add_job( std::function<void(void)> job )
    {
        // scoped lock
        {
            std::lock_guard<std::mutex> lock( m_queue_mutex );
            m_queue.emplace( job );
        }
        // scoped lock
        {
            std::lock_guard<std::mutex> lock( m_jobs_left_mutex );
            ++m_jobs_left;
        }
        m_job_available_var.notify_one();
    }

    /**
     *  Join with all threads. Block until all threads have completed.
     *  Params: WaitForAll: If true, will wait for the queue to empty
     *          before joining with threads. If false, will complete
     *          current jobs, then inform the threads to exit.
     *  The queue will be empty after this call, and the threads will
     *  be done. After invoking `ThreadPool::JoinAll`, the pool can no
     *  longer be used. If you need the pool to exist past completion
     *  of jobs, look to use `ThreadPool::WaitAll`.
     */
    void join_all( bool wait_for_all = true )
    {
        // scoped lock
        {
            std::lock_guard<std::mutex> lock( m_queue_mutex );
            if ( m_bailout )
            {
                return;
            }
            m_bailout = true;
        }

        // note that we're done, and wake up any thread that's
        // waiting for a new job
        m_job_available_var.notify_all();

        for ( auto& x : m_threads )
        {
            if ( x.joinable() )
            {
                x.join();
            }
        }
    }

    /**
     *  Wait for the pool to empty before continuing.
     *  This does not call `std::thread::join`, it only waits until
     *  all jobs have finshed executing.
     */
    void wait_all()
    {
        std::unique_lock<std::mutex> lock( m_jobs_left_mutex );
        if ( m_jobs_left > 0)
        {
            m_wait_var.wait( lock, [this]
                {
                    return m_jobs_left == 0;
                });
        }
    }

private:

    /**
     *  Take the next job in the queue and run it.
     *  Notify the main thread that a job has completed.
     */
    void task()
    {
        while ( true )
        {
            std::function<void(void)> job;

            // scoped lock
            {
                std::unique_lock<std::mutex> lock( m_queue_mutex );

                if ( m_bailout )
                    return;

                // Wait for a job if we don't have any.
                m_job_available_var.wait(lock, [this]
                    {
                        return m_queue.size() > 0 || m_bailout;
                    });

                if ( m_bailout )
                    return;

                // Get job from the queue
                job = m_queue.front();
                m_queue.pop();
            }

            // do the job
            job();

            // scoped lock
            {
                std::lock_guard<std::mutex> lock( m_jobs_left_mutex );
                --m_jobs_left;
            }

            m_wait_var.notify_one();
        }
    }

private:

    std::vector<std::thread> m_threads;
    std::queue<std::function<void(void)>> m_queue;

    std::atomic_int         m_jobs_left;
    std::atomic_bool        m_bailout;
    std::condition_variable m_job_available_var;
    std::condition_variable m_wait_var;
    std::mutex              m_jobs_left_mutex;
    std::mutex              m_queue_mutex;

    const size_t m_thread_count;
};

#endif //THREAD_POOL_H

