#include <stdio.h>
#include <stdlib.h>

struct Tuple5
{
    uint32_t client_ip;
    uint16_t client_port;
    uint32_t server_ip;
    uint16_t server_port;
    u_char l4;
};
struct transaction
{
    int transaction_id;
    struct timeval start;
    struct timeval time_live;
    int conn_id;
    int total_size_packet;
    // struct Tuple5 tuple;
    int flag_start;
    int num_inbound_packets_in_range;
    int num_outbound_packets_in_range;
    int max_packet_size_inbound;
    int min_packet_size_inbound;
    double max_diff_time_inbound;
    double min_diff_time_inbound;
    double SumSquareInboundPacketTimeDiff;
    double SumInboundPacketTimeDiff;
    struct timeval time_last_packet;
    struct timeval Time_request;
    struct timeval first_packet_from_the_server;
    time_t RTT;
};

struct connection
{
    int conn_id;
    struct listelement* element;
    int indexConn;
    struct Tuple5 *tuple;
    int start_flag;
    // struct transaction *tran;
    struct transaction *all_tran;
    int current_tran;
    size_t total_bandwidth;
    struct timeval time_live;
    struct timeval start;
    struct timeval end;
    int status;
};
struct timer
{
      struct timeval lockal_time;
};
struct hash_value
{
    int conn_id;
};
struct packet
{
    uint32_t src_ip;
    uint16_t src_port;
    uint32_t dest_ip;
    uint16_t dest_port;
    u_char version;
    struct timeval time;
    size_t size_data;
    size_t size_packet;
};
struct emptyQueue
{
    int *emptyQueueConn;
    int index;
};
struct my_object
{
    int request_packet_threshold;
    int minimum_video_connection_size;
    int inbound_packets_in_range_min;
    int inbound_packets_in_range_max;
    int outbound_packets_in_range_min;
    int outbound_packets_in_range_max;
    int max_diff_time_inbound_threshold;
    int min_diff_time_inbound_threshold;
    int number_of_videos_to_output_statistics_per_video;
    int max_number_of_connections;
    int max_number_of_transaction_per_video;
    int video_connection_timeout;
};
