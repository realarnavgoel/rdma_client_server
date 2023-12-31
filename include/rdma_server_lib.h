#ifndef RDMA_SERVER_LIB_H
#define RDMA_SERVER_LIB_H

#include <netinet/in.h>
#include <stdarg.h>
#include <stdbool.h>
#include <stdint.h>
#include <stdio.h>
#include <sys/types.h>

#define MAX_SEND_WR 1024
#define MAX_RECV_WR 512
#define MAX_CQE 512

/**
 * @struct thread_fn_t
 * @brief Server Thread Function Type
 */
typedef void *(*thread_fn_t)(void *);

/**
 * @struct server_ctx_t
 * @brief Server Connection Context Info
 */
typedef struct server_ctx_s {
    /* RDMA Connection Specific Attributes */
    struct rdma_cm_id *cm_id;     //< RDMA CM Identifier
    struct rdma_cm_id *listen_id; //< RDMA CM Listen Identifier
    struct ibv_context *verbs;    //< Verbs Context
    struct ibv_pd *pd;            //< Verbs Protection Domain
    struct ibv_cq *scq;           //< Verbs Send CQ
    struct ibv_cq *rcq;           //< Verbs Recv CQ

    /* Event Monitor Specific attributes */
    struct rdma_event_channel *channel; //< RDMA Event Channel
    pthread_t evt_thread;               //< RDMA Event Thread
    thread_fn_t evt_fn;                 //< RDMA Event Thread Function Callback
    pthread_mutex_t evt_mtx;            //< RDMA Event Thread Sync Mtx
    pthread_cond_t evt_cv;              //< RDMA Event Thread Sync Cv
    bool is_connected;

    /* Poll Monitor Specific attributes */
    pthread_t wcq_thread;
    thread_fn_t wcq_fn;
    pthread_mutex_t wcq_mtx;
    pthread_cond_t wcq_cv;

    /* Memory to be registered and used by client-server communication */
    void *send_server_buf;      //< RDMA compliant send buf
    size_t send_server_buf_sz;  //< size of send buf
    void *recv_server_buf;      //< RDMA complaint recv for send buf
    size_t recv_server_buf_sz;  //< size of recv for send buf
    struct ibv_mr *send_buf_mr; //< RDMA compliant send buf mr
    struct ibv_mr *recv_buf_mr; //< RDMA compliant recv buf mr
    int recv_opc[MAX_SEND_WR];  //< RDMA received immediate opcode from client
    size_t recv_sz;             //< RDMA WCQE byte len
} server_ctx_t;

/**
 * @brief Given a user-defined IP and port, setup the server
 * control plane
 */
server_ctx_t *setup_server(struct sockaddr *addr, uint16_t port_id);

/**
 * @brief Given a server context, setup its connection to client
 */
int connect_server(server_ctx_t *ctx);

/**
 * @brief Given a previously connected server context, teardown its connection
 * to a client
 */
int disconnect_server(server_ctx_t *ctx);

/**
 * @brief Prepare the input/output req/response data for server
 */
int prepare_server_data(server_ctx_t *ctx);

/**
 * @brief Recv the request, based on the immediate opcode, send response
 * to client
 */
int send_recv_server(server_ctx_t *ctx);

#endif /*! RDMA_SERVER_LIB_H */
