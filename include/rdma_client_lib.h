#ifndef RDMA_CLIENT_LIB_H
#define RDMA_CLIENT_LIB_H

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
 * @brief Client Thread Function Type
 */
typedef void *(*thread_fn_t)(void *);

/**
 * @struct client_ctx_t
 * @brief Client Connection Context Info
 */
typedef struct client_ctx_s {
    /* RDMA Connection Specific Attributes */
    struct rdma_cm_id *cm_id;      //< RDMA CM Core Identifier
    struct rdma_cm_id *addr_id;    //< RDMA CM Address Identifier
    struct rdma_cm_id *connect_id; //< RDMA CM Connect Identifier
    struct ibv_context *verbs;     //< Verbs Context
    struct ibv_pd *pd;             //< Verbs Protection Domain
    struct ibv_cq *scq;            //< Verbs Send CQ
    struct ibv_cq *rcq;            //< Verbs Recv CQ

    /* Event Monitor Specific attributes */
    struct rdma_event_channel *channel; //< RDMA Event Channel
    pthread_t evt_thread;               //< RDMA Event Thread
    thread_fn_t evt_fn;                 //< RDMA Event Thread Function Callback
    pthread_mutex_t evt_mtx;            //< RDMA Event Thread Sync Mtx
    pthread_cond_t evt_cv;              //< RDMA Event Thread Sync Cv
    bool is_connected;                  //< RDMA Client-Server Connected

    /* Poll Monitor Specific attributes */
    pthread_t wcq_thread;
    thread_fn_t wcq_fn;
    pthread_mutex_t wcq_mtx;
    pthread_cond_t wcq_cv;

    /* Memory to be registered and used by client-server communication */
    void *send_client_buf;      //< RDMA compliant send buf
    size_t send_client_buf_sz;  //< size of send buf
    void *recv_client_buf;      //< RDMA complaint recv for send buf
    size_t recv_client_buf_sz;  //< size of recv for send buf
    struct ibv_mr *send_buf_mr; //< RDMA compliant send buf mr
    struct ibv_mr *recv_buf_mr; //< RDMA compliant recv buf mr
    bool rtt_done[MAX_SEND_WR]; //< Condition to be set if a single send/recv
                                // round-trip is done on client
} client_ctx_t;

/**
 * @brief Given a source and target IP address, setup & connect a client
 * control plane to a target server
 */
client_ctx_t *setup_client(struct sockaddr *src_addr,
                           struct sockaddr *dst_addr);

/**
 * @brief Process client response received
 */
int process_client_response(client_ctx_t *ctx, int opc, size_t msg_sz);

/**
 * @brief Send client request to server
 */
int send_client_request(client_ctx_t *ctx, int opc, size_t msg_sz);

/**
 * @brief Prepare client request & response to be send/recv
 */
int prepare_client_data(client_ctx_t *ctx, int opc);

#endif /*! RDMA_CLIENT_LIB_H */
